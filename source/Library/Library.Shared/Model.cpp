#include "pch.h"
#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include <memory>
#include <iostream>
#include <sstream>

using namespace std;
namespace Library
{
	Model::Model(const std::string& filePath, bool flipUVs)
		: mMeshes(), mMaterials()
	{
		std::ifstream inFile(filePath, ifstream::in | ifstream::binary);
		if (inFile.is_open())
		{
			char readData[sizeof(uint32_t)];

			inFile.read(readData, sizeof(uint32_t));
			uint32_t numMat;
			memcpy(&numMat, readData, sizeof(uint32_t));

			inFile.read(readData, sizeof(uint32_t));
			uint32_t numMesh;
			memcpy(&numMesh, readData, sizeof(uint32_t));

			LoadMaterials(inFile, numMat);
			LoadMeshes(inFile, numMesh);

			inFile.close();
		}
	}

	void Model::LoadMeshes(std::ifstream& inputFile, uint32_t numMesh)
	{
		mMeshes.reserve(numMesh);
		for (uint32_t i = 0; i < numMesh; i++)
		{
			Mesh* newMesh = new Mesh(*this, inputFile);
			mMeshes.push_back(newMesh);
		}
	}

	void Model::LoadMaterials(std::ifstream& inputFile, uint32_t numMaterials)
	{
		mMaterials.reserve(numMaterials);
		for (uint32_t i = 0; i < numMaterials; i++)
		{
			ModelMaterial* newMaterial = new ModelMaterial(*this, inputFile);
			mMaterials.push_back(newMaterial);
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

}
