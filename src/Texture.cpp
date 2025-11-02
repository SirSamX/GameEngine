#include "Texture.h"
#include <iostream>
#include <GL/glew.h>
#include "stb_image.h"

Texture::Texture(const char* path) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        std::cout << "Loaded texture: " << path << " with " << nrChannels << " channels, " << width << "x" << height << std::endl;
        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;
        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
        }
        else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        }
        else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
}

void Texture::bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}