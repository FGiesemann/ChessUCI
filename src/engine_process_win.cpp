/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_process_win.h"
#include "chessuci/protocol.h"

namespace chessuci {

auto EngineProcessWin::utf8_to_wide(const std::string &utf8) -> std::wstring {
    if (utf8.empty()) {
        return {};
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (size == 0) {
        return {};
    }
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), result.data(), size);

    return result;
}

auto EngineProcessWin::wide_to_utf8(const std::wstring &wide) -> std::string {
    if (wide.empty()) {
        return {};
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        return {};
    }
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), result.data(), size, nullptr, nullptr);

    return result;
}

EngineProcessWin::~EngineProcessWin() {
    if (is_running()) {
        terminate(1000);
        if (is_running()) {
            kill();
        }
    }
    close_handles();
}

auto EngineProcessWin::start(const ProcessParams &params) -> bool {
    if (is_running()) {
        set_error("Process already running");
        return false;
    }

    if (!create_pipes()) {
        return false;
    }

    if (!create_child_process(params)) {
        close_handles();
        return false;
    }

    m_running = true;

    DWORD exit_code{};
    if (GetExitCodeProcess(m_process_handle, &exit_code) == TRUE) {
        if (exit_code != STILL_ACTIVE) {
            m_running = false;
            set_error("Process exited immediately with code " + std::to_string(exit_code));
            close_handles();
            return false;
        }
    }

    return true;
}

