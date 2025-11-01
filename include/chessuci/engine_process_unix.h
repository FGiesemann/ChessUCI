/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_UNIX_H
#define CHESSUCI_ENGINE_PROCESS_UNIX_H

#include "chessuci/engine_process.h"
#include <atomic>
#include <unistd.h>

namespace chessuci {

class EngineProcessUnix : public EngineProcess {
public:
    EngineProcessUnix() = default;
    ~EngineProcessUnix() override;

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
    static auto close_fd(int &fd) -> void {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }

    class Pipe {
    public:
        ~Pipe() {
            close_read();
            close_write();
        }

        auto create() -> bool { return pipe(m_file_handles) == 0; }

        auto read() const -> const int & { return m_file_handles[0]; }
        auto read() -> int & { return m_file_handles[0]; }
        auto write() const -> const int & { return m_file_handles[1]; }
        auto write() -> int & { return m_file_handles[1]; }

        auto close_read() -> void { close_fd(read()); }
        auto close_write() -> void { close_fd(write()); }
    private:
        int m_file_handles[2]{-1, -1};
    };
    Pipe m_std_in{};
    Pipe m_std_out{};
    Pipe m_std_err{};

    pid_t m_pid{-1};
    mutable std::atomic<bool> m_running{false};
    mutable std::string m_last_error;
    mutable int m_stored_exit_code{};

    std::string m_read_buffer;

    auto create_pipes() -> bool;
    auto create_child_process(const ProcessParams &params) -> bool;
    auto close_pipes() -> void;
    auto set_non_blocking(int fd) -> bool;
    auto wait_for_child(int timeout_ms, int &exit_status) -> bool;
    auto set_error(const std::string &message) -> void { m_last_error = message; }
};

} // namespace chessuci

#endif
