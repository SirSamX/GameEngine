#include "World.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

World::World() {
    // For now, let's just load a single chunk at the origin
    loadChunk(0, 0);
}

void World::loadChunk(int x, int z) {
    glm::ivec3 pos(x, 0, z);
    if (chunks.find(pos) == chunks.end()) {
        chunks.emplace(pos, Chunk(pos));
        std::cout << "Loading chunk at: " << x << ", " << z << std::endl;
    }
}

void World::update(const glm::vec3& cameraPos, int distance) {
    // Simple chunk loading logic
    int camChunkX = static_cast<int>(cameraPos.x) / Chunk::WIDTH;
    int camChunkZ = static_cast<int>(cameraPos.z) / Chunk::DEPTH;

    for (int x = camChunkX - distance; x <= camChunkX + distance; ++x) {
        for (int z = camChunkZ - distance; z <= camChunkZ + distance; ++z) {
            loadChunk(x, z);
        }
    }
}

void World::render(Shader& shader) {
    for (auto& pair : chunks) {
        if (pair.second.meshDirty) {
            pair.second.buildMesh(*this);
        }
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pair.first.x * Chunk::WIDTH, 0, pair.first.z * Chunk::DEPTH));
        shader.setMat4("model", model);
        pair.second.render();
    }
}

std::optional<RaycastResult> World::raycast(const glm::vec3& start, const glm::vec3& direction, float maxDist) {
    glm::ivec3 currentBlock(floor(start.x), floor(start.y), floor(start.z));
    glm::vec3 rayStep = glm::normalize(direction);

    for (float dist = 0; dist < maxDist; dist += 0.05f) {
        glm::vec3 pos = start + rayStep * dist;
        glm::ivec3 blockPos(floor(pos.x), floor(pos.y), floor(pos.z));

        if (getBlock(blockPos) != 0) {
            glm::vec3 prevPos = start + rayStep * (dist - 0.05f);
            glm::ivec3 prevBlockPos(floor(prevPos.x), floor(prevPos.y), floor(prevPos.z));
            glm::ivec3 face = blockPos - prevBlockPos;
            return RaycastResult{blockPos, face};
        }
    }
    return std::nullopt;
}

uint8_t World::getBlock(const glm::ivec3& pos) const {
    int chunkX = pos.x >= 0 ? pos.x / Chunk::WIDTH : (pos.x - (Chunk::WIDTH - 1)) / Chunk::WIDTH;
    int chunkZ = pos.z >= 0 ? pos.z / Chunk::DEPTH : (pos.z - (Chunk::DEPTH - 1)) / Chunk::DEPTH;
    auto it = chunks.find(glm::ivec3(chunkX, 0, chunkZ));
    if (it != chunks.end()) {
        int localX = pos.x - chunkX * Chunk::WIDTH;
        int localZ = pos.z - chunkZ * Chunk::DEPTH;
        return it->second.getBlock(localX, pos.y, localZ);
    }
    return 0;
}

void World::setBlock(const glm::ivec3& pos, uint8_t block) {
    int chunkX = pos.x >= 0 ? pos.x / Chunk::WIDTH : (pos.x - (Chunk::WIDTH - 1)) / Chunk::WIDTH;
    int chunkZ = pos.z >= 0 ? pos.z / Chunk::DEPTH : (pos.z - (Chunk::DEPTH - 1)) / Chunk::DEPTH;
    auto it = chunks.find(glm::ivec3(chunkX, 0, chunkZ));
    if (it != chunks.end()) {
        int localX = pos.x - chunkX * Chunk::WIDTH;
        int localZ = pos.z - chunkZ * Chunk::DEPTH;
        it->second.setBlock(localX, pos.y, localZ, block);
    }
}
