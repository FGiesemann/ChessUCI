#include <chrono>
#include <iostream>
#include <thread>

auto main(int argc, char *argv[]) -> int {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
