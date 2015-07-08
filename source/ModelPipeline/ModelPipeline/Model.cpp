#include "pch.h"
#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ModelPipeline
{
    Model::Model(const std::string& filename, bool flipUVs)
        : mMeshes(), mMaterials()
    {
        Assimp::Importer importer;

        UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
        if (flipUVs)
        {
            flags |= aiProcess_FlipUVs;
        }

        const aiScene* scene = importer.ReadFile(filename, flags);
        if (scene == nullptr)
        {
            throw std::exception("Cannot load obj!");
        }

        if (scene->HasMaterials())
        {
            for (UINT i = 0; i < scene->mNumMaterials; i++)
            {
                mMaterials.push_back(new ModelMaterial(*this, scene->mMaterials[i]));
            }
        }

        if (scene->HasMeshes())
        {
            for (UINT i = 0; i < scene->mNumMeshes; i++)
            {
                ModelMaterial* material = (mMaterials.size() > i ? mMaterials.at(i) : nullptr);

                Mesh* mesh = new Mesh(*this, *(scene->mMeshes[i]));
                mMeshes.push_back(mesh);
            }
        }
    }

    Model::~Model()
    {
        for (Mesh* mesh : mMeshes)
        {
            delete mesh;
        }

        for (ModelMaterial* material : mMaterials)
        {
            delete material;
        }
    }

    bool Model::HasMeshes() const
    {
        return (mMeshes.size() > 0);
    }

    bool Model::HasMaterials() const
    {
        return (mMaterials.size() > 0);
    }

    const std::vector<Mesh*>& Model::Meshes() const
    {
        return mMeshes;
    }

    const std::vector<ModelMaterial*>& Model::Materials() const
    {
        return mMaterials;
    }

	Model& Model::GetModel()
	{
		return *this;
	}


	void Model::SaveModel(const char* filePath)
	{
		std::ofstream outFile(filePath, std::ios::binary);

		if (outFile.is_open())
		{
			uint32_t meshCount = mMeshes.size();
			uint32_t materialCount = mMaterials.size();

			outFile.write(reinterpret_cast<const char*>(&materialCount), sizeof(uint32_t));
			outFile.write(reinterpret_cast<const char*>(&meshCount), sizeof(uint32_t));

			for (auto material : mMaterials)
			{
				material->SaveMaterial(outFile);
			}

			for (auto mesh : mMeshes)
			{
				mesh->SaveMesh(outFile);
			}
			outFile.close();
		}
		else
		{
			throw std::exception("Error opening file!");
		}
	}

}
