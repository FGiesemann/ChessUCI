/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/gui_handler.h"

namespace chessuci {

UCIGuiHandler::UCIGuiHandler() : m_process{ProcessFactory::create_local()} {
    setup_uci_commands();
}

UCIGuiHandler::~UCIGuiHandler() {
    if (m_running) {
        stop();
    }
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
    if (!m_running.exchange(false)) {
        return;
    }
    m_process->terminate(engine_terminate_timeout);
    if (m_process->is_running()) {
        m_process->kill();
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
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
        } else if (tokens[1] == "author") {
            call(m_id_author_callback, tokens[2]);
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
        auto parse_move = [&]() -> UCIMove {
            if (index + 1 < tokens.size()) {
                const auto move = parse_uci_move(tokens[++index]);
                if (move.has_value()) {
                    return move.value();
                }
                throw UCIError{"Invalid move parameter"};
            }
            throw UCIError{"Missing parameter value"};
        };

        auto parse_move_param = [&](std::optional<UCIMove> &target) -> void { target = parse_move(); };

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
            target_vector = &info.currline->line;
        } else if (token == "pv") {
            target_vector = &info.pv;
        } else if (token == "refutation") {
            target_vector = &info.refutation;
        } else {
            if (target_vector == nullptr) {
                throw UCIError("Invalid info command: unexpected token " + token);
            }
            target_vector->push_back(parse_move());
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
            if (tokens[index + 2] == "lowerbound") {
                info.lowerbound = true;
            } else if (tokens[index + 2] == "upperbound") {
                info.upperbound = true;
            }
        }
    }
    return info;
}

auto UCIGuiHandler::parse_option_command(const TokenList &tokens) -> Option {
    Option option{};
    if (tokens.size() < 5) {
        throw UCIError("Invalid option command: to few arguments");
    }
    if (tokens[1] != "name") {
        throw UCIError("Invalid option command: expected name");
    }
    if (tokens[3] != "type") {
        throw UCIError("Invalid option command: expected type");
    }
    option.name = tokens[2];
    option.type = Option::type_from_string(tokens[4]);
    for (size_t index = 5; index < tokens.size(); ++index) {
        const auto &token = tokens[index];
        if (token == "default") {
            if (index + 1 < tokens.size()) {
                option.default_value = tokens[++index];
            } else {
                throw UCIError("Invalid option command: expected default value");
            }
        } else if (token == "min") {
            parse_int_param(tokens, index, option.min);
        } else if (token == "max") {
            parse_int_param(tokens, index, option.max);
        } else if (token == "var") {
            if (index + 1 < tokens.size()) {
                option.combo_values.push_back(tokens[++index]);
            } else {
                throw UCIError("Invalid option command: expected var value");
            }
        }
    }
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
        throw UCIError{"Missing parameter value"};
    }
}

} // namespace chessuci
