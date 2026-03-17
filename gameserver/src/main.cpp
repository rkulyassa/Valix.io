#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <App.h>

constexpr int GAME_TICK_PERIOD = 100; // ms
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
    std::unordered_map<WebSocket*, Player*> players;
    Tile grid[WORLD_GRID_HEIGHT][WORLD_GRID_WIDTH];
} World;

World world{};

uWS::App *globalApp;

void gameTick(us_timer_t *) {
    // step each player based on their current direction
    for (auto &[ws, player] : world.players) {
        world.grid[player->r][player->c].owner = nullptr;

        int newR = player->r;
        int newC = player->c;

        switch (player->direction) {
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

        world.grid[newR][newC].owner = player;
        player->r = newR;
        player->c = newC;
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
    if (!statePacket.empty()) {
        statePacket.pop_back(); // remove the last comma
    }
    globalApp->publish("game", statePacket, uWS::OpCode::TEXT, false);
}

int main() {
    uWS::App app;

    app.ws<PerSocketData>("/*", {
        .open = [](WebSocket* ws) {
            // std::cout << "Client connected" << std::endl;
            ws->subscribe("game");

            Player player = Player{
                .pid = pidIndex++,
                .r = 0,
                .c = 0,
                .direction = Direction::RIGHT
            };

            world.players[ws] = &player;
            world.grid[0][0].owner = &player;

            std::cout << "Player joined (pid " << std::to_string(player.pid) << ")" << std::endl;
        },
        .message = [](WebSocket* ws, std::string_view rawMessage, uWS::OpCode opCode) {
            std::string message = std::string(rawMessage);

            Player *player = world.players[ws];

            if (message == "ArrowUp") {
                player->direction = Direction::UP;
            } else if (message == "ArrowDown") {
                player->direction = Direction::DOWN;
            } else if (message == "ArrowLeft") {
                player->direction = Direction::LEFT;
            } else if (message == "ArrowRight") {
                player->direction = Direction::RIGHT;
            } else {
                return;
            }
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