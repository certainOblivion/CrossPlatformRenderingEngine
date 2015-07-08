#include "pch.h"
#include "ModelMaterial.h"
#include <memory>

using namespace std;
namespace Library
{
    ModelMaterial::ModelMaterial(Model& model)
        : mModel(model), mTextures()
    {
    }

    ModelMaterial::ModelMaterial(Model& model, std::ifstream& inputFile)
        : mModel(model), mTextures()
    {
		LoadMaterial(inputFile);
    }

    ModelMaterial::~ModelMaterial()
    {
        for (std::pair<TextureType, std::vector<std::string>*> textures : mTextures)
        {
            delete (textures.second);
			textures.second = nullptr;
        }
    }

    Model& ModelMaterial::GetModel()
    {
        return mModel;
    }

    const std::string& ModelMaterial::Name() const
    {
        return mName;
    }

    const std::map<TextureType, std::vector<std::string>*> ModelMaterial::Textures() const
    {
        return mTextures;
    }


	void ModelMaterial::LoadMaterial(std::ifstream& inFile)
	{
		char readData[sizeof(uint32_t)];
		inFile.read(readData, sizeof(uint32_t));
		uint32_t nameLength;
		memcpy(&nameLength, readData, sizeof(uint32_t));

		unique_ptr<char> readName = unique_ptr<char>(new char[nameLength]);
		inFile.read(readName.get(), sizeof(char)* nameLength);
		mName = std::string(readName.get(), nameLength);

		inFile.read(readData, sizeof(uint32_t));
		uint32_t textureTypeCount;
		memcpy(&textureTypeCount, readData, sizeof(uint32_t));

		for (uint32_t i = 0; i < textureTypeCount; i++)
		{
			uint32_t textureTypeInt;
			inFile.read(readData, sizeof(uint32_t));
			memcpy(&textureTypeInt, readData, sizeof(uint32_t));

			TextureType textureType = (TextureType)textureTypeInt;

			uint32_t pathVectorSize;
			inFile.read(readData, sizeof(uint32_t));
			memcpy(&pathVectorSize, readData, sizeof(uint32_t));

			auto pathVector = new std::vector<std::string>();
			pathVector->reserve(pathVectorSize);
			mTextures.insert(std::pair<TextureType, std::vector<std::string>*>(textureType, pathVector));

			for (uint32_t i = 0; i < pathVectorSize; i++)
			{
				inFile.read(readData, sizeof(uint32_t));
				uint32_t pathLength;
				memcpy(&pathLength, readData, sizeof(uint32_t));

				unique_ptr<char> readPath = unique_ptr<char>(new char[pathLength]);
				inFile.read(readPath.get(), sizeof(char)* pathLength);
				string path = std::string(readPath.get(), pathLength);

				pathVector->push_back(path);
			}
		}
	}
}