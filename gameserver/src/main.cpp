#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <optional>
#include <App.h>

typedef struct PlayerData {
    float x;
    float y;
} Player;

constexpr int GAME_TICK_PERIOD = 50; // tick period in ms


// typedef struct GameState {
//     std::vector<Player> players;
//     int gridWidth;
//     int gridHeight;
//     int count;
// } GameState;

// GameState s{
//     .players = {},
//     .gridWidth = 20,
//     .gridHeight = 15,
//     .count = 0
// };
int tickCount = 0;

enum class ClientOpcode {
    JOIN,
    LEAVE
};

typedef uWS::WebSocket<false, true, PlayerData> WebSocket;

// typedef struct InputEvent {
//     WebSocket* client;
//     ClientOpcode opcode;
//     std::optional<std::string> msg;
// } InputEvent;
uWS::App *globalApp;

void gameTick(us_timer_t*) {
    globalApp->publish("game", "tick " + std::to_string(tickCount++), uWS::OpCode::TEXT, false);
}


int main() {

    // constexpr double fixed_dt = 1.0 / 60.0;
    // double accumulator = 0.0;
    // auto last = std::chrono::steady_clock::now();

    // while (true) {
    //     auto now = std::chrono::steady_clock::now();
    //     std::chrono::duration<double> elapsed = now - last;
    //     last = now;

    //     accumulator += elapsed.count();

    //     while (accumulator >= fixed_dt) {
    //         tickCount++;
    //         if (tickCount % 60 == 0) {
    //             std::cout << "tick " << tickCount << std::endl;
    //         }
    //         accumulator -= fixed_dt;
    //     }

    //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // }

    uWS::App app;

    app.ws<PlayerData>("/*", {
        .open = [](WebSocket* ws) {
            // std::cout << "Client connected" << std::endl;
            // ws->send("Hello", uWS::OpCode::TEXT);
            ws->subscribe("game");
        },
        .message = [](WebSocket* ws, std::string_view rawMessage, uWS::OpCode opCode) {
            float dx;
            float dy;
            sscanf(rawMessage.data(), "%f %f", &dx, &dy);
            // std::cout << "Received message: " << std::string(raw_message) << std::endl;
            // std::cout << "Received message: " << raw_message.data() << std::endl;
            // std::cout << dx << dy << std::endl;
            // ws->send(raw_message, uWS::OpCode::TEXT);
        }
    });

    app.listen(9001, [](auto *listenSocket) {
        if (listenSocket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        } else {
            std::cout << "Failed to load certs or to bind to port" << std::endl;
        }
    });

    // run game loop
    us_loop_t *loop = (us_loop_t *)uWS::Loop::get();
    us_timer_t *timer = us_create_timer(loop, 0, 0);
    us_timer_set(timer, gameTick, GAME_TICK_PERIOD, GAME_TICK_PERIOD);
    std::cout << "Game loop running at " << 1000/GAME_TICK_PERIOD << "Hz" << std::endl;

    globalApp = &app;

    app.run();
}