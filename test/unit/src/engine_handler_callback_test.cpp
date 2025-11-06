/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_handler.h"
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <future>
#include <sstream>

using namespace chessuci;

TEST_CASE("EngineHandler.Callback.No Callbacks", "[engine_handler]") {
    std::stringstream sstr{"quit\n"};
    UCIEngineHandler handler{sstr};

    CHECK_FALSE(handler.is_running());
    handler.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    handler.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_FALSE(handler.is_running());
}

TEST_CASE("EngineHandler.Callback.UCI", "[engine_handler]") {
    std::stringstream sstr{"uci\n"};
    UCIEngineHandler handler{sstr};

    std::promise<void> callback_done;
    auto future = callback_done.get_future();
    handler.on_uci([&callback_done]() -> void { callback_done.set_value(); });

    handler.start();
    REQUIRE(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    // Stop shouldn't be necessary, will be called in handler's desctuctor anyway
}

TEST_CASE("EngineHandler.Callback.Debug", "[engine_handler]") {
    std::stringstream sstr{"debug on\n"};
    UCIEngineHandler handler{sstr};

    std::promise<bool> debugging;
    auto future = debugging.get_future();
    handler.on_debug([&debugging](bool on) -> void { debugging.set_value(on); });

    handler.start();
    CHECK(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(future.get());
    handler.stop();
}

TEST_CASE("EngineHandler.Callback.Multiple", "[engine_handler]") {
    std::stringstream sstr{"debug on\ngo depth 5\n"};
    UCIEngineHandler handler{sstr};

    std::promise<bool> debugging;
    auto debugging_future = debugging.get_future();
    handler.on_debug([&debugging](bool on) -> void { debugging.set_value(on); });
    std::promise<go_command> go;
    auto go_future = go.get_future();
    handler.on_go([&go](const go_command &command) -> void { go.set_value(command); });

    handler.start();
    CHECK(debugging_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(debugging_future.get());
    CHECK(go_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(go_future.get().depth == 5);
    handler.stop();
}

TEST_CASE("EngineHandler.Callback.Custom", "[engine_handler]") {
    std::stringstream sstr{"perft 5\n"};
    UCIEngineHandler handler{sstr};

    std::promise<int> perft;
    auto perft_future = perft.get_future();
    handler.register_command("perft", [&perft](const TokenList &tokens) -> void {
        if (tokens.size() == 2 && tokens[0] == "perft") {
            perft.set_value(std::stoi(tokens[1]));
        }
    });

    handler.start();
    CHECK(perft_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(perft_future.get() == 5);
    handler.stop();
}

TEST_CASE("EngineHandler.Callback.Unknown", "[engine_handler]") {
    std::stringstream sstr{"unknown_command\n"};
    UCIEngineHandler handler{sstr};

    std::promise<std::string> unknown_command;
    auto unknown_command_future = unknown_command.get_future();
    handler.on_unknown_command([&unknown_command](const TokenList &tokens) -> void { unknown_command.set_value(tokens[0]); });

    handler.start();
    CHECK(unknown_command_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(unknown_command_future.get() == "unknown_command");
    handler.stop();
}
