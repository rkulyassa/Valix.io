#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <queue>
#include <App.h>
#include "BinaryReader.hpp"
#include "BinaryWriter.hpp"

constexpr int GAME_TICK_MS = 100;
constexpr int WORLD_GRID_SIZE = 25;
constexpr int SPAWN_AREA_SIZE = 3;

struct PerSocketData {};
typedef uWS::WebSocket<false, true, PerSocketData> WebSocket;

enum class ServerOpcode {
    WELCOME = 0,
    GAMESTATE = 1,
};

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

typedef struct Player {
    uint8_t pid; // this is for the client. server should distinguish/get players by world.players[WebSocket*]
    int r;
    int c;
    Direction direction;
} Player;

uint8_t pidIndex = 1;

enum class TileStatus {
    EMPTY,
    CAPTURED,
    TRAIL,
};


typedef struct Tile {
    Player *owner;
    TileStatus status;
} Tile;

struct World {
    std::unordered_map<WebSocket*, Player*> players;
    Tile grid[WORLD_GRID_SIZE][WORLD_GRID_SIZE];
};

World world{};

uWS::App *globalApp;

int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

/**
 * Launches a multi-source BFS from all unowned grid edges to identify tiles not already captured by the player or part of their trail.
 * All tiles not reachable by the fill are thus within the capture range and are captured by the player.
 * @param player The capturing player
 */
void floodFillBFS(Player *player) {
    bool captureGrid[WORLD_GRID_SIZE][WORLD_GRID_SIZE] = {};

    std::queue<std::pair<int, int>> q;
    
    // initialize queue with all unowned edge tiles
    for (int r = 0; r < WORLD_GRID_SIZE; r++) {
        for (int c = 0; c < WORLD_GRID_SIZE; c++) {
            bool onEdge = r == 0 || r == WORLD_GRID_SIZE-1 || c == 0 || c == WORLD_GRID_SIZE-1;
            if (onEdge && !world.grid[r][c].owner) {
                q.emplace(r, c);
            }
        }
    }

    bool visited[WORLD_GRID_SIZE][WORLD_GRID_SIZE] = {};    

    while (!q.empty()) {
        std::pair<int, int> tile = q.front();
        q.pop();
        
        int r = tile.first;
        int c = tile.second;
        
        if (visited[r][c] || r < 0 || c < 0 || r >= WORLD_GRID_SIZE || c >= WORLD_GRID_SIZE) continue;

        visited[r][c] = true;

        if (world.grid[r][c].owner != player) {
            captureGrid[r][c] = true;
            q.emplace(r+1, c);
            q.emplace(r, c+1);
            q.emplace(r, c-1);
            q.emplace(r-1, c);
        }
    }

    // at this point, captureGrid[r][c] = false means the tile is WITHIN captured territory
    for (int r = 0; r < WORLD_GRID_SIZE; r++) {
        for (int c = 0; c < WORLD_GRID_SIZE; c++) {
            if (!captureGrid[r][c]) {
                world.grid[r][c].owner = player;
                world.grid[r][c].status = TileStatus::CAPTURED;
            }
        }
    }
}

void removePlayerTiles(Player *player) {
    for (int r = 0; r < WORLD_GRID_SIZE; r++) {
        for (int c = 0; c < WORLD_GRID_SIZE; c++) {
            if (world.grid[r][c].owner == player) {
                world.grid[r][c].owner = nullptr;
                world.grid[r][c].status = TileStatus::EMPTY;
            }
        }
    }
}

Player* spawnPlayer() {
    int spawnR = randomInRange(0, WORLD_GRID_SIZE-SPAWN_AREA_SIZE);
    int spawnC = randomInRange(0, WORLD_GRID_SIZE-SPAWN_AREA_SIZE);
    
    Player* player = new Player{
        .pid = pidIndex++,
        .r = spawnR + SPAWN_AREA_SIZE/2,
        .c = spawnC + SPAWN_AREA_SIZE/2,
        .direction = Direction::RIGHT,
    };
    
    for (int r = spawnR; r < spawnR+SPAWN_AREA_SIZE; r++) {
        for (int c = spawnC; c < spawnC+SPAWN_AREA_SIZE; c++) {
            world.grid[r][c].owner = player;
            world.grid[r][c].status = TileStatus::CAPTURED;
        }
    }
    
    std::cout << "Spawning player (pid: " << std::to_string(player->pid) << ") at (" << std::to_string(spawnR) << "," << std::to_string(spawnC) << ")" << std::endl;
    return player;
}

