#include <chrono>
#include <thread>

int main() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
    return 0;
}