auto EngineProcessWin::is_running() const -> bool {
    if (!m_running || m_process_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD result = WaitForSingleObject(m_process_handle, 0);
    switch (result) {
    case WAIT_TIMEOUT:
        return true;
    case WAIT_OBJECT_0:
        [[fallthrough]];
    case WAIT_FAILED:
    default:
        m_running = false;
        return false;
    }
}

auto EngineProcessWin::pid() const -> proc_id_t {
    return m_process_id;
}

auto EngineProcessWin::terminate(int timeout_ms) -> bool {
    if (!is_running()) {
        return true;
    }

    write_line("quit");
    DWORD exit_code{};
    if (wait_for_process(timeout_ms, exit_code)) {
        m_running = false;
        close_handles();
        return true;
    }

    return false;
}

auto EngineProcessWin::kill() -> void {
    if (!is_running() || m_process_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    if (TerminateProcess(m_process_handle, 1) == FALSE) {
        m_running = false;
        close_handles();
        return;
    }

    DWORD result = WaitForSingleObject(m_process_handle, terminate_timeout);
    if (result == WAIT_TIMEOUT) {
        set_error("Process didn't terminate");
    }

    m_running = false;
    close_handles();
}

auto EngineProcessWin::wait_for_exit(int timeout_ms) -> std::optional<int> {
    if (m_process_handle == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }

    DWORD exit_code{};
    if (wait_for_process(timeout_ms, exit_code)) {
        m_running = false;
        close_handles();
        return static_cast<int>(exit_code);
    }

    return std::nullopt;
}

auto EngineProcessWin::wait_for_process(DWORD timeout_ms, DWORD &exit_code) const -> bool {
    DWORD result = WaitForSingleObject(m_process_handle, timeout_ms);
    if (result == WAIT_OBJECT_0) {
        GetExitCodeProcess(m_process_handle, &exit_code);
        return true;
    }

    return false;
}

auto EngineProcessWin::write_line(const std::string &line) -> bool {
    if (!is_running()) {
        set_error("Process not running");
        return false;
    }

    std::string message = line + "\n";
    auto bytes_to_write = static_cast<DWORD>(message.size());
    DWORD bytes_written{0};
    bool success = WriteFile(m_std_in.write(), message.c_str(), bytes_to_write, &bytes_written, nullptr) == TRUE;
    if (!success || bytes_written != bytes_to_write) {
        set_error("WriteFile failed: " + format_windows_error(GetLastError()));
        return false;
    }
    FlushFileBuffers(m_std_in.write());

    return true;
}

auto EngineProcessWin::read_line(std::string &line) -> bool {
    size_t newline_pos = m_read_buffer.find('\n');
    if (newline_pos != std::string::npos) {
        line = m_read_buffer.substr(0, newline_pos);
        m_read_buffer.erase(0, newline_pos + 1);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return true;
    }

    char buffer[4096];
    while (true) {
        DWORD bytes_read = 0;
        bool success = ReadFile(m_std_out.read(), buffer, sizeof(buffer), &bytes_read, nullptr) == TRUE;

        if (success && bytes_read > 0) {
            m_read_buffer.append(buffer, bytes_read);
            newline_pos = m_read_buffer.find('\n');
            if (newline_pos != std::string::npos) {
                line = m_read_buffer.substr(0, newline_pos);
                m_read_buffer.erase(0, newline_pos + 1);

                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                return true;
            }
        } else if (bytes_read == 0) {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE) {
                set_error("Process closed stdout (broken pipe)");
            } else {
                set_error("ReadFile failed: " + format_windows_error(error));
            }
            return false;

        } else {
            set_error("ReadFile failed: " + format_windows_error(GetLastError()));
            return false;
        }
    }
}

auto EngineProcessWin::can_read() const -> bool {
    if (m_read_buffer.find('\n') != std::string::npos) {
        return true;
    }
    DWORD bytes_available{};
    bool success = PeekNamedPipe(m_std_out.read(), nullptr, 0, nullptr, &bytes_available, nullptr) == TRUE;
    return success && bytes_available > 0;
}

auto EngineProcessWin::last_error() const -> const std::string & {
    return m_last_error;
}

auto EngineProcessWin::create_pipes() -> bool {
    SECURITY_ATTRIBUTES saAttr{.nLength = sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor = nullptr, .bInheritHandle = TRUE};
    if (!m_std_in.create(&saAttr, true, false)) {
        set_error("Failed to create stdin pipe: " + format_windows_error(GetLastError()));
        return false;
    }
    if (!m_std_out.create(&saAttr, false, true)) {
        set_error("Failed to create stdout pipe: " + format_windows_error(GetLastError()));
        return false;
    }
    if (!m_std_err.create(&saAttr, false, true)) {
        set_error("Failed to create stderr pipe: " + format_windows_error(GetLastError()));
        return false;
    }
    return true;
}

auto EngineProcessWin::create_child_process(const ProcessParams &params) -> bool {
    std::wstring command_line = build_command_line(params);

    STARTUPINFOW start_info;
    ZeroMemory(&start_info, sizeof(start_info));
    start_info.cb = sizeof(STARTUPINFOW);
    start_info.hStdError = m_std_err.write();
    start_info.hStdOutput = m_std_out.write();
    start_info.hStdInput = m_std_in.read();
    start_info.dwFlags = STARTF_USESTDHANDLES;

    const wchar_t *working_dir = nullptr;
    std::wstring working_dir_str;
    if (params.working_directory.has_value()) {
        working_dir_str = params.working_directory->wstring();
        working_dir = working_dir_str.c_str();
    }

    PROCESS_INFORMATION process_info;
    BOOL success =
        CreateProcessW(nullptr, command_line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, nullptr, working_dir, &start_info, &process_info);
    if (success == FALSE) {
        DWORD error = GetLastError();
        m_last_error = "Error creating process: " + format_windows_error(error);
        return false;
    }

    m_std_err.close_write();
    m_std_out.close_write();
    m_std_in.close_read();

    m_process_handle = process_info.hProcess;
    m_process_id = static_cast<proc_id_t>(process_info.dwProcessId);

    CloseHandle(process_info.hThread);
    return true;
}

auto EngineProcessWin::close_handles() -> void {
    close_handle(&m_process_handle);
}

auto EngineProcessWin::build_command_line(const ProcessParams &params) -> std::wstring {
    std::wstring cmd = params.executable.wstring();
    if (cmd.find(L' ') != std::wstring::npos || cmd.find(L'\t') != std::wstring::npos) {
        cmd = L"\"" + cmd + L"\"";
    }

    for (const auto &arg : params.arguments) {
        cmd += L" ";
        std::wstring warg = utf8_to_wide(arg);
        if (warg.find(L' ') != std::wstring::npos || warg.find(L'\t') != std::wstring::npos) {
            cmd += L"\"" + warg + L"\"";
        } else {
            cmd += warg;
        }
    }

    return cmd;
}

auto EngineProcessWin::format_windows_error(DWORD error_code) -> std::string {
    wchar_t *message = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, reinterpret_cast<LPWSTR>(&message), 0, nullptr);
    std::string result = wide_to_utf8(message);
    LocalFree(message);
    return result;
}

auto EngineProcessWin::Pipe::create(SECURITY_ATTRIBUTES *attributes, bool inherit_read, bool inherit_write) -> bool {
    if (CreatePipe(&m_read, &m_write, attributes, 0) == FALSE) {
        return false;
    }
    if (!inherit_read) {
        SetHandleInformation(m_read, HANDLE_FLAG_INHERIT, 0);
    }
    if (!inherit_write) {
        SetHandleInformation(m_write, HANDLE_FLAG_INHERIT, 0);
    }
    return true;
}

EngineProcessWin::Pipe::~Pipe() {
    close_read();
    close_write();
}

} // namespace chessuci
