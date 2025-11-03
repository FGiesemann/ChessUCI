/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/gui_handler.h"

namespace chessuci {

UCIGuiHandler::UCIGuiHandler() : m_process{ProcessFactory::create_local()} {
    setup_uci_commands();
}

UCIGuiHandler::UCIGuiHandler(std::unique_ptr<EngineProcess> process) : m_process{std::move(process)} {
    setup_uci_commands();
}

UCIGuiHandler::~UCIGuiHandler() {
    stop();
}

auto UCIGuiHandler::start(const ProcessParams &params) -> bool {
    if (m_running.exchange(true)) {
        return false;
    }
    auto result = m_process->start(params);
    if (result) {
        m_thread = std::thread([this] { read_loop(); });
    } else {
        m_running = false;
    }
    return result;
}

auto UCIGuiHandler::stop() -> void {
    m_running = false;
    m_process->terminate(engine_terminate_timeout);
    if (m_process->is_running()) {
        m_process->kill();
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

auto UCIGuiHandler::send_uci() -> bool {
    return send_raw("uci");
}

auto UCIGuiHandler::send_debug(bool on) -> bool {
    return send_raw(on ? "debug on" : "debug off");
}

auto UCIGuiHandler::send_isready() -> bool {
    return send_raw("isready");
}

auto UCIGuiHandler::send_ucinewgame() -> bool {
    return send_raw("ucinewgame");
}

auto UCIGuiHandler::send_position(const position_command &command) -> bool {
    return send_raw(to_string(command));
}

auto UCIGuiHandler::send_go(const go_command &command) -> bool {
    return send_raw(to_string(command));
}

auto UCIGuiHandler::send_stop() -> bool {
    return send_raw("stop");
}

auto UCIGuiHandler::send_ponderhist() -> bool {
    return send_raw("ponderhist");
}

auto UCIGuiHandler::send_quit() -> bool {
    return send_raw("quit");
}

auto UCIGuiHandler::send_raw(const std::string &message) -> bool {
    std::lock_guard<std::mutex> lock{m_output_mutex};
    return m_process->write_line(message);
}

auto UCIGuiHandler::read_loop() -> void {
    std::string line;
    while (m_running && m_process->read_line(line)) {
        strip_trailing_whitespace(line);
        if (line.empty()) {
            continue;
        }
        process_line(line);
    }
    m_running = false;
}

auto UCIGuiHandler::setup_uci_commands() -> void {
    m_uci_commands["id"] = [this](const auto &args) -> void { handle_id_message(args); };
    m_uci_commands["uciok"] = [this](const auto &) -> void { call(m_uciok_callback); };
    m_uci_commands["readyok"] = [this](const auto &) -> void { call(m_readyok_callback); };
    m_uci_commands["bestmove"] = [this](const auto &args) -> void { call(m_bestmove_callback, parse_bestmove_command(args)); };
    m_uci_commands["info"] = [this](const auto &args) -> void { call(m_info_callback, parse_info_command(args)); };
    m_uci_commands["option"] = [this](const auto &args) -> void { call(m_option_callback, parse_option_command(args)); };
}

auto UCIGuiHandler::handle_id_message(const TokenList &tokens) const -> void {
    if (tokens.size() > 2) {
        if (tokens[1] == "name") {
            call(m_id_name_callback, tokens[2]);
            return;
        } else if (tokens[1] == "author") {
            call(m_id_author_callback, tokens[2]);
            return;
        }
    }
    throw UCIError("Invalid id command: expected name or author");
}

auto UCIGuiHandler::parse_bestmove_command(const TokenList &tokens) -> bestmove_info {
    bestmove_info info{};
    if (tokens.size() > 1) {
        const auto move = parse_uci_move(tokens[1]);
        if (move.has_value()) {
            info.bestmove = move.value();
        } else {
            throw UCIError("Invalid bestmove command: invalid best move " + tokens[1]);
        }
    }
    if (tokens.size() > 3) {
        if (tokens[2] != "ponder") {
            throw UCIError("Invalid bestmove command: expected ponder");
        }
        const auto move = parse_uci_move(tokens[3]);
        if (move.has_value()) {
            info.pondermove = move.value();
        } else {
            throw UCIError("Invalid bestmove command: invalid ponder move " + tokens[3]);
        }
    }
    return info;
}

auto UCIGuiHandler::parse_info_command(const TokenList &tokens) -> search_info {
    search_info info{};
    std::vector<UCIMove> *target_vector{nullptr};
    for (size_t index = 1; index < tokens.size(); ++index) {
        auto parse_move_param = [&](std::optional<UCIMove> &target) -> void {
            if (index + 1 < tokens.size()) {
                const auto move = parse_uci_move(tokens[++index]);
                if (move.has_value()) {
                    target = move.value();
                }
                throw UCIError{"Invalid move parameter"};
            }
            throw UCIError{"Missing move parameter"};
        };

        const auto &token = tokens[index];
        if (token == "depth") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.depth);
        } else if (token == "seldepth") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.seldepth);
        } else if (token == "time") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.time);
        } else if (token == "nodes") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.nodes);
        } else if (token == "multipv") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.multipv);
        } else if (token == "score") {
            target_vector = nullptr;
            info.score = parse_score(tokens, index);
        } else if (token == "currmove") {
            target_vector = nullptr;
            parse_move_param(info.currmove);
        } else if (token == "currmovenumber") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.currmovenumber);
        } else if (token == "hashfull") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.hashfull);
        } else if (token == "nps") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.nps);
        } else if (token == "tbhits") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.tbhits);
        } else if (token == "sbhits") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.sbhits);
        } else if (token == "cpuload") {
            target_vector = nullptr;
            parse_int_param(tokens, index, info.cpuload);
        } else if (token == "currline") {
            info.currline = line_info{};
            parse_int_param(tokens, index, info.currline->cpunr);
            ++index;
            target_vector = &info.currline->line;
        } else if (token == "pv") {
            target_vector = &info.pv;
        } else if (token == "refutation") {
            target_vector = &info.refutation;
        } else if (token == "string") {
            target_vector = nullptr;
            info.string = collect_string(tokens, index + 1);
        } else {
            if (target_vector == nullptr) {
                continue;
            }
            const auto move = parse_uci_move(token);
            if (move.has_value()) {
                target_vector->push_back(move.value());
            } else {
                throw UCIError{"Invalid info command: move expected, but found " + token};
            }
        }
    }
    return info;
}

