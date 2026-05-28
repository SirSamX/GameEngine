#include "MeshFactory.h"
#include <vector>
#include "Mesh.h"
#include <glm/glm.hpp>
#include <cmath>

Mesh MeshFactory::plane(float width, float height) {
    std::vector<Vertex> vertices(4);
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    vertices[0].Position = glm::vec3(-hw, 0.0f, -hh);
    vertices[1].Position = glm::vec3( hw, 0.0f, -hh);
    vertices[2].Position = glm::vec3( hw, 0.0f,  hh);
    vertices[3].Position = glm::vec3(-hw, 0.0f,  hh);

    for (int i = 0; i < 4; ++i)
        vertices[i].Normal = glm::vec3(0.0f, 1.0f, 0.0f);

    vertices[0].TexCoords = glm::vec2(0.0f, 0.0f);
    vertices[1].TexCoords = glm::vec2(1.0f, 0.0f);
    vertices[2].TexCoords = glm::vec2(1.0f, 1.0f);
    vertices[3].TexCoords = glm::vec2(0.0f, 1.0f);

    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
    return {vertices, indices, {}};
}

Mesh MeshFactory::cube(float size) {
    float hs = size / 2.0f;
    std::vector<Vertex> vertices(24);
    std::vector<unsigned int> indices(36);

    glm::vec3 positions[8] = {
        {-hs, -hs, -hs}, { hs, -hs, -hs}, { hs,  hs, -hs}, {-hs,  hs, -hs},
        {-hs, -hs,  hs}, { hs, -hs,  hs}, { hs,  hs,  hs}, {-hs,  hs,  hs}
    };

    unsigned int faceIndices[6][4] = {
        {0,1,2,3}, {5,4,7,6}, {4,0,3,7},
        {1,5,6,2}, {3,2,6,7}, {4,5,1,0}
    };

    glm::vec3 normals[6] = {
        {0,0,-1}, {0,0,1}, {-1,0,0}, {1,0,0}, {0,1,0}, {0,-1,0}
    };

    int vert = 0;
    for (int f = 0; f < 6; ++f) {
        for (int i = 0; i < 4; ++i) {
            vertices[vert].Position = positions[faceIndices[f][i]];
            vertices[vert].Normal = normals[f];
            vertices[vert].TexCoords = glm::vec2(i==0||i==3?0.0f:1.0f, i<2?0.0f:1.0f);
            vert++;
        }
    }

    unsigned int idx = 0;
    for (int f = 0; f < 6; ++f) {
        int base = f*4;
        indices[idx++] = base+0; indices[idx++] = base+1; indices[idx++] = base+2;
        indices[idx++] = base+2; indices[idx++] = base+3; indices[idx++] = base+0;
    }

    return {vertices, indices, {}};
}

Mesh MeshFactory::sphere(float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = static_cast<float>(x) / segments;
            float ySegment = static_cast<float>(y) / segments;
            float xPos = radius * cos(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);
            float yPos = radius * cos(ySegment * M_PI);
            float zPos = radius * sin(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);

            Vertex v;
            v.Position = glm::vec3(xPos, yPos, zPos);
            v.Normal = glm::normalize(v.Position);
            v.TexCoords = glm::vec2(xSegment, ySegment);
            vertices.push_back(v);
        }
    }

    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            int i0 = y * (segments + 1) + x;
            int i1 = i0 + segments + 1;
            indices.push_back(i0); indices.push_back(i1); indices.push_back(i0+1);
            indices.push_back(i0+1); indices.push_back(i1); indices.push_back(i1+1);
        }
    }

    return {vertices, indices, {}};
}