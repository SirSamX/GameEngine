#pragma once
#include <glm/glm.hpp>
#include <optional>


struct RaycastResult {
    glm::ivec3 blockPos;
    glm::ivec3 face;
};

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}

    std::optional<RaycastResult> cast(World& world, float maxDist) const {
        glm::ivec3 currentBlock = glm::floor(origin);
        glm::vec3 step = glm::sign(direction);
        glm::vec3 tMax = (glm::vec3(currentBlock) + (step + 1.0f) / 2.0f - origin) / direction;
        glm::vec3 tDelta = step / direction;
        glm::ivec3 face(0);

        for (int i = 0; i < maxDist; ++i) {
            if (world.getBlock(currentBlock) != 0) {
                return RaycastResult{currentBlock, face};
            }

            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    currentBlock.x += static_cast<int>(step.x);
                    tMax.x += tDelta.x;
                    face = glm::ivec3(-step.x, 0, 0);
                } else {
                    currentBlock.z += static_cast<int>(step.z);
                    tMax.z += tDelta.z;
                    face = glm::ivec3(0, 0, -step.z);
                }
            } else {
                if (tMax.y < tMax.z) {
                    currentBlock.y += static_cast<int>(step.y);
                    tMax.y += tDelta.y;
                    face = glm::ivec3(0, -step.y, 0);
                } else {
                    currentBlock.z += static_cast<int>(step.z);
                    tMax.z += tDelta.z;
                    face = glm::ivec3(0, 0, -step.z);
                }
            }
        }
        return std::nullopt;
    }
};
