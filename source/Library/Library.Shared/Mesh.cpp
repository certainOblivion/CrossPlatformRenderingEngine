#include "pch.h"
#include "Mesh.h"
#include "Model.h"
#include "ModelMaterial.h"
#include <assert.h>

using namespace DirectX;
using namespace std;

namespace Library
{
	Mesh::Mesh(Model& model, std::ifstream& inputFile)
        : mModel(model), mMaterial(nullptr), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
    {
		LoadMesh(inputFile);
    }


	void Mesh::LoadMesh(std::ifstream& inputFile)
	{
		//reading length of name and then reading the name to look for the material in the model
 		char readUnsignedInt[sizeof(uint32_t)];
 		inputFile.read(readUnsignedInt, sizeof(uint32_t));
 		uint32_t nameLength;
		memcpy(&nameLength, readUnsignedInt, sizeof(uint32_t));

 		unique_ptr<char> readName = unique_ptr<char>(new char[nameLength]);
 		inputFile.read(readName.get(), sizeof(char)* nameLength);
		string materialName = std::string(readName.get(), nameLength);
		auto& materials = mModel.Materials();
		for (auto material: materials)
		{
			if (material->Name() == materialName)
			{
				mMaterial = material;
			}
		}

		//reading index count and then reading all the indices
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t indexCount;
		memcpy(&indexCount, readUnsignedInt, sizeof(uint32_t));

		mIndices.reserve(indexCount);

		for (uint32_t i = 0; i < indexCount; i++)
		{
			char readChar[sizeof(char32_t)];
			inputFile.read(readChar, sizeof(char32_t));
			char32_t index;
			memcpy(&index, readChar, sizeof(char32_t));
			mIndices.push_back(index);
		}

		//reading vertex count and then reading individual vertices
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t vertexCount;
		memcpy(&vertexCount, readUnsignedInt, sizeof(uint32_t));

		mVertices.reserve(vertexCount);

		for (uint32_t i = 0; i < vertexCount; i++)
		{
			char readChar[sizeof(DirectX::XMFLOAT3)];
			inputFile.read(readChar, sizeof(DirectX::XMFLOAT3));
			DirectX::XMFLOAT3 vertex;
			memcpy(&vertex, readChar, sizeof(DirectX::XMFLOAT3));
			mVertices.push_back(vertex);
		}

		//reading normal count and then reading individual normals
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t normalCount;
		memcpy(&normalCount, readUnsignedInt, sizeof(uint32_t));

		mNormals.reserve(normalCount);

		for (uint32_t i = 0; i < normalCount; i++)
		{
			char readChar[sizeof(DirectX::XMFLOAT3)];
			inputFile.read(readChar, sizeof(DirectX::XMFLOAT3));
			DirectX::XMFLOAT3 normal;
			memcpy(&normal, readChar, sizeof(DirectX::XMFLOAT3));
			mNormals.push_back(normal);
		}

		//reading tangent count and then reading individual tangents
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t tangentCount;
		memcpy(&tangentCount, readUnsignedInt, sizeof(uint32_t));

		mTangents.reserve(tangentCount);

		for (uint32_t i = 0; i < tangentCount; i++)
		{
			char readChar[sizeof(DirectX::XMFLOAT3)];
			inputFile.read(readChar, sizeof(DirectX::XMFLOAT3));
			DirectX::XMFLOAT3 tangent;
			memcpy(&tangent, readChar, sizeof(DirectX::XMFLOAT3));
			mTangents.push_back(tangent);
		}

		//reading binormal count and then reading individual Binormals
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t biNormalCount;
		memcpy(&biNormalCount, readUnsignedInt, sizeof(uint32_t));

		mBiNormals.reserve(biNormalCount);

		for (uint32_t i = 0; i < biNormalCount; i++)
		{
			char readChar[sizeof(DirectX::XMFLOAT3)];
			inputFile.read(readChar, sizeof(DirectX::XMFLOAT3));
			DirectX::XMFLOAT3 biNormal;
			memcpy(&biNormal, readChar, sizeof(DirectX::XMFLOAT3));
			mBiNormals.push_back(biNormal);
		}

		//reading textureCoordinate count and then reading individual textureCoordinates
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t uvChannelCount;
		memcpy(&uvChannelCount, readUnsignedInt, sizeof(uint32_t));
		for (uint32_t i = 0; i < uvChannelCount; i++)
		{
			uint32_t vertexCount = (uint32_t)mVertices.size();
			std::vector<XMFLOAT3>* textureCoordinates = new std::vector<XMFLOAT3>();
			textureCoordinates->reserve(vertexCount);
			mTextureCoordinates.push_back(textureCoordinates);

			for (uint32_t j = 0; j < vertexCount; j++)
			{
				char readChar[sizeof(DirectX::XMFLOAT3)];
				inputFile.read(readChar, sizeof(DirectX::XMFLOAT3));
				DirectX::XMFLOAT3 textureCoord;
				memcpy(&textureCoord, readChar, sizeof(DirectX::XMFLOAT3));
				textureCoordinates->push_back(textureCoord);
			}
		}

		//reading vertex color count and then reading individual vertex colors
		inputFile.read(readUnsignedInt, sizeof(uint32_t));
		uint32_t colorChannelCount;
		memcpy(&colorChannelCount, readUnsignedInt, sizeof(uint32_t));
		for (uint32_t i = 0; i < colorChannelCount; i++)
		{
			std::vector<XMFLOAT4>* vertexColors = new std::vector<XMFLOAT4>();
			vertexColors->reserve(mVertices.size());
			mVertexColors.push_back(vertexColors);

			for (uint32_t j = 0; j < mVertices.size(); j++)
			{
				char readChar[sizeof(DirectX::XMFLOAT4)];
				inputFile.read(readChar, sizeof(DirectX::XMFLOAT4));
				DirectX::XMFLOAT4 vertexColor;
				memcpy(&vertexColor, readChar, sizeof(DirectX::XMFLOAT4));
				vertexColors->push_back(vertexColor);
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

	void Mesh::CreateIndexBuffer(ID3D11Buffer** indexBuffer, ID3D11Device2* device)
	{
		assert(indexBuffer != nullptr);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = (UINT)(sizeof(uint32_t) * mIndices.size());
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = &mIndices[0];
		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexSubResourceData, indexBuffer)))
		{
			throw exception("ID3D11Device::CreateBuffer() failed.");
		}
	}

}