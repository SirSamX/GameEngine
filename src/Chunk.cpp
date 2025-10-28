#include "Chunk.h"
#include "Noise.h"
#include <GL/glew.h>
#include "World.h"

Chunk::Chunk(glm::ivec3 pos) : position(pos), blocks(WIDTH * HEIGHT * DEPTH, 0), meshDirty(true) {
    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float height = Noise::generate(position.x * WIDTH + x, position.z * DEPTH + z) * HEIGHT;
            for (int y = 0; y < height; ++y) {
                blocks[x + z * WIDTH + y * WIDTH * DEPTH] = 1; // 1 for solid block
            }
        }
    }
}

uint8_t Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || z < 0 || z >= DEPTH) {
        return 0; // Air block
    }
    return blocks[x + z * WIDTH + y * WIDTH * DEPTH];
}

void Chunk::setBlock(int x, int y, int z, uint8_t block) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || z < 0 || z >= DEPTH) {
        return;
    }
    blocks[x + z * WIDTH + y * WIDTH * DEPTH] = block;
    meshDirty = true;
}

void Chunk::buildMesh(const World& world) {
    vertices.clear();
    indices.clear();
    unsigned int indexOffset = 0;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int z = 0; z < DEPTH; ++z) {
            for (int x = 0; x < WIDTH; ++x) {
                if (getBlock(x, y, z) == 0) continue;

                // Check for exposed faces
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x, y + 1, position.z * DEPTH + z)) == 0) { // Top
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x - 0.5f, y + 0.5f, z - 0.5f, 0, 1, x + 0.5f, y + 0.5f, z - 0.5f, 1, 1, x + 0.5f, y + 0.5f, z + 0.5f, 1, 0, x - 0.5f, y + 0.5f, z + 0.5f, 0, 0});
                    indexOffset += 4;
                }
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x, y - 1, position.z * DEPTH + z)) == 0) { // Bottom
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x - 0.5f, y - 0.5f, z - 0.5f, 0, 0, x + 0.5f, y - 0.5f, z - 0.5f, 1, 0, x + 0.5f, y - 0.5f, z + 0.5f, 1, 1, x - 0.5f, y - 0.5f, z + 0.5f, 0, 1});
                    indexOffset += 4;
                }
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x + 1, y, position.z * DEPTH + z)) == 0) { // Right
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x + 0.5f, y - 0.5f, z - 0.5f, 0, 0, x + 0.5f, y + 0.5f, z - 0.5f, 1, 0, x + 0.5f, y + 0.5f, z + 0.5f, 1, 1, x + 0.5f, y - 0.5f, z + 0.5f, 0, 1});
                    indexOffset += 4;
                }
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x - 1, y, position.z * DEPTH + z)) == 0) { // Left
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x - 0.5f, y - 0.5f, z - 0.5f, 0, 0, x - 0.5f, y + 0.5f, z - 0.5f, 1, 0, x - 0.5f, y + 0.5f, z + 0.5f, 1, 1, x - 0.5f, y - 0.5f, z + 0.5f, 0, 1});
                    indexOffset += 4;
                }
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x, y, position.z * DEPTH + z + 1)) == 0) { // Front
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x - 0.5f, y - 0.5f, z + 0.5f, 0, 0, x + 0.5f, y - 0.5f, z + 0.5f, 1, 0, x + 0.5f, y + 0.5f, z + 0.5f, 1, 1, x - 0.5f, y + 0.5f, z + 0.5f, 0, 1});
                    indexOffset += 4;
                }
                if (world.getBlock(glm::ivec3(position.x * WIDTH + x, y, position.z * DEPTH + z - 1)) == 0) { // Back
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {x - 0.5f, y - 0.5f, z - 0.5f, 0, 0, x + 0.5f, y - 0.5f, z - 0.5f, 1, 0, x + 0.5f, y + 0.5f, z - 0.5f, 1, 1, x - 0.5f, y + 0.5f, z - 0.5f, 0, 1});
                    indexOffset += 4;
                }
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    meshDirty = false;
}

void Chunk::render() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}