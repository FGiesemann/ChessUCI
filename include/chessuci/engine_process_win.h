/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_WIN_H
#define CHESSUCI_ENGINE_PROCESS_WIN_H

#include "chessuci/engine_process.h"

#include <windows.h>

namespace chessuci {

class EngineProcessWin : public EngineProcess {
public:
    EngineProcessWin() = default;
    ~EngineProcessWin() override;

    /** \copydoc EngineProcess::start */
    auto start(const ProcessParams &params) -> bool override;

    /** \copydoc EngineProcess::is_running */
    auto is_running() const -> bool override;

    /** \copydoc EngineProcess::pid */
    auto pid() const -> proc_id_t override;

    /** \copydoc EngineProcess::terminate */
    auto terminate(int timeout_ms = 3000) -> bool override;

    /** \copydoc EngineProcess::kill */
    auto kill() -> void override;

    /** \copydoc EngineProcess::wait_for_exit */
    auto wait_for_exit(int timeout_ms = 0) -> std::optional<int> override;

    /** \copydoc EngineProcess::write_line */
    auto write_line(const std::string &line) -> bool override;

    /** \copydoc EngineProcess::read_line */
    auto read_line(std::string &line) -> bool override;

    /** \copydoc EngineProcess::can_read */
    auto can_read() const -> bool override;

    /** \copydoc EngineProcess::last_error */
    auto last_error() const -> const std::string & override;
private:
    static constexpr DWORD terminate_timeout{5000};

    class Pipe {
    public:
        ~Pipe();

        auto create(SECURITY_ATTRIBUTES *attributes, bool inherit_read, bool inherit_write) -> bool;

        auto read() const -> const HANDLE & { return m_read; }
        auto read() -> HANDLE & { return m_read; }
        auto write() const -> const HANDLE & { return m_write; }
        auto write() -> HANDLE & { return m_write; }

        auto close_read() -> void { close_handle(&m_read); }
        auto close_write() -> void { close_handle(&m_write); }
    private:
        HANDLE m_read{INVALID_HANDLE_VALUE};
        HANDLE m_write{INVALID_HANDLE_VALUE};

        auto close_handle(HANDLE *handle) -> void {
            if (*handle != INVALID_HANDLE_VALUE) {
                CloseHandle(*handle);
                *handle = INVALID_HANDLE_VALUE;
            }
        }
    };
    Pipe m_std_in{};
    Pipe m_std_out{};
    Pipe m_std_err{};
    HANDLE m_process_handle{INVALID_HANDLE_VALUE};
    proc_id_t m_process_id{};

    mutable std::atomic<bool> m_running{false};
    mutable std::string m_last_error{};

    std::string m_read_buffer;

    auto create_pipes() -> bool;
    auto create_child_process(const ProcessParams &params) -> bool;
    auto close_handles() -> void;
    auto set_error(const std::string &message) -> void { m_last_error = message; }
    auto wait_for_process(DWORD timeout_ms, DWORD &exit_code) const -> bool;
    static auto build_command_line(const ProcessParams &params) -> std::wstring;
    static auto format_windows_error(DWORD error_code) -> std::string;
};

} // namespace chessuci

#endif
