#pragma once
#include <functional>
#include <GLFW/glfw3.h>
#include <utility>
#include <vector>

struct TimedTask {
    float interval;
    float lastRun;
    std::function<void()> task;
};

class Scheduler {
public:
    void addTask(float interval, std::function<void()> task) {
        tasks.push_back({interval, 0.0f, std::move(task)});
    }

    void update() {
        const auto now = static_cast<float>(glfwGetTime());
        for (auto &t : tasks) {
            if (now - t.lastRun >= t.interval) {
                t.task();
                t.lastRun = now;
            }
        }
    }

private:
    std::vector<TimedTask> tasks;
};
