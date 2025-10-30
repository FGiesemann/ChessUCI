#ifdef __unix__
#include <iostream>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signal) {
    std::cout << "Received signal " << signal << ", ignoring..." << std::endl;
}

int main() {
    signal(SIGTERM, signal_handler);

    std::cout << "Zombie process started, ignoring SIGTERM" << std::endl;

    while (true) {
        sleep(1);
    }

    return 0;
}
#else
int main() {
    return 0;
}
#endif
