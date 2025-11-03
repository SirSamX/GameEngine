#pragma once

class Texture {
public:
    unsigned int texture;
    Texture(const char* path);
    void bind();
};