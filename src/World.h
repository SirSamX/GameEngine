#pragma once
#include "Chunk.h"
#include "Shader.h"
#include <unordered_map>
#include <glm/vec3.hpp>
#include <optional>

// Simple hash function for glm::ivec3
namespace std {
    template <>
    struct hash<glm::ivec3> {
        std::size_t operator()(const glm::ivec3& v) const {
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
        }
    };
}

struct RaycastResult {
    glm::ivec3 blockPos;
    glm::ivec3 face;
};

class World {
public:
    World();
    void render(Shader& shader);
    void update(const glm::vec3& cameraPos, int distance);

    std::optional<RaycastResult> raycast(const glm::vec3& start, const glm::vec3& direction, float maxDist);
    uint8_t getBlock(const glm::ivec3& pos) const;
    void setBlock(const glm::ivec3& pos, uint8_t block);

private:
    std::unordered_map<glm::ivec3, Chunk> chunks;
    void loadChunk(int x, int z);
};
