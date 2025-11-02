#pragma once

class Texture {
private:
    unsigned int texture;
public:
    Texture(const char* path);
    void bind();
};