void respawnPlayer(Player *player) {
    removePlayerTiles(player);

    int spawnR = randomInRange(0, WORLD_GRID_SIZE-SPAWN_AREA_SIZE);
    int spawnC = randomInRange(0, WORLD_GRID_SIZE-SPAWN_AREA_SIZE);

    for (int r = spawnR; r < spawnR+SPAWN_AREA_SIZE; r++) {
        for (int c = spawnC; c < spawnC+SPAWN_AREA_SIZE; c++) {
            world.grid[r][c].owner = player;
            world.grid[r][c].status = TileStatus::CAPTURED;
        }
    }

    player->r = spawnR + SPAWN_AREA_SIZE/2;
    player->c = spawnC + SPAWN_AREA_SIZE/2;

    std::cout << "Respawning player (pid: " << std::to_string(player->pid) << ") at (" << std::to_string(spawnR) << "," << std::to_string(spawnC) << ")" << std::endl;
}

void gameTick(us_timer_t *) {
    for (auto &[ws, player] : world.players) {
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
        
        bool outOfBounds = newR < 0 || newC < 0 || newR >= WORLD_GRID_SIZE || newC >= WORLD_GRID_SIZE;
        if (outOfBounds) {
            respawnPlayer(player);
            continue;
        }
        
        Tile &currentTile = world.grid[player->r][player->c];
        Tile &newTile = world.grid[newR][newC];
        
        // check if player is going from captured -> capturing
        if (currentTile.owner == player && currentTile.status == TileStatus::CAPTURED && newTile.owner != player) {
            newTile.status = TileStatus::TRAIL;
        }

        // if player is in capturing state
        if (currentTile.status == TileStatus::TRAIL) {
            if (newTile.status == TileStatus::CAPTURED && newTile.owner == player) {
                floodFillBFS(player);
            } else if (newTile.status == TileStatus::TRAIL && newTile.owner == player) {
                respawnPlayer(player);
                continue;
            } else {
                newTile.status = TileStatus::TRAIL;
            }
        }

        // update player position
        player->r = newR;
        player->c = newC;
        newTile.owner = player;
    }

    // build + broadcast flattened gamestate packet
    BinaryWriter writer;
    writer.writeUint8(static_cast<uint8_t>(ServerOpcode::GAMESTATE));
    for (int r = 0; r < WORLD_GRID_SIZE; r++) {
        for (int c = 0; c < WORLD_GRID_SIZE; c++) {
            Tile *tile = &world.grid[r][c];
            writer.writeUint8(tile->owner ? tile->owner->pid : 0);
            writer.writeUint8(static_cast<uint8_t>(tile->status));
        }
    }
    writer.writeUint8(world.players.size());
    for (const auto &[ws, player] : world.players) {
        writer.writeUint8(player->pid);
        writer.writeUint8(player->r);
        writer.writeUint8(player->c);
    }
    // writer.print();
    globalApp->publish("game", writer.asStringView(), uWS::OpCode::BINARY, false);
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    uWS::App app;

    app.ws<PerSocketData>("/*", {
        .open = [](WebSocket* ws) {
            ws->subscribe("game");

            Player *player = spawnPlayer();
            world.players[ws] = player;

            BinaryWriter writer;
            writer.writeUint8(static_cast<uint8_t>(ServerOpcode::WELCOME));
            writer.writeUint8(player->pid);
            writer.writeUint8(WORLD_GRID_SIZE);
            ws->send(writer.asStringView(), uWS::OpCode::BINARY);
        },
        .message = [](WebSocket* ws, std::string_view rawMessage, uWS::OpCode opCode) {
            std::string message = std::string(rawMessage);

            Player *player = world.players[ws];

            if (message == "ArrowUp" && player->direction != Direction::DOWN) {
                player->direction = Direction::UP;
            } else if (message == "ArrowDown" && player->direction != Direction::UP) {
                player->direction = Direction::DOWN;
            } else if (message == "ArrowLeft" && player->direction != Direction::RIGHT) {
                player->direction = Direction::LEFT;
            } else if (message == "ArrowRight" && player->direction != Direction::LEFT) {
                player->direction = Direction::RIGHT;
            } else {
                return;
            }
        },
        .close = [](WebSocket* ws, int code, std::string_view rawMessage) {
            Player *player = world.players[ws];
            removePlayerTiles(player);
            world.players.erase(ws);
            std::cout << "Disconnecting player (pid: " << std::to_string(player->pid) << ")" << std::endl;
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
    us_timer_set(timer, gameTick, GAME_TICK_MS, GAME_TICK_MS);
    std::cout << "Game loop running at " << 1000/GAME_TICK_MS << "Hz" << std::endl;

    globalApp = &app;

    app.run();
}