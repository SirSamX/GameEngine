#include "World.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cmath>

World::World() {
    worker = std::thread(&World::worldGenThread, this);
}

World::~World() {
    running = false;
    cv.notify_all();
    if (worker.joinable()) worker.join();
}

void World::update(const glm::vec3& cameraPos, int distance) {
    int camChunkX = static_cast<int>(cameraPos.x) / Chunk::WIDTH;
    int camChunkZ = static_cast<int>(cameraPos.z) / Chunk::DEPTH;

    for (int x = camChunkX - distance; x <= camChunkX + distance; ++x) {
        for (int z = camChunkZ - distance; z <= camChunkZ + distance; ++z) {
            loadChunkAsync(x, z);
        }
    }
}

void World::render(Shader& shader) {
    std::lock_guard<std::recursive_mutex> lock(chunksMutex);
    for (auto& pair : chunks) {
        if (pair.second.meshDirty && pair.second.dataReady) {
            pair.second.generateMeshData(*this);
            pair.second.uploadMesh();
        }
        shader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(pair.first.x * Chunk::WIDTH, 0, pair.first.z * Chunk::DEPTH)));
        pair.second.render();
    }
}



uint8_t World::getBlock(const glm::ivec3& pos) const {
    int chunkX = static_cast<int>(floor(static_cast<float>(pos.x) / Chunk::WIDTH));
    int chunkZ = static_cast<int>(floor(static_cast<float>(pos.z) / Chunk::DEPTH));
    std::lock_guard<std::recursive_mutex> lock(chunksMutex);
    auto it = chunks.find(glm::ivec3(chunkX, 0, chunkZ));
    if (it != chunks.end()) {
        int localX = pos.x - chunkX * Chunk::WIDTH;
        int localZ = pos.z - chunkZ * Chunk::DEPTH;
        return it->second.getBlock(localX, pos.y, localZ);
    }
    return 0;
}

void World::setBlock(const glm::ivec3& pos, uint8_t block) {
    int chunkX = static_cast<int>(floor(static_cast<float>(pos.x) / Chunk::WIDTH));
    int chunkZ = static_cast<int>(floor(static_cast<float>(pos.z) / Chunk::DEPTH));
    std::lock_guard<std::recursive_mutex> lock(chunksMutex);
    auto it = chunks.find(glm::ivec3(chunkX, 0, chunkZ));
    if (it != chunks.end()) {
        int localX = pos.x - chunkX * Chunk::WIDTH;
        int localZ = pos.z - chunkZ * Chunk::DEPTH;
        it->second.setBlock(localX, pos.y, localZ, block);
        it->second.meshDirty = true;

        if (localX == 0) markChunkDirty(chunkX - 1, chunkZ);
        if (localX == Chunk::WIDTH - 1) markChunkDirty(chunkX + 1, chunkZ);
        if (localZ == 0) markChunkDirty(chunkX, chunkZ - 1);
        if (localZ == Chunk::DEPTH - 1) markChunkDirty(chunkX, chunkZ + 1);
    }
}

void World::loadChunkAsync(int x, int z) {
    std::lock_guard<std::mutex> lock(queueMutex);
    loadQueue.emplace(x, z);
    cv.notify_one();
}

void World::worldGenThread() {
    while (running) {
        std::pair<int,int> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&]{ return !loadQueue.empty() || !running; });
            if (!running) break;
            task = loadQueue.front();
            loadQueue.pop();
        }

        glm::ivec2 chunkPos(task.first, task.second);
        {
            std::lock_guard<std::recursive_mutex> lock(chunksMutex);
            if (chunks.count(glm::ivec3(chunkPos.x, 0, chunkPos.y))) continue;
        }

        auto start = std::chrono::high_resolution_clock::now();
        Chunk c(chunkPos);
        c.generateMeshData(*this);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> ms = end - start;
        //std::cout << "Chunk (" << task.first << ", " << task.second << ") generated in " << ms.count() << " ms\n";

        {
            std::lock_guard<std::recursive_mutex> lock(chunksMutex);
            chunks.emplace(glm::ivec3(chunkPos.x, 0, chunkPos.y), std::move(c));

            auto markDirty = [&](int x, int z) {
                auto it = chunks.find(glm::ivec3(x, 0, z));
                if (it != chunks.end()) {
                    it->second.meshDirty = true;
                }
            };

            markDirty(chunkPos.x + 1, chunkPos.y);
            markDirty(chunkPos.x - 1, chunkPos.y);
            markDirty(chunkPos.x, chunkPos.y + 1);
            markDirty(chunkPos.x, chunkPos.y - 1);
        }
    }
}
            
void World::markChunkDirty(int x, int z) {
    auto it = chunks.find(glm::ivec3(x, 0, z));
    if (it != chunks.end()) {
        it->second.meshDirty = true;
    }
}