#ifdef __unix__
#include <iostream>
#include <signal.h>
#include <unistd.h>

auto signal_handler(int signal) -> void {
    std::cout << "Received signal " << signal << ", ignoring..." << std::endl;
}

auto main() -> int {
    signal(SIGTERM, signal_handler);

    std::cout << "Zombie process started, ignoring SIGTERM" << std::endl;

    while (true) {
        sleep(1);
    }

    return 0;
}
#else
auto main() -> int {
    return 0;
}
#endif
