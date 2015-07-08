#pragma once
#include <vector>

namespace Library
{
    class Mesh;
    class ModelMaterial;

    class Model
    {
    public:
        Model(const std::string& filePath, bool flipUVs = false);
        ~Model();

        bool HasMeshes() const;
        bool HasMaterials() const;

        const std::vector<Mesh*>& Meshes() const;
        const std::vector<ModelMaterial*>& Materials() const;
		Model& GetModel();

    private:
        Model(const Model& rhs);
        Model& operator=(const Model& rhs);
		void LoadMeshes(std::ifstream& inputFile, uint32_t numMesh);
		void LoadMaterials(std::ifstream& inputFile, uint32_t numMaterials);
        std::vector<Mesh*> mMeshes;
        std::vector<ModelMaterial*> mMaterials;
    };
}
