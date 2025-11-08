/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_MOVE_H
#define CHESSUCI_MOVE_H

#include <expected>
#include <optional>

#include <chesscore/move.h>
#include <chesscore/position.h>

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

    UCIMove() = default;
    UCIMove(const chesscore::Square &from, const chesscore::Square &to, const std::optional<chesscore::PieceType> &promotion_piece = std::nullopt)
        : from{from}, to{to}, promotion_piece{promotion_piece} {}
    explicit UCIMove(const chesscore::Move &move)
        : from{move.from}, to{move.to}, promotion_piece{move.promoted.has_value() ? std::optional<chesscore::PieceType>{move.promoted.value().type} : std::nullopt} {}

    auto operator==(const UCIMove &rhs) const -> bool { return from == rhs.from && to == rhs.to && promotion_piece == rhs.promotion_piece; }
};

/**
 * \brief Convert an UCIMove to a chesscore::Move.
 *
 * An UCIMove does not contain all the information that a chesscore::Move
 * carries. This function tries to find the chesscore::Move that is described by
 * the UCIMove in the given position. Only legal moves can be converted.
 * \param move The UCIMove to convert.
 * \param position The position to which the move applies.
 * \return The converted move, if it is legal.
 */
auto convert_move(const UCIMove &move, const chesscore::Position &position) -> std::optional<chesscore::Move>;

/**
 * \brief Convert a legal UCIMove to a chesscore::Move.
 *
 * Converts the UCIMove to a chesscore::Move without legality check, which
 * should be faster than using convert_move().
 * This conversion should only be used when the move is known to be legal. Some
 * basic checks are made (e.g., move from an empty square), but pieces could be
 * moved to invalid squares and leave their king in check.
 * \param move The UCIMove to convert.
 * \param position The position to which the move applies.
 * \return The converted move.
 */
auto convert_legal_move(const UCIMove &move, const chesscore::Position &position) -> std::optional<chesscore::Move>;

/**
 * \brief Convert a UCIMove to a string.
 *
 * The string is in long algebraic notation, as used in the UCI protocol.
 * \param move The move to convert.
 * \return The move in long algebraic notation.
 */
auto to_string(const UCIMove &move) -> std::string;

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
auto match_move(const UCIMove &move, const chesscore::MoveList &moves) -> chesscore::MoveList;

} // namespace chessuci

#endif
