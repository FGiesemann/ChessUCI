/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "EngineProcessMock.h"

#include <thread>

namespace chessuci::test {

auto EngineProcessMock::when_receives(const std::string &input, ResponseFunction response) -> void {
    m_responses[input] = std::move(response);
}

auto EngineProcessMock::write_line(const std::string &line) -> bool {
    auto it = m_responses.find(line);
    if (it != m_responses.end()) {
        const auto responses = it->second(line);
        std::lock_guard<std::mutex> lock{m_queue_mutex};
        for (const auto &response : responses) {
            m_pending_responses.push(response);
        }
    }
    return true;
}

auto EngineProcessMock::can_read() const -> bool {
    std::lock_guard<std::mutex> lock{m_queue_mutex};
    return !m_pending_responses.empty();
}

auto EngineProcessMock::read_line(std::string &line) -> bool {
    while (m_running) {
        if (can_read()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!m_running) {
        return false;
    }
    std::lock_guard<std::mutex> lock{m_queue_mutex};
    line = std::move(m_pending_responses.front());
    m_pending_responses.pop();

    return true;
}

} // namespace chessuci::test
