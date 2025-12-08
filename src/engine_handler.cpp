/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_handler.h"
#include "chessuci/string_conversion.h"

#include <algorithm>
#include <ranges>
#include <sstream>

namespace chessuci {

UCIEngineHandler::UCIEngineHandler(std::istream &input, std::ostream &output) : m_input(input), m_output(output) {
    setup_uci_commands();
}

UCIEngineHandler::~UCIEngineHandler() {
    stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

auto UCIEngineHandler::start() -> void {
    if (m_running.exchange(true)) {
        return;
    }
    m_thread = std::thread([this] -> void { read_loop(); });
}

auto UCIEngineHandler::stop() -> void {
    m_running = false;
}

auto UCIEngineHandler::read_loop() -> void {
    std::string line;
    while (m_running && std::getline(m_input, line)) {
        strip_trailing_whitespace(line);
        if (line.empty()) {
            continue;
        }
        process_line(line);
    }
    m_running = false;
}

auto UCIEngineHandler::send_raw(const std::string &message) -> void {
    std::lock_guard<std::mutex> lock{m_output_mutex};
    // we are using std::endl here, to flush the buffer
    m_output << message << std::endl;
}

auto UCIEngineHandler::send_id(const id_info &info) -> void {
    send_raw("id name " + info.name);
    send_raw("id author " + info.author);
}

auto UCIEngineHandler::send_option(const Option &option) -> void {
    send_raw(option.to_uci_string());
}

auto UCIEngineHandler::send_uciok() -> void {
    send_raw("uciok");
}

auto UCIEngineHandler::send_readyok() -> void {
    send_raw("readyok");
}

auto UCIEngineHandler::send_bestmove(const bestmove_info &info) -> void {
    std::string msg = "bestmove " + to_string(info.bestmove);

    if (info.pondermove.has_value()) {
        msg += " ponder " + to_string(info.pondermove.value());
    }

    send_raw(msg);
}

auto UCIEngineHandler::send_bestmove(const UCIMove &move, const std::optional<UCIMove> &ponder) -> void {
    send_bestmove(bestmove_info{.bestmove = move, .pondermove = ponder});
}

auto UCIEngineHandler::send_info(const search_info &info) -> void {
    std::ostringstream oss;
    oss << "info";

    if (info.depth) {
        oss << " depth " << *info.depth;
    }
    if (info.seldepth) {
        oss << " seldepth " << *info.seldepth;
    }
    if (info.time) {
        oss << " time " << *info.time;
    }
    if (info.nodes) {
        oss << " nodes " << *info.nodes;
    }
    if (info.nps) {
        oss << " nps " << *info.nps;
    }
    if (info.hashfull) {
        oss << " hashfull " << *info.hashfull;
    }
    if (info.tbhits) {
        oss << " tbhits " << *info.tbhits;
    }
    if (info.multipv) {
        oss << " multipv " << *info.multipv;
    }

    if (info.score.has_value()) {
        oss << " score";
        if (info.score->cp) {
            oss << " cp " << *info.score->cp;
        } else if (info.score->mate) {
            oss << " mate " << *info.score->mate;
        }
        if (info.score->lowerbound) {
            oss << " lowerbound";
        } else if (info.score->upperbound) {
            oss << " upperbound";
        }
    }

    if (info.currmove) {
        oss << " currmove " << to_string(*info.currmove);
        if (info.currmovenumber) {
            oss << " currmovenumber " << *info.currmovenumber;
        }
    }

    if (!info.pv.empty()) {
        oss << " pv";
        for (const auto &move : info.pv) {
            oss << " " << to_string(move);
        }
    }

    if (!info.string.empty()) {
        oss << " string " << info.string;
    }

    send_raw(oss.str());
}

auto UCIEngineHandler::send_info_string(const std::string &message) -> void {
    send_raw("info string " + message);
}

auto UCIEngineHandler::setup_uci_commands() -> void {
    m_uci_commands["uci"] = [this](const auto &) { call(m_uci_callback); };
    m_uci_commands["debug"] = [this](const auto &args) { call(m_debug_callback, parse_debug_command(args)); };
    m_uci_commands["isready"] = [this](const auto &) { call(m_is_ready_callback); };
    m_uci_commands["setoption"] = [this](const auto &args) { call(m_set_option_callback, parse_set_option_command(args)); };
    m_uci_commands["ucinewgame"] = [this](const auto &) { call(m_uci_new_game_callback); };
    m_uci_commands["position"] = [this](const auto &args) { call(m_position_callback, parse_position_command(args)); };
    m_uci_commands["go"] = [this](const auto &args) { call(m_go_callback, parse_go_command(args)); };
    m_uci_commands["stop"] = [this](const auto &) { call(m_stop_callback); };
    m_uci_commands["ponderhit"] = [this](const auto &) { call(m_ponder_hit_callback); };
    m_uci_commands["quit"] = [this](const auto &) { call(m_quit_callback); };
}

auto UCIEngineHandler::parse_debug_command(const TokenList &tokens) -> bool {
    if (tokens.size() >= 2) {
        return tokens[1] == "on";
    }
    throw UCIError{"Invalid debug command: expected on or off"};
}

auto UCIEngineHandler::parse_set_option_command(const TokenList &tokens) -> setoption_command {
    setoption_command command;
    if (tokens.size() < 3 || tokens[1] != "name") {
        throw UCIError{"Invalid setoption command: missing token name"};
    }
    command.name = tokens[2];
    std::size_t index = 3;
    while (index < tokens.size() && tokens[index] != "value") {
        command.name += " " + tokens[index++];
    }
    if (command.name.empty()) {
        throw UCIError{"Invalid setoption command: missing name"};
    }
    if (index < tokens.size() && tokens[index] == "value") {
        index++;
        std::string value;
        while (index < tokens.size()) {
            if (!value.empty()) {
                value += " ";
            }
            value += tokens[index++];
        }
        if (value.empty()) {
            throw UCIError{"Invalid setoption command: missing value"};
        }
        command.value = value;
    } else {
        command.value = std::nullopt;
    }
    return command;
}

auto UCIEngineHandler::parse_position_command(const TokenList &tokens) -> position_command {
    position_command command;
    if (tokens.size() < 2) {
        throw UCIError{"Invalid position command: too few arguments"};
    }

    std::size_t index = 1;
    if (tokens[index] == "startpos") {
        command.fen = "startpos";
        ++index;
    } else if (tokens[index] == "fen") {
        ++index;
        if (index >= tokens.size()) {
            throw UCIError{"Invalid position command: FEN string missing"};
        }
        command.fen = tokens[index];
        ++index;
        while (index < tokens.size() && tokens[index] != "moves") {
            command.fen += " " + tokens[index];
            ++index;
        }
    } else {
        throw UCIError{"Invalid position command: expected startpos or fen"};
    }

    if (index < tokens.size() && tokens[index] == "moves") {
        ++index;
        while (index < tokens.size()) {
            const auto move = parse_uci_move(tokens[index]);
            if (move.has_value()) {
                command.moves.push_back(move.value());
            } else {
                throw UCIError{"Invalid position command: invalid move"};
            }
            ++index;
        }
    }

    return command;
}

auto UCIEngineHandler::parse_go_command(const TokenList &tokens) -> go_command {
    go_command command;
    for (std::size_t index = 1; index < tokens.size(); ++index) {
        auto parse_int_param = [&](auto &target) {
            if (index + 1 < tokens.size()) {
                using TargetValueType = typename std::remove_reference_t<decltype(target)>::value_type;
                target = str_to_inttype<TargetValueType>(tokens[++index]);
                if (!target.has_value()) {
                    throw UCIError{"Invalid integer parameter"};
                }
            } else {
                throw UCIError{"Missing parameter value"};
            }
        };

        const auto &token = tokens[index];
        if (token == "depth") {
            parse_int_param(command.depth);
        } else if (token == "nodes") {
            parse_int_param(command.nodes);
        } else if (token == "movetime") {
            parse_int_param(command.movetime);
        } else if (token == "wtime") {
            parse_int_param(command.wtime);
        } else if (token == "btime") {
            parse_int_param(command.btime);
        } else if (token == "winc") {
            parse_int_param(command.winc);
        } else if (token == "binc") {
            parse_int_param(command.binc);
        } else if (token == "movestogo") {
            parse_int_param(command.movestogo);
        } else if (token == "mate") {
            parse_int_param(command.mate);
        } else if (token == "infinite") {
            command.infinite = true;
        } else if (token == "ponder") {
            command.ponder = true;
        } else if (token == "searchmoves") {
            while (index + 1 < tokens.size()) {
                const auto &next = tokens[index + 1];
                if (next == "depth" || next == "nodes" || next == "movetime" || next == "wtime" || next == "btime" || next == "winc" || next == "binc" || next == "movestogo" ||
                    next == "mate" || next == "infinite" || next == "ponder") {
                    break;
                }
                const auto move = parse_uci_move(tokens[++index]);
                if (!move.has_value()) {
                    throw UCIError{"Invalid search move"};
                }
                command.searchmoves.push_back(move.value());
            }
        }
    }
    return command;
}

} // namespace chessuci
