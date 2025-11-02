/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "EngineProcessMock.h"

#include <thread>

#include <iostream>

namespace chessuci::test {

auto EngineProcessMock::when_receives(const std::string &input, ResponseFunction response) -> void {
    m_responses[input] = std::move(response);
}

auto EngineProcessMock::write_line(const std::string &line) -> bool {
    std::cout << "Mock::write_line(): " << line << std::endl;
    auto it = m_responses.find(line);
    if (it != m_responses.end()) {
        std::cout << "Mock::write_line(): Found response for " << line << std::endl;
        const auto responses = it->second(line);
        std::lock_guard<std::mutex> lock{m_queue_mutex};
        for (const auto &response : responses) {
            std::cout << "Mock::write_line(): Adding response " << response << std::endl;
            m_pending_responses.push(response);
        }
    } else {
        std::cout << "Mock::write_line(): No response for " << line << std::endl;
    }
    return true;
}

auto EngineProcessMock::read_line(std::string &line) -> bool {
    std::cout << "Mock::read_line()" << std::endl;
    while (true) {
        std::lock_guard<std::mutex> lock{m_queue_mutex};
        if (!m_pending_responses.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::lock_guard<std::mutex> lock{m_queue_mutex};
    line = std::move(m_pending_responses.front());
    m_pending_responses.pop();
    std::cout << "Mock::read_line(): " << line << std::endl;

    return true;
}

} // namespace chessuci::test
