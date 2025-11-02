/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_HANDLER_H
#define CHESSUCI_ENGINE_HANDLER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "chessuci/protocol.h"
#include "chessuci/uci_handler.h"

namespace chessuci {

class UCIEngineHandler : public UCIHandler {
public:
    using UciCallback = std::function<void()>;
    using DebugCallback = std::function<void(bool)>;
    using IsReadyCallback = std::function<void()>;
    using SetOptionCallback = std::function<void(const setoption_command &)>;
    using UCINewGameCallback = std::function<void()>;
    using PositionCallback = std::function<void(const position_command &)>;
    using GoCallback = std::function<void(const go_command &)>;
    using StopCallback = std::function<void()>;
    using PonderHitCallback = std::function<void()>;
    using QuitCallback = std::function<void()>;

    explicit UCIEngineHandler(std::istream &input = std::cin, std::ostream &output = std::cout);
    ~UCIEngineHandler();

    UCIEngineHandler(const UCIEngineHandler &) = delete;
    auto operator=(const UCIEngineHandler &) -> UCIEngineHandler & = delete;

    auto on_uci(UciCallback callback) -> void { m_uci_callback = std::move(callback); }
    auto on_debug(DebugCallback callback) -> void { m_debug_callback = std::move(callback); }
    auto on_isready(IsReadyCallback callback) -> void { m_is_ready_callback = std::move(callback); }
    auto on_setoption(SetOptionCallback callback) -> void { m_set_option_callback = std::move(callback); }
    auto on_ucinewgame(UCINewGameCallback callback) -> void { m_uci_new_game_callback = std::move(callback); }
    auto on_position(PositionCallback callback) -> void { m_position_callback = std::move(callback); }
    auto on_go(GoCallback callback) -> void { m_go_callback = std::move(callback); }
    auto on_stop(StopCallback callback) -> void { m_stop_callback = std::move(callback); }
    auto on_ponderhit(PonderHitCallback callback) -> void { m_ponder_hit_callback = std::move(callback); }
    auto on_quit(QuitCallback callback) -> void { m_quit_callback = std::move(callback); }

    auto start() -> void;
    auto stop() -> void;

    auto send_id(const id_info &info) -> void;
    auto send_option(const Option &option) -> void;
    auto send_uciok() -> void;
    auto send_readyok() -> void;
    auto send_bestmove(const bestmove_info &info) -> void;
    auto send_bestmove(const UCIMove &move, const std::optional<UCIMove> &ponder = std::nullopt) -> void;
    auto send_info(const search_info &info) -> void;
    auto send_info_string(const std::string &message) -> void;
    auto send_raw(const std::string &message) -> void;

    static auto parse_debug_command(const TokenList &tokens) -> bool;
    static auto parse_set_option_command(const TokenList &tokens) -> setoption_command;
    static auto parse_position_command(const TokenList &tokens) -> position_command;
    static auto parse_go_command(const TokenList &tokens) -> go_command;
private:
    std::istream &m_input;
    std::ostream &m_output;

    auto setup_uci_commands() -> void;

    UciCallback m_uci_callback;
    DebugCallback m_debug_callback;
    IsReadyCallback m_is_ready_callback;
    SetOptionCallback m_set_option_callback;
    UCINewGameCallback m_uci_new_game_callback;
    PositionCallback m_position_callback;
    GoCallback m_go_callback;
    StopCallback m_stop_callback;
    PonderHitCallback m_ponder_hit_callback;
    QuitCallback m_quit_callback;

    auto read_loop() -> void;
};

} // namespace chessuci

#endif
