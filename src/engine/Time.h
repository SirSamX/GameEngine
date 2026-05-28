#pragma once

class Time {
    double lastFrameTime = 0.0;
    float deltaTime = 0.0f;
    float totalTime = 0.0f;

public:
    void update();

    [[nodiscard]] float getDeltaTime() const { return deltaTime; }
    [[nodiscard]] float getTotalTime() const { return totalTime; }
};

extern Time gameTime;
