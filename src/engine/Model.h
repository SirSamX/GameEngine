#pragma once
#include <assimp/scene.h>
#include <string>
#include <vector>
#include "Mesh.h"

class Model {
public:
    explicit Model(const char *path, unsigned int flags = 0) {
        loadModel(path, flags);
    }
    void draw(const Shader &shader) const;
private:
    std::vector<MeshTexture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path, unsigned int flags);
    void processNode(const aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<MeshTexture> loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const std::string &typeName);
};
