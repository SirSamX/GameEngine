#pragma once
#include <glm/gtc/noise.hpp>

class Noise {
public:
    static float generate(float x, float z) {
        float n = glm::perlin(glm::vec2(x / 64.0f, z / 64.0f));
        return (n + 1.0f) * 0.5f;
    }
};
