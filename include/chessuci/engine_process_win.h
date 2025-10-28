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
    /** \copydoc EngineProcess::start */
    auto start(const ProcessParams &params) -> void override;

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
    struct Pipe {
        HANDLE read{INVALID_HANDLE_VALUE};
        HANDLE write{INVALID_HANDLE_VALUE};

        ~Pipe();
    };
    Pipe m_std_out{};
    Pipe m_std_in{};
    HANDLE m_process_handle{INVALID_HANDLE_VALUE};
    proc_id_t m_process_id{};

    mutable std::string m_last_error{};

    auto create_pipes() -> bool;
    auto create_child_process(const ProcessParams &params) -> void;
    static auto build_command_line(const ProcessParams &params) -> std::wstring;
    static auto format_windows_error(DWORD error_code) -> std::string;
};

} // namespace chessuci

#endif
