#pragma once

class Texture {
public:
    unsigned int id;
    Texture(const char* path);
    void bind();
};