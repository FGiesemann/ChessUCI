#include <chrono>
#include <iostream>
#include <thread>

auto main(int argc, char *argv[]) -> int {
    int delay_ms = 2000;
    if (argc > 1) {
        delay_ms = std::atoi(argv[1]);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    std::cout << "Ready" << std::endl;

    return 0;
}
