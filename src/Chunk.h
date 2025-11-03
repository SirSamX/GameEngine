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

    glm::ivec2 position;
    std::vector<uint8_t> blocks;

    unsigned int VAO = 0, VBO = 0, EBO = 0;
    bool meshDirty = true;
    bool dataReady = false;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Chunk(glm::ivec2 pos);

    uint8_t getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint8_t block);

    void generateMeshData(const World& world);

    void uploadMesh();

    void render();
};
