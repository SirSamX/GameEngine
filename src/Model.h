#pragma once
#include <assimp/scene.h>
#include <string>
#include <vector>
#include "Mesh.h"

class Model {
public:
    Model(const char *path, unsigned int flags = 0) {
        loadModel(path, flags);
    }
    void draw(Shader &shader);
private:
    std::vector<MeshTexture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path, unsigned int flags);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<MeshTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
