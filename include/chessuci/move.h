/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_MOVE_H
#define CHESSUCI_MOVE_H

#include <expected>
#include <optional>

#include <chesscore/move.h>

namespace chessuci {

/**
 * \brief Representation of a move in long algebraic notation.
 *
 * Describes a chess move in long algebraic notation as used in UCI.
 */
struct UCIMove {
    chesscore::Square from;                                            ///< The starting square of the move.
    chesscore::Square to;                                              ///< The target square of the move.
    std::optional<chesscore::PieceType> promotion_piece{std::nullopt}; ///< Type of piece that the moving piece promotes to, if any.

    auto operator==(const UCIMove &rhs) const -> bool { return from == rhs.from && to == rhs.to && promotion_piece == rhs.promotion_piece; }
};

/**
 * \brief Error conditions while parsing an UCI move.
 */
enum class UCIParserErrorType {
    InvalidFile,           ///< Invalid file.
    InvalidRank,           ///< Invalid rank.
    InvalidPromotionPiece, ///< Invalid piece for promotion.
    UnexpectedToken,       ///< Unexpected data.
    MissingData,           ///< The UCI string is too short.
};

/**
 * \brief An error from parsing UCI moves.
 */
struct UCIParserError {
    UCIParserErrorType type; ///< Type of the error.
    std::string uci_str;     ///< The uci move string that could not be parsed.

    auto operator==(const UCIParserError &rhs) const -> bool { return type == rhs.type && uci_str == rhs.uci_str; }
};

/**
 * \brief Parse an UCI move from a string.
 *
 * Extracts the information of a move from a move string in long algebraic notation.
 * \param uci_str The move string.
 * \return The parsed move.
 */
auto parse_uci_move(const std::string &uci_str) -> std::expected<UCIMove, UCIParserError>;

/**
 * \brief Check, if a UCI move matches a move.
 *
 * Checks if the given move can be described by the UCI move.
 * \param uci_move The UCI move.
 * \param move The move.
 * \return If the move can be described by the UCI move.
 */
auto uci_move_matches(const UCIMove &uci_move, const chesscore::Move &move) -> bool;

/**
 * \brief Match a move list against a UCI move.
 *
 * Finds all moves in the move list that can be described by the UCI move.
 * \param move The UCI move.
 * \param moves The move list.
 * \return List of all matching moves.
 */
auto match_move(const UCIMove &move, chesscore::MoveList &moves) -> chesscore::MoveList;

} // namespace chessuci

#endif
