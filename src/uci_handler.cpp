/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/uci_handler.h"

#include <ranges>
#include <sstream>

namespace chessuci {

auto UCIHandler::register_command(const std::string &command, CustomCommandCallback callback) -> void {
    std::lock_guard<std::mutex> lock{m_custom_commands_mutex};
    m_custom_commands[command] = std::move(callback);
}

auto UCIHandler::unregister_command(const std::string &command) -> void {
    std::lock_guard<std::mutex> lock{m_custom_commands_mutex};
    m_custom_commands.erase(command);
}

auto UCIHandler::process_line(const std::string &line) -> void {
    const auto tokens = tokenize(line);
    if (tokens.empty()) {
        return;
    }
    const auto &command = tokens[0];
    const auto command_it = m_uci_commands.find(command);
    if (command_it != m_uci_commands.end()) {
        command_it->second(tokens);
        return;
    }

    {
        std::lock_guard<std::mutex> lock{m_custom_commands_mutex};
        auto custom_it = m_custom_commands.find(command);
        if (custom_it != m_custom_commands.end()) {
            custom_it->second(tokens);
            return;
        }
    }

    call(m_unknown_command_callback, tokens);
}

auto UCIHandler::strip_trailing_whitespace(std::string &line) -> void {
    line.erase(std::ranges::find_if(std::ranges::reverse_view(line), [](int chr) { return !std::isspace(chr); }).base(), line.end());
}

auto UCIHandler::tokenize(const std::string &line) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

} // namespace chessuci
