#pragma once
#include "Mesh.h"

class MeshFactory {
public:
    static Mesh plane(float width, float height);
    static Mesh cube(float size);
    static Mesh sphere(float radius, int segments);
};
