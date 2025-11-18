/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_MOCK_H
#define CHESSUCI_ENGINE_PROCESS_MOCK_H

#include "chessuci/engine_process.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace chessuci::test {

class EngineProcessMock : public chessuci::EngineProcess {
public:
    using ResponseFunction = std::function<std::vector<std::string>(const std::string &)>;

    EngineProcessMock() = default;
    virtual ~EngineProcessMock() override = default;

    auto when_receives(const std::string &input, ResponseFunction response) -> void;

    auto write_line(const std::string &line) -> bool override;
    auto read_line(std::string &line) -> bool override;

    auto start(const ProcessParams &) -> bool override {
        m_running = true;
        return true;
    }
    auto terminate(int) -> bool override {
        kill();
        return true;
    }

    auto is_running() const -> bool override { return m_running; }
    auto pid() const -> proc_id_t override { return 12; }
    auto kill() -> void override { m_running = false; }
    auto wait_for_exit(int) -> std::optional<int> override { return 0; }
    auto can_read() const -> bool override;
    auto last_error() const -> const std::string & override { return m_last_error; }
private:
    std::unordered_map<std::string, ResponseFunction> m_responses;
    mutable std::mutex m_queue_mutex;
    std::queue<std::string> m_pending_responses;

    std::atomic<bool> m_running{false};
    std::string m_last_error{};
};

} // namespace chessuci::test

#endif
