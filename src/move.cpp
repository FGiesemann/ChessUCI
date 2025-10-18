/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/move.h"

#include <ranges>

namespace chessuci {

namespace {
constexpr size_t min_uci_move_length{4};
constexpr size_t max_uci_move_length{5};

auto is_valid_file(char file) -> bool {
    return file >= 'a' && file <= 'h';
}

auto is_valid_rank(char rank) -> bool {
    return rank >= '1' && rank <= '8';
}

auto is_valid_promotion_piece(char piece) -> bool {
    return piece == 'q' || piece == 'r' || piece == 'b' || piece == 'n';
}
} // namespace

auto parse_uci_move(const std::string &uci_str) -> std::expected<UCIMove, UCIParserError> {
    if (uci_str.length() < min_uci_move_length) {
        return std::unexpected{UCIParserError{.type = UCIParserErrorType::MissingData, .uci_str = uci_str}};
    }
    if (uci_str.length() > max_uci_move_length) {
        return std::unexpected{UCIParserError{.type = UCIParserErrorType::UnexpectedToken, .uci_str = uci_str}};
    }
    if (!is_valid_file(uci_str[0]) || !is_valid_file(uci_str[2])) {
        return std::unexpected{UCIParserError{.type = UCIParserErrorType::InvalidFile, .uci_str = uci_str}};
    }
    if (!is_valid_rank(uci_str[1]) || !is_valid_rank(uci_str[3])) {
        return std::unexpected{UCIParserError{.type = UCIParserErrorType::InvalidRank, .uci_str = uci_str}};
    }
    if (uci_str.length() == max_uci_move_length && !is_valid_promotion_piece(uci_str[4])) {
        return std::unexpected{UCIParserError{.type = UCIParserErrorType::InvalidPromotionPiece, .uci_str = uci_str}};
    }

    chesscore::Square from = chesscore::Square{chesscore::File{uci_str[0]}, chesscore::Rank{uci_str[1] - '0'}};
    chesscore::Square to = chesscore::Square{chesscore::File{uci_str[2]}, chesscore::Rank{uci_str[3] - '0'}};
    std::optional<chesscore::PieceType> promotion_piece{std::nullopt};
    if (uci_str.length() == max_uci_move_length) {
        promotion_piece = chesscore::piece_type_from_char(uci_str[4]);
    }

    return UCIMove{from, to, promotion_piece};
}

auto uci_move_matches(const UCIMove &uci_move, const chesscore::Move &move) -> bool {
    return move.from == uci_move.from && move.to == uci_move.to &&
           ((move.promoted.has_value() && move.promoted.value().type == uci_move.promotion_piece) || (!move.promoted.has_value() && !uci_move.promotion_piece.has_value()));
}

auto match_move(const UCIMove &move, chesscore::MoveList &moves) -> chesscore::MoveList {
    return moves | std::views::filter([&move](const chesscore::Move &candidate) { return uci_move_matches(move, candidate); }) | std::ranges::to<chesscore::MoveList>();
}

} // namespace chessuci
