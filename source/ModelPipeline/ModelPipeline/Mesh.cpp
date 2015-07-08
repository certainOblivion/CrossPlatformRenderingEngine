#include "pch.h"
#include "Mesh.h"
#include "Model.h"
#include <assimp/scene.h>

namespace ModelPipeline
{
    Mesh::Mesh(Model& model, aiMesh& mesh)
        : mModel(model), mMaterial(nullptr), mName(mesh.mName.C_Str()), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
    {
		mMaterial = mModel.Materials().at(mesh.mMaterialIndex);

        // Vertices
        mVertices.reserve(mesh.mNumVertices);
        for (UINT i = 0; i < mesh.mNumVertices; i++)
        {
            mVertices.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mVertices[i])));			
        }

        // Normals
        if (mesh.HasNormals())
        {
            mNormals.reserve(mesh.mNumVertices);
            for (UINT i = 0; i < mesh.mNumVertices; i++)
            {
                mNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mNormals[i])));
            }
        }

        // Tangents and Binormals
        if (mesh.HasTangentsAndBitangents())
        {
            mTangents.reserve(mesh.mNumVertices);
            mBiNormals.reserve(mesh.mNumVertices);
            for (UINT i = 0; i < mesh.mNumVertices; i++)
            {
                mTangents.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mTangents[i])));
                mBiNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mBitangents[i])));
            }
        }

        // Texture Coordinates
        UINT uvChannelCount = mesh.GetNumUVChannels();
        for (UINT i = 0; i < uvChannelCount; i++)
        {
            std::vector<XMFLOAT3>* textureCoordinates = new std::vector<XMFLOAT3>();
            textureCoordinates->reserve(mesh.mNumVertices);
            mTextureCoordinates.push_back(textureCoordinates);

            aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
            for (UINT j = 0; j < mesh.mNumVertices; j++)
            {
                textureCoordinates->push_back(XMFLOAT3(reinterpret_cast<const float*>(&aiTextureCoordinates[j])));
            }
        }

        // Vertex Colors
        UINT colorChannelCount = mesh.GetNumColorChannels();
        for (UINT i = 0; i < colorChannelCount; i++)
        {
            std::vector<XMFLOAT4>* vertexColors = new std::vector<XMFLOAT4>();
            vertexColors->reserve(mesh.mNumVertices);
            mVertexColors.push_back(vertexColors);

            aiColor4D* aiVertexColors = mesh.mColors[i];
            for (UINT j = 0; j < mesh.mNumVertices; j++)
            {
                vertexColors->push_back(XMFLOAT4(reinterpret_cast<const float*>(&aiVertexColors[j])));
            }
        }

        // Faces (note: could pre-reserve if we limit primitive types)
        if (mesh.HasFaces())
        {
            mFaceCount = mesh.mNumFaces;
            for (UINT i = 0; i < mFaceCount; i++)
            {
                aiFace* face = &mesh.mFaces[i];
            
                for (UINT j = 0; j < face->mNumIndices; j++)
                {		
                    mIndices.push_back(face->mIndices[j]);
                }
            }
        }
    }

    Mesh::~Mesh()
    {
        for (std::vector<XMFLOAT3>* textureCoordinates : mTextureCoordinates)
        {
            delete textureCoordinates;
        }

        for (std::vector<XMFLOAT4>* vertexColors : mVertexColors)
        {
            delete vertexColors;
        }
    }

    Model& Mesh::GetModel()
    {
        return mModel;
    }

    ModelMaterial* Mesh::GetMaterial()
    {
        return mMaterial;
    }

    const std::string& Mesh::Name() const
    {
        return mName;
    }

    const std::vector<XMFLOAT3>& Mesh::Vertices() const
    {
        return mVertices;
    }

    const std::vector<XMFLOAT3>& Mesh::Normals() const
    {
        return mNormals;
    }

    const std::vector<XMFLOAT3>& Mesh::Tangents() const
    {
        return mTangents;
    }

    const std::vector<XMFLOAT3>& Mesh::BiNormals() const
    {
        return mBiNormals;
    }

    const std::vector<std::vector<XMFLOAT3>*>& Mesh::TextureCoordinates() const
    {
        return mTextureCoordinates;
    }

    const std::vector<std::vector<XMFLOAT4>*>& Mesh::VertexColors() const
    {
        return mVertexColors;
    }

    UINT Mesh::FaceCount() const
    {
        return mFaceCount;
    }

    const std::vector<UINT>& Mesh::Indices() const
    {
        return mIndices;
    }

	void Mesh::SaveMesh(std::ofstream& outFile)
	{
		std::string materialName = mMaterial->Name();
		uint32_t lengthOfName = materialName.length();

		outFile.write(reinterpret_cast<const char*>(&lengthOfName), sizeof(uint32_t));
		outFile.write(materialName.c_str(), sizeof(char)*lengthOfName);


		//writing number of indices and then the individual indices
		auto indexCount = Indices().size();
		outFile.write(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

		for (unsigned int i = 0; i < indexCount; i++)
		{
			char32_t index = mIndices[i];
			outFile.write(reinterpret_cast<const char*>(&index), sizeof(char32_t));
		}

		//writing the number of vertices and then the individual vertices
		auto size = mVertices.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			DirectX::XMFLOAT3 vertex = mVertices.at(i);
			outFile.write(reinterpret_cast<const char*>(&vertex), sizeof(DirectX::XMFLOAT3));
		}

		//writing the number of normals and then the individual normals
		size = Normals().size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			DirectX::XMFLOAT3 normal = mNormals.at(i);
			outFile.write(reinterpret_cast<const char*>(&normal), sizeof(DirectX::XMFLOAT3));
		}

		//writing the number of tangents and then the individual tangents
		size = mTangents.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			DirectX::XMFLOAT3 tangent = mTangents.at(i);
			outFile.write(reinterpret_cast<const char*>(&tangent), sizeof(DirectX::XMFLOAT3));
		}

		//writing the number of binormals and then the individual binormals
		size = mBiNormals.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			DirectX::XMFLOAT3 binormal = mBiNormals.at(i);
			outFile.write(reinterpret_cast<const char*>(&binormal), sizeof(DirectX::XMFLOAT3));
		}

		//writing the number of textureCoordinates and then the individual textureCoordinates
		size = mTextureCoordinates.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			std::vector<DirectX::XMFLOAT3>* vertCoordinates = mTextureCoordinates.at(i);
			uint32_t vectorSize = vertCoordinates->size();

			for (uint32_t j = 0; j < vectorSize; j++)
			{
				DirectX::XMFLOAT3 textureCoord = vertCoordinates->at(j);
				outFile.write(reinterpret_cast<const char*>(&textureCoord), sizeof(DirectX::XMFLOAT3));
			}
		}
		//writing the number of vertexColors and then the individual vertexColors
		size = mVertexColors.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

		for (unsigned int i = 0; i < size; i++)
		{
			std::vector<DirectX::XMFLOAT4>* vertColors = mVertexColors.at(i);
			uint32_t vectorSize = vertColors->size();

			for (uint32_t j = 0; j < vectorSize; j++)
			{
				DirectX::XMFLOAT4 vertColor = vertColors->at(j);
				outFile.write(reinterpret_cast<const char*>(&vertColor), sizeof(DirectX::XMFLOAT4));
			}
		}
		//writing the number of facecount

	}

}