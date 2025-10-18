#include <functional>
#include <GLFW/glfw3.h>
#include <vector>

struct TimedTask {
    float interval;
    float lastRun;
    std::function<void()> task;
};

class Scheduler {
public:
    void addTask(float interval, std::function<void()> task) {
        tasks.push_back({interval, 0.0f, task});
    }

    void update() {
        float now = static_cast<float>(glfwGetTime());
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
