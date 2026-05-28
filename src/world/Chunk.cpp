#include <GL/glew.h>
#include "Chunk.h"
#include "engine/Noise.h"
#include "World.h"
#include <vector>
#include <map>
#include "Block.h"

std::map<Block, std::vector<glm::vec2>> block_textures = {
    {Block::Grass, {glm::vec2(0.0f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)}},
    {Block::Stone, {glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.5f), glm::vec2(0.0f, 0.5f)}}
};

Chunk::Chunk(glm::ivec2 pos) : position(pos), blocks(WIDTH * HEIGHT * DEPTH, 0), meshDirty(true) {
    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float height = Noise::generate(position.x * WIDTH + x, position.y * DEPTH + z) * HEIGHT;
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
        chunk.position.y * Chunk::DEPTH + z
    };
    return world.getBlock(worldPos);
}

void Chunk::generateMeshData(const World& world) {
    vertices.clear();
    indices.clear();

    // Optimization: Reserve memory upfront to avoid massive heap reallocation cycles
    // (Assumes an average chunk mesh fills a fraction of total possible capacity)
    vertices.reserve(WIDTH * HEIGHT * DEPTH * 4);
    indices.reserve(WIDTH * HEIGHT * DEPTH * 6);

    unsigned int indexOffset = 0;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int z = 0; z < DEPTH; ++z) {
            for (int x = 0; x < WIDTH; ++x) {
                uint8_t block_id = getBlock(x, y, z);
                if (block_id == 0) continue;

                auto block_type = static_cast<Block>(block_id);
                auto tex_coords = block_textures[block_type];

                // Convert loop variables to floats once per block to avoid repeated casting
                const auto fx = static_cast<float>(x);
                const auto fy = static_cast<float>(y);
                const auto fz = static_cast<float>(z);

                // --- TOP FACE ---
                if (getBlock_internal(x, y + 1, z, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx,        fy + 1.0f, fz,        tex_coords[0].x, tex_coords[0].y, 0.0f, 1.0f, 0.0f,
                        fx + 1.0f, fy + 1.0f, fz,        tex_coords[1].x, tex_coords[1].y, 0.0f, 1.0f, 0.0f,
                        fx + 1.0f, fy + 1.0f, fz + 1.0f, tex_coords[2].x, tex_coords[2].y, 0.0f, 1.0f, 0.0f,
                        fx,        fy + 1.0f, fz + 1.0f, tex_coords[3].x, tex_coords[3].y, 0.0f, 1.0f, 0.0f
                    });
                    indexOffset += 4;
                }

                // --- BOTTOM FACE ---
                if (getBlock_internal(x, y - 1, z, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx,        fy,        fz,        tex_coords[0].x, tex_coords[0].y, 0.0f, -1.0f, 0.0f,
                        fx + 1.0f, fy,        fz,        tex_coords[1].x, tex_coords[1].y, 0.0f, -1.0f, 0.0f,
                        fx + 1.0f, fy,        fz + 1.0f, tex_coords[2].x, tex_coords[2].y, 0.0f, -1.0f, 0.0f,
                        fx,        fy,        fz + 1.0f, tex_coords[3].x, tex_coords[3].y, 0.0f, -1.0f, 0.0f
                    });
                    indexOffset += 4;
                }

                // --- RIGHT FACE ---
                if (getBlock_internal(x + 1, y, z, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx + 1.0f, fy,        fz,        tex_coords[0].x, tex_coords[0].y, 1.0f, 0.0f, 0.0f,
                        fx + 1.0f, fy + 1.0f, fz,        tex_coords[1].x, tex_coords[1].y, 1.0f, 0.0f, 0.0f,
                        fx + 1.0f, fy + 1.0f, fz + 1.0f, tex_coords[2].x, tex_coords[2].y, 1.0f, 0.0f, 0.0f,
                        fx + 1.0f, fy,        fz + 1.0f, tex_coords[3].x, tex_coords[3].y, 1.0f, 0.0f, 0.0f
                    });
                    indexOffset += 4;
                }

                // --- LEFT FACE ---
                if (getBlock_internal(x - 1, y, z, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx,        fy,        fz,        tex_coords[0].x, tex_coords[0].y, -1.0f, 0.0f, 0.0f,
                        fx,        fy + 1.0f, fz,        tex_coords[1].x, tex_coords[1].y, -1.0f, 0.0f, 0.0f,
                        fx,        fy + 1.0f, fz + 1.0f, tex_coords[2].x, tex_coords[2].y, -1.0f, 0.0f, 0.0f,
                        fx,        fy,        fz + 1.0f, tex_coords[3].x, tex_coords[3].y, -1.0f, 0.0f, 0.0f
                    });
                    indexOffset += 4;
                }

                // --- FRONT FACE ---
                if (getBlock_internal(x, y, z + 1, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx,        fy,        fz + 1.0f, tex_coords[0].x, tex_coords[0].y, 0.0f, 0.0f, 1.0f,
                        fx + 1.0f, fy,        fz + 1.0f, tex_coords[1].x, tex_coords[1].y, 0.0f, 0.0f, 1.0f,
                        fx + 1.0f, fy + 1.0f, fz + 1.0f, tex_coords[2].x, tex_coords[2].y, 0.0f, 0.0f, 1.0f,
                        fx,        fy + 1.0f, fz + 1.0f, tex_coords[3].x, tex_coords[3].y, 0.0f, 0.0f, 1.0f
                    });
                    indexOffset += 4;
                }

                // --- BACK FACE ---
                if (getBlock_internal(x, y, z - 1, *this, world) == 0) {
                    indices.insert(indices.end(), {indexOffset, indexOffset + 1, indexOffset + 2, indexOffset + 2, indexOffset + 3, indexOffset});
                    vertices.insert(vertices.end(), {
                        fx,        fy,        fz,        tex_coords[0].x, tex_coords[0].y, 0.0f, 0.0f, -1.0f,
                        fx + 1.0f, fy,        fz,        tex_coords[1].x, tex_coords[1].y, 0.0f, 0.0f, -1.0f,
                        fx + 1.0f, fy + 1.0f, fz,        tex_coords[2].x, tex_coords[2].y, 0.0f, 0.0f, -1.0f,
                        fx,        fy + 1.0f, fz,        tex_coords[3].x, tex_coords[3].y, 0.0f, 0.0f, -1.0f
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
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size()*sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size()*sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void *>(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));

    glBindVertexArray(0);
    meshDirty = false;
}

void Chunk::render() const {
    if (VAO==0) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
