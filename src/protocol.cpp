/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/protocol.h"

#include <sstream>

namespace chessuci {

namespace {

auto add_bool_flag(std::string &line, const std::string &name, bool flag) -> void {
    if (flag) {
        line += " " + name;
    }
}

template<typename T>
auto add_optional_value(std::string &line, const std::string &name, const std::optional<T> &value) -> void {
    if (value.has_value()) {
        using std::to_string;
        line += " " + name + " " + to_string(value.value());
    }
}

auto add_move_list(std::string &line, const std::vector<UCIMove> &moves, const std::string &name = "") -> void {
    if (!moves.empty()) {
        line += " " + name;
    }
    for (const auto &move : moves) {
        line += " " + to_string(move);
    }
}

} // namespace

auto to_string(const position_command &command) -> std::string {
    std::string message{"position "};
    if (command.fen != position_command::startpos) {
        message += "fen " + command.fen;
    }
    if (!command.moves.empty()) {
        message += " moves";
        add_move_list(message, command.moves);
    }
    return message;
}

const std::string position_command::startpos{"startpos"};

auto to_string(const go_command &command) -> std::string {
    std::string message{"go"};
    add_move_list(message, command.searchmoves, "searchmoves");
    add_bool_flag(message, "ponder", command.ponder);
    add_optional_value(message, "wtime", command.wtime);
    add_optional_value(message, "btime", command.btime);
    add_optional_value(message, "winc", command.winc);
    add_optional_value(message, "binc", command.binc);
    add_optional_value(message, "movestogo", command.movestogo);
    add_optional_value(message, "depth", command.depth);
    add_optional_value(message, "nodes", command.nodes);
    add_optional_value(message, "mate", command.mate);
    add_optional_value(message, "movetime", command.movetime);
    add_bool_flag(message, "infinite", command.infinite);

    return message;
}

auto go_command::has_timing_control() const -> bool {
    return wtime.has_value() || btime.has_value() || movetime.has_value() || infinite;
}

auto to_string(const search_info &info) -> std::string {
    std::string message{"info"};
    add_optional_value(message, "depth", info.depth);
    add_optional_value(message, "seldepth", info.seldepth);
    add_optional_value(message, "time", info.time);
    add_optional_value(message, "nodes", info.nodes);
    add_optional_value(message, "multipv", info.multipv);
    add_optional_value(message, "score", info.score);
    add_optional_value(message, "currmove", info.currmove);
    add_optional_value(message, "currmovenumber", info.currmovenumber);
    add_optional_value(message, "hashfull", info.hashfull);
    add_optional_value(message, "nps", info.nps);
    add_optional_value(message, "tbhits", info.tbhits);
    add_optional_value(message, "sbhits", info.sbhits);
    add_optional_value(message, "cpuload", info.cpuload);
    if (!info.string.empty()) {
        message += " string " + info.string;
    }
    add_move_list(message, info.pv, "pv");
    add_move_list(message, info.refutation, "refutation");
    add_optional_value(message, "currline", info.currline);
    return message;
}

auto to_string(const score_info &score) -> std::string {
    std::string message{"score"};
    add_optional_value(message, "cp", score.cp);
    add_optional_value(message, "mate", score.mate);
    add_bool_flag(message, "lowerbound", score.lowerbound);
    add_bool_flag(message, "upperbound", score.upperbound);
    return message;
}

auto to_string(const line_info &info) -> std::string {
    std::string message{"line"};
    add_optional_value(message, "cpunr", info.cpunr);
    add_move_list(message, info.line);
    return message;
}

auto Option::to_uci_string() const -> std::string {
    std::ostringstream oss;
    oss << "option ";
    oss << "name " << name;
    oss << "type " << type_to_string();
    if (default_value.has_value()) {
        oss << "default " << *default_value;
    }
    if (min.has_value()) {
        oss << "min " << *min;
    }
    if (max.has_value()) {
        oss << "max " << *max;
    }
    for (const auto &value : combo_values) {
        oss << "var " << value;
    }
    return oss.str();
}

auto Option::type_to_string() const -> std::string {
    switch (type) {
    case Type::Button:
        return "button";
    case Type::Check:
        return "check";
    case Type::Combo:
        return "combo";
    case Type::Spin:
        return "spin";
    case Type::String:
        return "string";
    default:
        return "<unknown>";
    }
}

auto Option::type_from_string(const std::string &str) -> Type {
    if (str == "button") {
        return Type::Button;
    } else if (str == "check") {
        return Type::Check;
    } else if (str == "combo") {
        return Type::Combo;
    } else if (str == "spin") {
        return Type::Spin;
    } else if (str == "string") {
        return Type::String;
    } else {
        throw UCIError("unknown option type: " + str);
    }
}

} // namespace chessuci
