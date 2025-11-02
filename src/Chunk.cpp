#include <GL/glew.h>
#include "Chunk.h"
#include "Noise.h"
#include "World.h"
#include <vector>
#include <map>
#include "Block.h"

std::map<Block, std::vector<glm::vec2>> block_textures = {
    {Block::Grass, {glm::vec2(0.0f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)}},
    {Block::Stone, {glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.5f), glm::vec2(0.0f, 0.5f)}}
};

Chunk::Chunk(glm::ivec3 pos) : position(pos), blocks(WIDTH * HEIGHT * DEPTH, 0), meshDirty(true) {
    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float height = Noise::generate(position.x * WIDTH + x, position.z * DEPTH + z) * HEIGHT;
            for (int y = 0; y < height; ++y) {
                blocks[x + z * WIDTH + y * WIDTH * DEPTH] = (y < height - 5) ? (uint8_t)Block::Stone : (uint8_t)Block::Grass;
            }
        }
    }
}

uint8_t Chunk::getBlock(int x, int y, int z) const {
    if (x<0||x>=WIDTH||y<0||y>=HEIGHT||z<0||z>=DEPTH) return 0;
    return blocks[x + z*WIDTH + y*WIDTH*DEPTH];
}

void Chunk::setBlock(int x, int y, int z, uint8_t block) {
    if (x<0||x>=WIDTH||y<0||y>=HEIGHT||z<0||z>=DEPTH) return;
    blocks[x + z*WIDTH + y*WIDTH*DEPTH] = block;
    meshDirty = true;
}

uint8_t getBlock_internal(int x, int y, int z, const Chunk& chunk, const World& world) {
    if (x >= 0 && x < Chunk::WIDTH &&
        y >= 0 && y < Chunk::HEIGHT &&
        z >= 0 && z < Chunk::DEPTH) {
        return chunk.getBlock(x, y, z);
    }

    glm::ivec3 worldPos = {
        chunk.position.x * Chunk::WIDTH + x,
        y,
        chunk.position.z * Chunk::DEPTH + z
    };
    return world.getBlock(worldPos);
}

void Chunk::generateMeshData(const World& world) {
    vertices.clear();
    indices.clear();
    unsigned int indexOffset = 0;

    for (int y=0; y<HEIGHT; ++y) {
        for (int z=0; z<DEPTH; ++z) {
            for (int x=0; x<WIDTH; ++x) {
                uint8_t block_id = getBlock(x, y, z);
                if (block_id==0) continue;
                Block block_type = (Block)block_id;
                auto tex_coords = block_textures[block_type];

                if (getBlock_internal(x, y + 1, z, *this, world) == 0) { // Top
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x,     (float)y + 1, (float)z,     tex_coords[0].x, tex_coords[0].y, 0.0f, 1.0f, 0.0f,
                        (float)x + 1, (float)y + 1, (float)z,     tex_coords[1].x, tex_coords[1].y, 0.0f, 1.0f, 0.0f,
                        (float)x + 1, (float)y + 1, (float)z + 1, tex_coords[2].x, tex_coords[2].y, 0.0f, 1.0f, 0.0f,
                        (float)x,     (float)y + 1, (float)z + 1, tex_coords[3].x, tex_coords[3].y, 0.0f, 1.0f, 0.0f
                    });
                    indexOffset += 4;
                }
                if (getBlock_internal(x, y - 1, z, *this, world) == 0) { // Bottom
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x,     (float)y, (float)z,     tex_coords[0].x, tex_coords[0].y, 0.0f, -1.0f, 0.0f,
                        (float)x + 1, (float)y, (float)z,     tex_coords[1].x, tex_coords[1].y, 0.0f, -1.0f, 0.0f,
                        (float)x + 1, (float)y, (float)z + 1, tex_coords[2].x, tex_coords[2].y, 0.0f, -1.0f, 0.0f,
                        (float)x,     (float)y, (float)z + 1, tex_coords[3].x, tex_coords[3].y, 0.0f, -1.0f, 0.0f
                    });
                    indexOffset += 4;
                }
                if (getBlock_internal(x + 1, y, z, *this, world) == 0) { // Right
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x + 1, (float)y,     (float)z,     tex_coords[0].x, tex_coords[0].y, 1.0f, 0.0f, 0.0f,
                        (float)x + 1, (float)y + 1, (float)z,     tex_coords[1].x, tex_coords[1].y, 1.0f, 0.0f, 0.0f,
                        (float)x + 1, (float)y + 1, (float)z + 1, tex_coords[2].x, tex_coords[2].y, 1.0f, 0.0f, 0.0f,
                        (float)x + 1, (float)y,     (float)z + 1, tex_coords[3].x, tex_coords[3].y, 1.0f, 0.0f, 0.0f
                    });
                    indexOffset += 4;
                }
                if (getBlock_internal(x - 1, y, z, *this, world) == 0) { // Left
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x, (float)y,     (float)z,     tex_coords[0].x, tex_coords[0].y, -1.0f, 0.0f, 0.0f,
                        (float)x, (float)y + 1, (float)z,     tex_coords[1].x, tex_coords[1].y, -1.0f, 0.0f, 0.0f,
                        (float)x, (float)y + 1, (float)z + 1, tex_coords[2].x, tex_coords[2].y, -1.0f, 0.0f, 0.0f,
                        (float)x, (float)y,     (float)z + 1, tex_coords[3].x, tex_coords[3].y, -1.0f, 0.0f, 0.0f
                    });
                    indexOffset += 4;
                }
                if (getBlock_internal(x, y, z + 1, *this, world) == 0) { // Front
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x,     (float)y,     (float)z + 1, tex_coords[0].x, tex_coords[0].y, 0.0f, 0.0f, 1.0f,
                        (float)x + 1, (float)y,     (float)z + 1, tex_coords[1].x, tex_coords[1].y, 0.0f, 0.0f, 1.0f,
                        (float)x + 1, (float)y + 1, (float)z + 1, tex_coords[2].x, tex_coords[2].y, 0.0f, 0.0f, 1.0f,
                        (float)x,     (float)y + 1, (float)z + 1, tex_coords[3].x, tex_coords[3].y, 0.0f, 0.0f, 1.0f
                    });
                    indexOffset += 4;
                }
                if (getBlock_internal(x, y, z - 1, *this, world) == 0) { // Back
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        (float)x,     (float)y,     (float)z, tex_coords[0].x, tex_coords[0].y, 0.0f, 0.0f, -1.0f,
                        (float)x + 1, (float)y,     (float)z, tex_coords[1].x, tex_coords[1].y, 0.0f, 0.0f, -1.0f,
                        (float)x + 1, (float)y + 1, (float)z, tex_coords[2].x, tex_coords[2].y, 0.0f, 0.0f, -1.0f,
                        (float)x,     (float)y + 1, (float)z, tex_coords[3].x, tex_coords[3].y, 0.0f, 0.0f, -1.0f
                    });
                    indexOffset += 4;
                }
            }
        }
    }

    dataReady = true;
}

void Chunk::uploadMesh() {
    if (!dataReady) return;

    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    meshDirty = false;
}

void Chunk::render() {
    if (VAO==0) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
