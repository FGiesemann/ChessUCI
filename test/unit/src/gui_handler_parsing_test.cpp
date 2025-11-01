/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/gui_handler.h"
#include "chessuci/uci_handler.h"
#include <catch2/catch_test_macros.hpp>

using namespace chessuci;

namespace {

auto parse_bestmove(const std::string &bestmove_str) -> bestmove_info {
    const auto tokens = UCIHandler::tokenize(bestmove_str);
    return UCIGuiHandler::parse_bestmove_command(tokens);
}

auto parse_info(const std::string &info_str) -> search_info {
    const auto tokens = UCIHandler::tokenize(info_str);
    return UCIGuiHandler::parse_info_command(tokens);
}

auto parse_option(const std::string &option_str) -> Option {
    const auto tokens = UCIHandler::tokenize(option_str);
    return UCIGuiHandler::parse_option_command(tokens);
}

} // namespace

TEST_CASE("GuiHandler.Parser.Bestmove", "[gui_handler]") {
    const auto info1 = parse_bestmove("bestmove e2e4");
    CHECK(to_string(info1.bestmove) == "e2e4");

    const auto info2 = parse_bestmove("bestmove e2e4 ponder d2d3");
    CHECK(to_string(info2.bestmove) == "e2e4");
    REQUIRE(info2.pondermove.has_value());
    CHECK(to_string(info2.pondermove.value()) == "d2d3");
}

TEST_CASE("GuiHandler.Parser.Info", "[gui_handler]") {
    const auto info1 = parse_info("info depth 20 seldepth 25 currmovenumber 15 nps 3456789 pv a2a3 b7b5 a3a4");
    CHECK(info1.depth == 20);
    CHECK(info1.seldepth == 25);
    CHECK(info1.currmovenumber == 15);
    CHECK(info1.nps == 3456789);
    REQUIRE(info1.pv.size() == 3);
    CHECK(to_string(info1.pv[0]) == "a2a3");
    CHECK(to_string(info1.pv[1]) == "b7b5");
    CHECK(to_string(info1.pv[2]) == "a3a4");

    const auto info2 = parse_info("info score cp -500 lowerbound hashfull 80 multipv 1");
    REQUIRE(info2.score.has_value());
    CHECK(info2.score->cp == -500);
    CHECK(info2.score->lowerbound);
    CHECK_FALSE(info2.score->upperbound);
    CHECK(info2.hashfull == 80);
    CHECK(info2.multipv == 1);

    const auto info3 = parse_info("info string Opening  analysis is\tcomplete.");
    CHECK(info3.string == "Opening analysis is complete.");

    const auto info4 = parse_info("info score mate 3 currline 0 a2a3 b7b5 a3a4 nps 456456 multipv 1");
    REQUIRE(info4.score.has_value());
    CHECK(info4.score->mate == 3);
    REQUIRE(info4.currline.has_value());
    CHECK(info4.currline->cpunr == 0);
    REQUIRE(info4.currline->line.size() == 3);
    CHECK(to_string(info4.currline->line[0]) == "a2a3");
    CHECK(to_string(info4.currline->line[1]) == "b7b5");
    CHECK(to_string(info4.currline->line[2]) == "a3a4");
    CHECK(info4.nps == 456456);
    CHECK(info4.multipv == 1);
}

TEST_CASE("GuiHandler.Parser.Option", "[gui_handler]") {
    const auto option1 = parse_option("option name UCI_EngineAbout type string default Shredder by Stefan Meyer-Kahlen, see www.shredderchess.com");
    CHECK(option1.name == "UCI_EngineAbout");
    CHECK(option1.type == Option::Type::String);
    CHECK(option1.default_value == "Shredder by Stefan Meyer-Kahlen, see www.shredderchess.com");

    const auto option2 = parse_option("option name Style type combo default Normal var Solid var Normal var Risky");
    CHECK(option2.name == "Style");
    CHECK(option2.type == Option::Type::Combo);
    REQUIRE(option2.combo_values.size() == 3);
    CHECK(option2.combo_values[0] == "Solid");
    CHECK(option2.combo_values[1] == "Normal");
    CHECK(option2.combo_values[2] == "Risky");
    CHECK(option2.default_value == "Normal");

    const auto option3 = parse_option("option name Max Depth type spin default 20 min 1 max 100");
    CHECK(option3.name == "Max Depth");
    CHECK(option3.type == Option::Type::Spin);
    CHECK(option3.default_value == "20");
    CHECK(option3.min == 1);
    CHECK(option3.max == 100);

    const auto option4 = parse_option("option name cheat type check default false");
    CHECK(option4.name == "cheat");
    CHECK(option4.type == Option::Type::Check);
    CHECK(option4.default_value == "false");

    const auto option5 = parse_option("option name run type button");
    CHECK(option5.name == "run");
    CHECK(option5.type == Option::Type::Button);
}