auto UCIGuiHandler::parse_score(const TokenList &tokens, size_t index) -> score_info {
    score_info info{};
    if (index + 1 < tokens.size()) {
        if (tokens[index + 1] == "cp") {
            parse_int_param(tokens, index + 1, info.cp);
        } else if (tokens[index + 1] == "mate") {
            parse_int_param(tokens, index + 1, info.mate);
        }

        if (index + 3 < tokens.size()) {
            if (tokens[index + 3] == "lowerbound") {
                info.lowerbound = true;
            } else if (tokens[index + 3] == "upperbound") {
                info.upperbound = true;
            }
        }
    }
    return info;
}

namespace {

enum class OptionItem { name, type, default_value, min_value, max_value, var_value, unknown };

auto process_option_item(OptionItem &item_type, std::string &value, Option &option) -> void {
    if (value.empty() || item_type == OptionItem::unknown) {
        return;
    }

    switch (item_type) {
    case OptionItem::name:
        option.name = value;
        break;
    case OptionItem::type:
        option.type = Option::type_from_string(value);
        break;
    case OptionItem::default_value:
        option.default_value = value;
        break;
    case OptionItem::min_value:
        option.min = std::stoi(value);
        break;
    case OptionItem::max_value:
        option.max = std::stoi(value);
        break;
    case OptionItem::var_value:
        option.combo_values.push_back(value);
        break;
    default:
        break;
    }

    item_type = OptionItem::unknown;
    value.clear();
}

} // namespace

auto UCIGuiHandler::parse_option_command(const TokenList &tokens) -> Option {
    Option option{};
    OptionItem current_item{OptionItem::unknown};
    std::string collected_tokens{};
    for (size_t index = 1; index < tokens.size(); ++index) {
        const auto &token = tokens[index];
        if (token == "name") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::name;
        } else if (token == "type") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::type;
        } else if (token == "default") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::default_value;
        } else if (token == "min") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::min_value;
        } else if (token == "max") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::max_value;
        } else if (token == "var") {
            process_option_item(current_item, collected_tokens, option);
            current_item = OptionItem::var_value;
        } else {
            if (collected_tokens.empty()) {
                collected_tokens = token;
            } else {
                collected_tokens += " " + token;
            }
        }
    }
    process_option_item(current_item, collected_tokens, option);
    return option;
}

auto UCIGuiHandler::parse_int_param(const TokenList &tokens, size_t index, std::optional<int> &target) -> void {
    if (index + 1 < tokens.size()) {
        try {
            target = std::stoi(tokens[++index]);
        } catch (...) {
            throw UCIError{"Invalid integer parameter"};
        }
    } else {
        throw UCIError{"Missing integer parameter"};
    }
}

auto UCIGuiHandler::collect_string(const TokenList &tokens, size_t index) -> std::string {
    std::ostringstream oss{};
    if (index < tokens.size()) {
        oss << tokens[index];
    }
    for (size_t i = index + 1; i < tokens.size(); ++i) {
        oss << " " << tokens[i];
    }
    return oss.str();
}

} // namespace chessuci
