#include <chrono>
#include <cstdlib>
#include <thread>

int main(int argc, char *argv[]) {
    int delay_ms = 100;
    if (argc > 1) {
        delay_ms = std::atoi(argv[1]);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

    std::abort();

    return 1;
}
