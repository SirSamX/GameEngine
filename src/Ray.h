
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "RaycastResult.h"
#include <optional>

class World;

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}

    std::optional<RaycastResult> cast(World& world, float maxDist) {
        glm::ivec3 currentBlock = glm::floor(origin);
        glm::vec3 step = glm::sign(direction);
        glm::vec3 tMax = (glm::vec3(currentBlock) + (step + 1.0f) / 2.0f - origin) / direction;
        glm::vec3 tDelta = step / direction;

        for (int i = 0; i < maxDist; ++i) {
            if (world.getBlock(currentBlock) != 0) {
                glm::vec3 face = glm::vec3(0.0f);
                if (tMax.x < tMax.y) {
                    if (tMax.x < tMax.z) face.x = -step.x;
                    else face.z = -step.z;
                } else {
                    if (tMax.y < tMax.z) face.y = -step.y;
                    else face.z = -step.z;
                }
                return RaycastResult{currentBlock, glm::ivec3(face)};
            }

            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    currentBlock.x += step.x;
                    tMax.x += tDelta.x;
                } else {
                    currentBlock.z += step.z;
                    tMax.z += tDelta.z;
                }
            } else {
                if (tMax.y < tMax.z) {
                    currentBlock.y += step.y;
                    tMax.y += tDelta.y;
                } else {
                    currentBlock.z += step.z;
                    tMax.z += tDelta.z;
                }
            }
        }
        return std::nullopt;
    }
};
