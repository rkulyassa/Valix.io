#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

typedef struct Player {
    float x;
    float y;
} Player;

typedef struct GameState {
    std::vector<Player> players;
} GameState;

int main() {
    GameState s;
    int tickCount = 0;

    constexpr double fixed_dt = 1.0 / 60.0;
    double accumulator = 0.0;
    auto last = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - last;
        last = now;

        accumulator += elapsed.count();

        while (accumulator >= fixed_dt) {
            tickCount++;
            if (tickCount % 60 == 0) {
                std::cout << "tick " << tickCount << std::endl;
            }
            accumulator -= fixed_dt;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}