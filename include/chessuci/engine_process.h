/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_H
#define CHESSUCI_ENGINE_PROCESS_H

#include <filesystem>
#include <optional>
#include <vector>

namespace chessuci {

using optional_path = std::optional<std::filesystem::path>;

/**
 * \brief Parameters needed to start an engine process.
 */
struct ProcessParams {
    std::filesystem::path executable;     ///< Path to the executable
    std::vector<std::string> arguments{}; ///< List of arguments
    optional_path working_directory{};    ///< Optional working directory
};

class EngineProcess {
public:
    virtual ~EngineProcess() = default;

    using proc_id_t = int;

    /**
     * \brief Start a new process.
     *
     * Starts a new engine process with the given parameters. If a process is
     * already running, nothing happens. In order to restart an engine process,
     * call terminate() first.
     * \param params Paramters for the process.
     * \return `true`, if the process was started successfully; `false`, if it
     *   could not be started.
     */
    virtual auto start(const ProcessParams &params) -> bool = 0;

    /**
     * \brief Check if the process is running.
     *
     * Checks if the engine process is (still) running.
     * \return If the process is running.
     */
    virtual auto is_running() const -> bool = 0;

    /**
     * \brief Get the process id.
     *
     * Returns the id of the engine process.
     * \return Process id of the engine.
     */
    virtual auto pid() const -> proc_id_t = 0;

    /**
     * \brief Terminate the process gracefully.
     *
     * Tries to terminate the engine process gracefully by sending the `quit`
     * command first. After the timeout is reached, the process is killed.
     * \param timeout_ms Timeout in milliseconds for graceful termination.
     * \return `true`, if the process was terminated gracefully; `false`, if it
     *   had to be killed.
     */
    virtual auto terminate(int timeout_ms = 3000) -> bool = 0;

    /**
     * \brief Kill the process immediately.
     */
    virtual auto kill() -> void = 0;

    /**
     * \brief Wait for the engine process to exit.
     *
     * If the engine process exits gracefully, the exit code is returned. If the
     * process is still running or is aborted by a signal, `std::nullopt` is
     * returned.
     * \param timeout_ms Timeout in milliseconds to wait for the process to
     *   exit.
     * \return Exit code of the engine process, or `std::nullopt`.
     */
    virtual auto wait_for_exit(int timeout_ms = 0) -> std::optional<int> = 0;

    /**
     * \brief Send a line of text to the engine process.
     *
     * \param line The text to send.
     * \return If the write was successful.
     */
    virtual auto write_line(const std::string &line) -> bool = 0;

    /**
     * \brief Read a line of text from the engine process.
     *
     * This call blocks, until a full line can be read from the engine process.
     * \param line The line will be stored here.
     * \return If the read was successful.
     */
    virtual auto read_line(std::string &line) -> bool = 0;

    /**
     * \brief Check, if data can be read from the engine process.
     *
     * Cheks, if data is available to read from the engine process. This call
     * does not block execution.
     * \return If data is available.
     */
    virtual auto can_read() const -> bool = 0;

    /**
     * \brief Return the last error message.
     *
     * The last error that occured in the communication with the engine process
     * can be retrieved with this function.
     * \return The latest error message.
     */
    virtual auto last_error() const -> const std::string & = 0;
};

} // namespace chessuci

#endif
