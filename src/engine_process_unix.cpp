/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_process_unix.h"

#include <fcntl.h>
#include <sys/wait.h>

namespace chessuci {

EngineProcessUnix::~EngineProcessUnix() {
    if (is_running()) {
        terminate(1000);
        if (is_running()) {
            kill();
        }
    }
}

auto EngineProcessUnix::start(const ProcessParams &params) -> bool {
    if (is_running()) {
        set_error("Process already running");
        return false;
    }

    if (!create_pipes()) {
        close_pipes();
        return false;
    }

    if (!create_child_process(params)) {
        close_pipes();
        return false;
    }

    m_std_in.close_read();
    m_std_out.close_write();
    m_std_err.close_write();

    if (!set_non_blocking(m_std_out.read())) {
        set_error("Failed to set stdout pipe non-blocking: ");
        kill();
        return false;
    }

    m_running = true;

    usleep(10000);

    int status;
    pid_t result = waitpid(m_pid, &status, WNOHANG);
    if (result == m_pid) {
        m_running = false;
        if (WIFEXITED(status)) {
            m_stored_exit_code = WEXITSTATUS(status);
            if (m_stored_exit_code != 0) {
                set_error("Process exited immediately with code " + std::to_string(WEXITSTATUS(status)));
                close_pipes();
                return false;
            }
        } else {
            set_error("Process terminated immediately by signal");
            close_pipes();
            return false;
        }
    }

    return true;
}

auto EngineProcessUnix::is_running() const -> bool {
    if (!m_running || m_pid == -1) {
        return false;
    }

    int status;
    pid_t result = waitpid(m_pid, &status, WNOHANG);
    if (result == m_pid) {
        m_running = false;
        if (WIFEXITED(status)) {
            m_stored_exit_code = WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            m_stored_exit_code = WTERMSIG(status);
        }
        return false;
    }
    if (result == -1) {
        m_running = false;
        m_stored_exit_code = -1;
        return false;
    }

    return true;
}

auto EngineProcessUnix::pid() const -> proc_id_t {
    return m_pid;
}

auto EngineProcessUnix::terminate(int timeout_ms) -> bool {
    if (!is_running()) {
        return true;
    }

    write_line("quit");
    int exit_status{};
    if (wait_for_child(timeout_ms, exit_status)) {
        m_running = false;
        close_pipes();
        return true;
    }

    return false;
}

auto EngineProcessUnix::kill() -> void {
    if (m_pid == -1) {
        return;
    }

    ::kill(m_pid, SIGKILL);

    int status;
    waitpid(m_pid, &status, 0);
    m_running = false;
    close_pipes();
}

auto EngineProcessUnix::wait_for_exit(int timeout_ms) -> std::optional<int> {
    if (m_pid == -1) {
        return std::nullopt;
    }

    if (!m_running) {
        return m_stored_exit_code;
    }

    int exit_status;
    if (wait_for_child(timeout_ms, exit_status)) {
        m_running = false;
        close_pipes();
        if (WIFEXITED(exit_status)) {
            return WEXITSTATUS(exit_status);
        }
        if (WIFSIGNALED(exit_status)) {
            return WTERMSIG(exit_status);
        }

        return -1;
    }

    return std::nullopt;
}

auto EngineProcessUnix::write_line(const std::string &line) -> bool {
    if (!is_running()) {
        set_error("Process not running");
        return false;
    }

    std::string message = line + "\n";
    const char *data = message.c_str();
    size_t remaining = message.size();

    while (remaining > 0) {
        auto written = write(m_std_in.write(), data, remaining);
        if (written == -1) {
            if (errno == EINTR) {
                continue; // Interrupted, try again
            }
            set_error(std::string{"Write failes: "} + strerror(errno));
            return false;
        }

        data += written;
        remaining -= static_cast<size_t>(written);
    }

    return true;
}

auto EngineProcessUnix::read_line(std::string &line) -> bool {
    std::size_t newline_pos = m_read_buffer.find('\n');
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
        ssize_t bytes_read = read(m_std_out.read(), buffer, sizeof(buffer));
        if (bytes_read > 0) {
            m_read_buffer.append(buffer, static_cast<size_t>(bytes_read));

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
            set_error("Process closed stdout");
            return false;
        } else {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            set_error(std::string{"Read failes: "} + strerror(errno));
            return false;
        }
    }
}

auto EngineProcessUnix::can_read() const -> bool {
    if (m_read_buffer.find('\n') != std::string::npos) {
        return true;
    }
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(m_std_out.read(), &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking

    int result = select(m_std_out.read() + 1, &read_fds, nullptr, nullptr, &timeout);

    return result > 0;
}

auto EngineProcessUnix::last_error() const -> const std::string & {
    return m_last_error;
}

auto EngineProcessUnix::close_pipes() -> void {
    m_std_in.close_read();
    m_std_in.close_write();
    m_std_out.close_read();
    m_std_out.close_write();
    m_std_err.close_read();
    m_std_err.close_write();
}

auto EngineProcessUnix::set_non_blocking(int fd) -> bool {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

auto EngineProcessUnix::wait_for_child(int timeout_ms, int &exit_status) -> bool {
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        pid_t result = waitpid(m_pid, &exit_status, WNOHANG);

        if (result == m_pid) {
            return true;
        } else if (result == -1) {
            return false;
        }

        if (timeout_ms > 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

            if (elapsed >= timeout_ms) {
                return false;
            }
        } else if (timeout_ms == 0) {
            return false;
        }

        usleep(10000);
    }
}

auto EngineProcessUnix::create_pipes() -> bool {
    if (!m_std_in.create()) {
        set_error(std::string{"Failed to create stdin pipe: "} + strerror(errno));
        return false;
    }
    if (!m_std_out.create()) {
        set_error(std::string{"Failed to create stdout pipe: "} + strerror(errno));
        return false;
    }
    if (!m_std_err.create()) {
        set_error(std::string{"Failed to create stderr pipe: "} + strerror(errno));
        return false;
    }
    return true;
}

auto EngineProcessUnix::create_child_process(const ProcessParams &params) -> bool {
    m_pid = fork();
    if (m_pid == -1) {
        set_error(std::string{"Failed to fork: "} + strerror(errno));
        return false;
    }

    if (m_pid == 0) {
        dup2(m_std_in.read(), STDIN_FILENO);
        m_std_in.close_read();
        m_std_in.close_write();
        dup2(m_std_out.write(), STDOUT_FILENO);
        m_std_out.close_read();
        m_std_out.close_write();
        dup2(m_std_err.write(), STDERR_FILENO);
        m_std_err.close_read();
        m_std_err.close_write();

        if (params.working_directory.has_value()) {
            if (chdir(params.working_directory->c_str()) == -1) {
                _exit(127);
            }
        }

        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(params.executable.c_str()));
        for (const auto &arg : params.arguments) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(params.executable.c_str(), argv.data());
        _exit(127);
    }

    return true;
}

} // namespace chessuci
