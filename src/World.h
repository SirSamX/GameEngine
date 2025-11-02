#pragma once
#include <unordered_map>
#include <glm/vec3.hpp>
#include <optional>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "Chunk.h"
#include "Shader.h"
#include "RaycastResult.h"

namespace std {
    template <>
    struct hash<glm::ivec3> {
        std::size_t operator()(const glm::ivec3& v) const {
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
        }
    };
}

class World {
    mutable std::recursive_mutex chunksMutex;
    std::thread worker;
    std::queue<std::pair<int,int>> loadQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
public:
    std::unordered_map<glm::ivec3, Chunk> chunks;
    World();
    ~World();
    void render(Shader& shader);
    void update(const glm::vec3& cameraPos, int distance);

    uint8_t getBlock(const glm::ivec3& pos) const;
    void setBlock(const glm::ivec3& pos, uint8_t block);
    void markChunkDirty(int x, int z);
    void loadChunk(int x, int z);
    void loadChunkAsync(int x, int z);

private:
    void worldGenThread();
};
