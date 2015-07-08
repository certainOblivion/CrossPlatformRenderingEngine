#include "pch.h"
#include "ModelMaterial.h"
#include "Utility.h"
#include <assimp/scene.h>

namespace ModelPipeline
{
    std::map<TextureType, UINT> ModelMaterial::sTextureTypeMappings;

    ModelMaterial::ModelMaterial(Model& model)
        : mModel(model), mTextures()
    {
        InitializeTextureTypeMappings();
    }

    ModelMaterial::ModelMaterial(Model& model, aiMaterial* material)
        : mModel(model), mTextures()
    {
        InitializeTextureTypeMappings();

        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        mName = name.C_Str();

        for (TextureType textureType = (TextureType)0; textureType < TextureTypeEnd; textureType = (TextureType)(textureType + 1))
        {
            aiTextureType mappedTextureType = (aiTextureType)sTextureTypeMappings[textureType];

            UINT textureCount = material->GetTextureCount(mappedTextureType);
            if (textureCount > 0)
            {
                std::vector<std::string>* textures = new std::vector<std::string>();
                mTextures.insert(std::pair<TextureType, std::vector<std::string>*>(textureType, textures));

                textures->reserve(textureCount);
                for (UINT textureIndex = 0; textureIndex < textureCount; textureIndex++)
                {
                    aiString path;
                    if (material->GetTexture(mappedTextureType, textureIndex, &path) == AI_SUCCESS)
                    {
						std::string wPath = std::string(path.C_Str());
                        textures->push_back(wPath);
                    }
                }
            }
        }
    }

    ModelMaterial::~ModelMaterial()
    {
        for (std::pair<TextureType, std::vector<std::string>*> textures : mTextures)
        {
            DeleteObject(textures.second);
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


	void ModelMaterial::SaveMaterial(std::ofstream& outFile)
	{
		uint32_t nameLength = mName.length();
		outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));

		outFile.write(mName.c_str(), sizeof(char)*nameLength);

		uint32_t textureTypeCount = mTextures.size();
		outFile.write(reinterpret_cast<const char*>(&textureTypeCount), sizeof(uint32_t));

		for (auto texturePair : mTextures)
		{
			uint32_t textureType = (uint32_t)texturePair.first;
			outFile.write(reinterpret_cast<const char*>(&textureType), sizeof(uint32_t));

			auto* textureVector = texturePair.second;
			uint32_t textureVectorCount = textureVector->size();
			outFile.write(reinterpret_cast<const char*>(&textureVectorCount), sizeof(uint32_t));

			for (auto path: *textureVector)
			{
				uint32_t pathLength = path.length();
				outFile.write(reinterpret_cast<const char*>(&pathLength), sizeof(uint32_t));
				outFile.write(path.c_str(), sizeof(char)* pathLength);
			}
		}
	}

	void ModelMaterial::InitializeTextureTypeMappings()
    {
        if (sTextureTypeMappings.size() != TextureTypeEnd)
        {
            sTextureTypeMappings[TextureTypeDifffuse] = aiTextureType_DIFFUSE;
            sTextureTypeMappings[TextureTypeSpecularMap] = aiTextureType_SPECULAR;
            sTextureTypeMappings[TextureTypeAmbient] = aiTextureType_AMBIENT;
            sTextureTypeMappings[TextureTypeHeightmap] = aiTextureType_HEIGHT;
            sTextureTypeMappings[TextureTypeNormalMap] = aiTextureType_NORMALS;
            sTextureTypeMappings[TextureTypeSpecularPowerMap] = aiTextureType_SHININESS;
            sTextureTypeMappings[TextureTypeDisplacementMap] = aiTextureType_DISPLACEMENT;
            sTextureTypeMappings[TextureTypeLightMap] = aiTextureType_LIGHTMAP;
        }
    }
}