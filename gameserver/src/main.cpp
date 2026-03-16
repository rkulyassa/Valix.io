#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <App.h>

constexpr int GAME_TICK_PERIOD = 50; // ms
constexpr int WORLD_GRID_WIDTH = 10;
constexpr int WORLD_GRID_HEIGHT = 10;

struct PerSocketData {};
typedef uWS::WebSocket<false, true, PerSocketData> WebSocket;

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

typedef struct Player {
    uint8_t pid;
    int r;
    int c;
    Direction direction;
} Player;

uint8_t pidIndex = 0;

typedef struct Tile {
    Player *owner;
} Tile;

typedef struct World {
    std::unordered_map<WebSocket*, Player> players;
    Tile grid[WORLD_GRID_HEIGHT][WORLD_GRID_WIDTH];
} World;

World world{};

uWS::App *globalApp;

void gameTick(us_timer_t *) {
    // step each player based on their current direction
    for (auto& [ws, player] : world.players) {
        Tile *currentTile = &world.grid[player.r][player.c];
        currentTile->owner = nullptr;

        int newR = player.r;
        int newC = player.c;

        switch (player.direction) {
            case Direction::UP:
                newR--;
                break;
            case Direction::DOWN:
                newR++;
                break;
            case Direction::LEFT:
                newC--;
                break;
            case Direction::RIGHT:
                newC++;
                break;
        }

        
        newR = std::max(0, std::min(newR, WORLD_GRID_HEIGHT - 1));
        newC = std::max(0, std::min(newC, WORLD_GRID_WIDTH - 1));

        Tile *newTile = &world.grid[newR][newC];
        newTile->owner = &player;
    }

    // Create a flattened state packet of the grid: each cell encodes the player id or empty
    std::string statePacket;
    for (int r = 0; r < WORLD_GRID_HEIGHT; r++) {
        for (int c = 0; c < WORLD_GRID_WIDTH; c++) {
            Tile *tile = &world.grid[r][c];
            Player *owner = tile ? tile->owner : nullptr;
            statePacket += owner ? std::to_string(owner->pid) + "," : "0,";
        }
    }
    // std::cout << statePacket << std::endl;
    globalApp->publish("game", statePacket, uWS::OpCode::TEXT, false);
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

    app.ws<PerSocketData>("/*", {
        .open = [](WebSocket* ws) {
            std::cout << "Client connected" << std::endl;
            // ws->send("Hello", uWS::OpCode::TEXT);
            ws->subscribe("game");

            Player player = Player{
                .pid = pidIndex++,
                .r = 0,
                .c = 0,
                .direction = Direction::RIGHT
            };

            world.players[ws] = player;
        },
        .message = [](WebSocket* ws, std::string_view rawMessage, uWS::OpCode opCode) {
            std::string message = std::string(rawMessage);

            Player &player = world.players[ws];

            if (message == "ArrowUp") {
                player.direction = Direction::UP;
            } else if (message == "ArrowDown") {
                player.direction = Direction::DOWN;
            } else if (message == "ArrowLeft") {
                player.direction = Direction::LEFT;
            } else if (message == "ArrowRight") {
                player.direction = Direction::RIGHT;
            } else {
                return;
            }
            // float dx;
            // float dy;
            // sscanf(rawMessage.data(), "%f %f", &dx, &dy);
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