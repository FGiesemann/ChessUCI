/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_PROTOCOL_H
#define CHESSUCI_PROTOCOL_H

#include <optional>
#include <stdexcept>
#include <vector>

#include "chessuci/move.h"

namespace chessuci {

class UCIError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct debug_command {
    bool enable_debugging;
};

struct setoption_command {
    std::string name;
    std::optional<std::string> value; // not set is not the same as empty string
};

struct position_command {
    std::string fen; // Could also be 'startpos'
    std::vector<UCIMove> moves;

    static const std::string startpos;
};

auto to_string(const position_command &command) -> std::string;

struct go_command {
    std::vector<UCIMove> searchmoves;
    bool ponder = false;
    std::optional<std::int64_t> wtime;
    std::optional<std::int64_t> btime;
    std::optional<int> winc;
    std::optional<int> binc;
    std::optional<int> movestogo;
    std::optional<std::uint64_t> depth;
    std::optional<std::uint64_t> nodes;
    std::optional<int> mate;
    std::optional<std::int64_t> movetime;
    bool infinite = false;

    auto has_timing_control() const -> bool;
};

auto to_string(const go_command &command) -> std::string;

struct id_info {
    std::string name;
    std::string author;
};

struct bestmove_info {
    UCIMove bestmove;
    std::optional<UCIMove> pondermove;
};

struct score_info {
    std::optional<int> cp;
    std::optional<int> mate;
    bool lowerbound;
    bool upperbound;
};

struct line_info {
    std::optional<int> cpunr;
    std::vector<UCIMove> line;
};

struct search_info {
    std::optional<int> depth;
    std::optional<int> seldepth;
    std::optional<int> time;
    std::optional<std::uint64_t> nodes;
    std::vector<UCIMove> pv;
    std::optional<int> multipv;
    std::optional<score_info> score;
    std::optional<UCIMove> currmove;
    std::optional<int> currmovenumber;
    std::optional<int> hashfull;
    std::optional<int> nps;
    std::optional<int> tbhits;
    std::optional<int> sbhits;
    std::optional<int> cpuload;
    std::string string;
    std::vector<UCIMove> refutation;
    std::optional<line_info> currline;
};

struct Option {
    enum class Type { Check, Spin, Combo, Button, String };

    std::string name;
    Type type;
    std::optional<std::string> default_value;
    std::optional<int> min;
    std::optional<int> max;
    std::vector<std::string> combo_values;

    auto to_uci_string() const -> std::string;
    auto type_to_string() const -> std::string;
    static auto type_from_string(const std::string &str) -> Type;
};

using TokenList = std::vector<std::string>;

} // namespace chessuci

#endif
