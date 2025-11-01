/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_handler.h"
#include "chessuci/uci_handler.h"
#include <catch2/catch_test_macros.hpp>
#include <chesscore/fen.h>

using namespace chessuci;

namespace {

auto parse_options(const std::string &option_str) -> setoption_command {
    const auto tokens = UCIHandler::tokenize(option_str);
    return UCIEngineHandler::parse_set_option_command(tokens);
}

auto parse_position(const std::string &position_str) -> position_command {
    const auto tokens = UCIHandler::tokenize(position_str);
    return UCIEngineHandler::parse_position_command(tokens);
}

auto parse_go(const std::string &go_str) -> go_command {
    const auto tokens = UCIHandler::tokenize(go_str);
    return UCIEngineHandler::parse_go_command(tokens);
}

} // namespace

TEST_CASE("EngineHandler.Parser.Debug", "[engine_handler]") {
    auto tokens = UCIHandler::tokenize("debug on");
    CHECK(UCIEngineHandler::parse_debug_command(tokens));

    tokens[1] = "off";
    CHECK_FALSE(UCIEngineHandler::parse_debug_command(tokens));
}

TEST_CASE("EngineHandler.Parser.Setoption", "[engine_handler]") {
    const auto command1 = parse_options("setoption name Selectivity value 3");
    CHECK(command1.name == "Selectivity");
    CHECK(command1.value == "3");

    const auto command2 = parse_options("setoption name Clear Hash");
    CHECK(command2.name == "Clear Hash");
    CHECK_FALSE(command2.value.has_value());

    const auto command3 = parse_options("setoption name Clear Hash value on");
    CHECK(command3.name == "Clear Hash");
    CHECK(command3.value == "on");

    const auto command4 = parse_options(R"(setoption name NalimovPath value c:\chess\tb\4;c:\chess\tb\5)");
    CHECK(command4.name == "NalimovPath");
    CHECK(command4.value == R"(c:\chess\tb\4;c:\chess\tb\5)");
}

TEST_CASE("EngineHandler.Parser.Position", "[engine_handler]") {
    const auto command1 = parse_position("position startpos");
    CHECK(command1.fen == "startpos");
    CHECK(command1.moves.empty());

    const auto command2 = parse_position("position startpos moves e2e4 e7e5");
    CHECK(command2.fen == "startpos");
    REQUIRE(command2.moves.size() == 2);
    CHECK(to_string(command2.moves[0]) == "e2e4");
    CHECK(to_string(command2.moves[1]) == "e7e5");

    const auto command3 = parse_position("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5");
    CHECK(command3.fen == chesscore::starting_position_fen);
    REQUIRE(command3.moves.size() == 2);
    CHECK(to_string(command3.moves[0]) == "e2e4");
    CHECK(to_string(command3.moves[1]) == "e7e5");
}

TEST_CASE("EngineHandler.Parser.Go", "[engine_handler]") {
    const auto command1 = parse_go("go infinite");
    CHECK(command1.infinite);

    const auto command2 = parse_go("go depth 5");
    CHECK(command2.depth == 5);

    const auto command3 = parse_go("go depth 5 movetime 250");
    CHECK(command3.depth == 5);
    CHECK(command3.movetime == 250);
}
