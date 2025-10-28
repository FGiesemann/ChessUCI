/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_process_win.h"
#include "chessuci/protocol.h"

namespace chessuci {

namespace {

auto utf8_to_wide(const std::string &utf8) -> std::wstring {
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

auto wide_to_utf8(const std::wstring &wide) -> std::string {
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

} // namespace

auto EngineProcessWin::start(const ProcessParams &params) -> void {
    if (!create_pipes()) {
        return;
    }
    create_child_process(params);
}

auto EngineProcessWin::is_running() const -> bool {
    if (m_process_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD result = WaitForSingleObject(m_process_handle, 0);
    return result == WAIT_TIMEOUT;
}

auto EngineProcessWin::pid() const -> proc_id_t {
    return m_process_id;
}

auto EngineProcessWin::terminate(int timeout_ms) -> bool {
    if (m_process_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    write_line("quit");
    DWORD wait_result = WaitForSingleObject(m_process_handle, static_cast<DWORD>(timeout_ms));
    if (wait_result == WAIT_OBJECT_0) {
        return true;
    }
}

auto EngineProcessWin::kill() -> void {}

auto EngineProcessWin::wait_for_exit(int timeout_ms) -> std::optional<int> {}

auto EngineProcessWin::write_line(const std::string &line) -> bool {}

auto EngineProcessWin::read_line(std::string &line) -> bool {}

auto EngineProcessWin::can_read() const -> bool {}

auto EngineProcessWin::last_error() const -> const std::string & {
    return m_last_error;
}

auto EngineProcessWin::create_pipes() -> bool {
    SECURITY_ATTRIBUTES saAttr{.nLength = sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor = nullptr, .bInheritHandle = TRUE};
    if (CreatePipe(&m_std_out.read, &m_std_out.write, &saAttr, 0) == FALSE) {
        m_last_error = "Could not create stdout pipe";
        return false;
    }
    if (SetHandleInformation(m_std_out.read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
        m_last_error = "Could not set stdout handle inheritance";
        return false;
    }

    if (CreatePipe(&m_std_in.read, &m_std_in.write, &saAttr, 0) == FALSE) {
        m_last_error = "Could not create stdin pipe";
        return false;
    }
    if (SetHandleInformation(m_std_in.write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
        m_last_error = "Could not set stdin handle inheritance";
        return false;
    }
    return true;
}

auto EngineProcessWin::create_child_process(const ProcessParams &params) -> void {
    std::wstring command_line = build_command_line(params);

    STARTUPINFOW start_info;
    ZeroMemory(&start_info, sizeof(start_info));
    start_info.cb = sizeof(STARTUPINFOW);
    start_info.hStdError = m_std_out.write;
    start_info.hStdOutput = m_std_out.write;
    start_info.hStdInput = m_std_in.read;
    start_info.dwFlags |= STARTF_USESTDHANDLES;

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
        return;
    }

    CloseHandle(m_std_out.write);
    m_std_out.write = INVALID_HANDLE_VALUE;
    CloseHandle(m_std_in.read);
    m_std_in.read = INVALID_HANDLE_VALUE;

    m_process_handle = process_info.hProcess;
    m_process_id = process_info.dwProcessId;

    DWORD wait_result = WaitForInputIdle(m_process_handle, 1000);
    if (wait_result == WAIT_FAILED) {
        m_last_error = "Error waiting for engine process to become idle";
    }

    CloseHandle(process_info.hThread);
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

EngineProcessWin::Pipe::~Pipe() {
    if (read != INVALID_HANDLE_VALUE) {
        CloseHandle(read);
        read = INVALID_HANDLE_VALUE;
    }
    if (write != INVALID_HANDLE_VALUE) {
        CloseHandle(write);
        write = INVALID_HANDLE_VALUE;
    }
}

} // namespace chessuci
