#pragma once

#include "Common.h"

namespace ModelPipeline
{
    class Mesh;
    class ModelMaterial;

    class Model
    {
    public:
        Model(const std::string& filename, bool flipUVs = false);
        ~Model();

        bool HasMeshes() const;
        bool HasMaterials() const;

        const std::vector<Mesh*>& Meshes() const;
        const std::vector<ModelMaterial*>& Materials() const;
		Model& GetModel();

		void SaveModel(const char* filePath);
    private:
        Model(const Model& rhs);
        Model& operator=(const Model& rhs);

        std::vector<Mesh*> mMeshes;
        std::vector<ModelMaterial*> mMaterials;
    };
}
