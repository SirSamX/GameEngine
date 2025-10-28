#pragma once

class Noise {
public:
    static float generate(float x, float z) {
        // Simple noise function for demonstration purposes
        return (sin(x * 0.1f) * cos(z * 0.1f) + 1.0f) * 0.5f;
    }
};
