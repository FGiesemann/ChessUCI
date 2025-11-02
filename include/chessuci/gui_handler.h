/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_GUI_HANDLER_H
#define CHESSUCI_GUI_HANDLER_H

#include <memory>
#include <thread>

#include "chessuci/engine_process.h"
#include "chessuci/process_factory.h"
#include "chessuci/protocol.h"
#include "chessuci/uci_handler.h"

namespace chessuci {

class UCIGuiHandler : public UCIHandler {
public:
    using IdNameCallback = std::function<void(const std::string &)>;
    using IdAuthorCallback = std::function<void(const std::string &)>;
    using UCIOkCallback = std::function<void()>;
    using ReadyokCallback = std::function<void()>;
    using BestmoveCallback = std::function<void(const bestmove_info &)>;
    using InfoCallback = std::function<void(const search_info &)>;
    using OptionCallback = std::function<void(const Option &)>;

    explicit UCIGuiHandler();
    ~UCIGuiHandler();

    auto on_id_name(IdNameCallback callback) -> void { m_id_name_callback = std::move(callback); }
    auto on_id_author(IdAuthorCallback callback) -> void { m_id_author_callback = std::move(callback); }
    auto on_uciok(UCIOkCallback callback) -> void { m_uciok_callback = std::move(callback); }
    auto on_readyok(ReadyokCallback callback) -> void { m_readyok_callback = std::move(callback); }
    auto on_bestmove(BestmoveCallback callback) -> void { m_bestmove_callback = std::move(callback); }
    auto on_info(InfoCallback callback) -> void { m_info_callback = std::move(callback); }
    auto on_option(OptionCallback callback) -> void { m_option_callback = std::move(callback); }

    auto send_uci() -> bool;
    auto send_debug(bool on) -> bool;
    auto send_isready() -> bool;
    auto send_ucinewgame() -> bool;
    auto send_position(const position_command &command) -> bool;
    auto send_go(const go_command &command) -> bool;
    auto send_stop() -> bool;
    auto send_ponderhist() -> bool;
    auto send_quit() -> bool;
    auto send_raw(const std::string &message) -> bool;

    auto start(const ProcessParams &params) -> bool;
    auto stop() -> void;

    static auto parse_bestmove_command(const TokenList &tokens) -> bestmove_info;
    static auto parse_info_command(const TokenList &tokens) -> search_info;
    static auto parse_option_command(const TokenList &tokens) -> Option;
    static auto parse_score(const TokenList &tokens, size_t index) -> score_info;

    static auto parse_int_param(const TokenList &tokens, size_t index, std::optional<int> &target) -> void;
    static auto collect_string(const TokenList &tokens, size_t index) -> std::string;
private:
    static constexpr int engine_terminate_timeout{3000};

    std::unique_ptr<EngineProcess> m_process;

    // dispatches different "id" messages to corresponding callbacks
    auto handle_id_message(const TokenList &tokens) const -> void;
    IdNameCallback m_id_name_callback;
    IdAuthorCallback m_id_author_callback;
    UCIOkCallback m_uciok_callback;
    ReadyokCallback m_readyok_callback;
    BestmoveCallback m_bestmove_callback;
    InfoCallback m_info_callback;
    OptionCallback m_option_callback;

    auto setup_uci_commands() -> void;
    auto read_loop() -> void;
};

} // namespace chessuci

#endif
