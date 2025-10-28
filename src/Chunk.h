#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <cstdint>

class World;

class Chunk {
public:
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 128;
    static constexpr int DEPTH = 16;

    glm::ivec3 position;
    std::vector<uint8_t> blocks;
    
    unsigned int VAO, VBO, EBO;
    bool meshDirty = true;

    Chunk(glm::ivec3 pos);

    uint8_t getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint8_t block);

    void buildMesh(const World& world);
    void render();

private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};
