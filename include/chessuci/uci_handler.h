/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_UCI_HANDLER_H
#define CHESSUCI_UCI_HANDLER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace chessuci {

class UCIHandler {
public:
    using TokenList = std::vector<std::string>;
    using CustomCommandCallback = std::function<void(const TokenList &)>;
    using UnknownCommandCallback = std::function<void(const TokenList &)>;

    auto is_running() const -> bool { return m_running; }

    auto register_command(const std::string &command, CustomCommandCallback callback) -> void;
    auto unregister_command(const std::string &command) -> void;
    auto on_unknown_command(UnknownCommandCallback callback) -> void { m_unknown_command_callback = std::move(callback); }

    static auto strip_trailing_whitespace(std::string &line) -> void;
    static auto tokenize(const std::string &line) -> TokenList;

    auto process_line(const std::string &line) -> void;
protected:
    std::atomic<bool> m_running{false};
    std::mutex m_custom_commands_mutex;
    std::unordered_map<std::string, CustomCommandCallback> m_custom_commands;
    std::mutex m_output_mutex;
    std::thread m_thread;

    UnknownCommandCallback m_unknown_command_callback;
    using CommandHandler = std::function<void(const TokenList &)>;
    std::unordered_map<std::string, CommandHandler> m_uci_commands;

    template<typename C, typename... Args>
    auto call(const C &callback, Args &&...args) const -> void {
        if (callback) {
            callback(std::forward<Args>(args)...);
        }
    }
};

} // namespace chessuci

#endif
