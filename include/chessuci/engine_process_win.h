/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_WIN_H
#define CHESSUCI_ENGINE_PROCESS_WIN_H

#include "chessuci/engine_process.h"

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
};

} // namespace chessuci

#endif
