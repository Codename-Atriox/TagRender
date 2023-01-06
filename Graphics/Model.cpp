#include "Model.h"

bool Model::Initialize(const std::string& filepath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;

	try
	{
		if (!this->LoadModel(filepath))
			return false;

		//Vertex v[] =
		//{
		//	Vertex(-0.5f,-0.5f,-0.5f,  0.0f,1.0f), // FRONT bottom left
		//	Vertex(-0.5f, 0.5f,-0.5f,  0.0f,0.0f), // FRONT top left
		//	Vertex(0.5f, 0.5f,-0.5f,  1.0f,0.0f), // FRONT top right
		//	Vertex(0.5f,-0.5f,-0.5f,  1.0f,1.0f), // FRONT bottom right

		//	Vertex(-0.5f,-0.5f, 0.5f,  0.0f,1.0f), // BACK bottom left
		//	Vertex(-0.5f, 0.5f, 0.5f,  0.0f,0.0f), // BACK top left
		//	Vertex(0.5f, 0.5f, 0.5f,  1.0f,0.0f), // BACK top right
		//	Vertex(0.5f,-0.5f, 0.5f,  1.0f,1.0f), // BACK bottom right
		//};

		//// LOAD VERTEX DATA
		//HRESULT hr = this->vertexBuffer.Initialize(this->device, v, ARRAYSIZE(v));   // this->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, this->vertexBuffer.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create vertex buffer");

		//DWORD indicies[] =
		//{
		//	0, 1, 2, // front 
		//	0, 2, 3, // front
		//	4, 7, 6, // back
		//	4, 6, 5, // back
		//	3, 2, 6, // right
		//	3, 6, 7, // right
		//	4, 5, 1, // left
		//	4, 1, 0, // left
		//	1, 5, 6, // top
		//	1, 6, 2, // top
		//	0, 3, 7, // bottom
		//	0, 7, 4  // bottom
		//};

		//// LOAD INDEX DATA
		//hr = this->indexBuffer.Initialize(this->device, indicies, ARRAYSIZE(indicies)); // device->CreateBuffer(&indexBufferDesc, &indexBufferData, indiciesBuffer.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create indicies buffer");
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}


void Model::Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
{
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());

	for (int i = 0; i < meshes.size(); i++)
	{
		this->cb_vs_vertexshader->data.wvpMatrix = meshes[i].GetTransformMatrix() * worldMatrix * viewProjectionMatrix;
		//this->cb_vs_vertexshader->data.wvpMatrix = XMMatrixTranspose(this->cb_vs_vertexshader->data.mat);
		this->cb_vs_vertexshader->data.worldMatrix = meshes[i].GetTransformMatrix() * worldMatrix;
		this->cb_vs_vertexshader->ApplyChanges();
		meshes[i].Draw();
	}
}

bool Model::LoadModel(const std::string& filepath)
{
	this->directory = StringHelper::GetDirectoryFromFilePath(filepath);
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filepath,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	this->ProcessNode(pScene->mRootNode, pScene, DirectX::XMMatrixIdentity());
	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix)
{
	XMMATRIX nodeTransformMatrix = XMMatrixTranspose(XMMATRIX(&node->mTransformation.a1)) * parentTransformMatrix;

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(this->ProcessMesh(mesh, scene, nodeTransformMatrix));
	}
	for (UINT i = 0; i < node->mNumChildren; i++) 
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}

}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix)
{
	std::vector<Vertex> verticies;
	std::vector<DWORD> indicies;

	// SETUP VERTS
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		verticies.push_back(vertex);
	}
	// SETUP INDICIES
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indicies.push_back(face.mIndices[j]);
	}

	std::vector<Texture> textures;
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE, scene);
	textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());

	return Mesh(this->device, this->deviceContext, verticies, indicies, textures, transformMatrix);
}

TextureStorageType Model::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();
	// index embedded textures
	if (texturePath[0] == '*')
	{
		if (pScene->mTextures[0]->mHeight == 0)
		{
			return TextureStorageType::EmbeddedIndexCompressed;
		}
		else 
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
	}
	// non indexed embedded texture
	if (auto pTex = pScene->GetEmbeddedTexture(texturePath.c_str()))
	{
		if (pTex->mHeight == 0)
		{
			return TextureStorageType::EmbeddedCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSED TEXTURES" && 0);
			return TextureStorageType::EmbeddedNonCompressed;
		}
	}
	// disk file texture
	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}
	// no texture
	return TextureStorageType::None;
}


std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);
	if (textureCount == 0) // if theres not textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) // if color is black, use gray?
			{
				materialTextures.push_back(Texture(this->device, Colors::UnloadedTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);

			switch (storetype)
			{
			case TextureStorageType::EmbeddedIndexCompressed:
			{
				int index = GetTextureIndex(&path);
				Texture embeddedIndexedTexture(this->device,
					reinterpret_cast<uint8_t*>(pScene->mTextures[index]->pcData),
					pScene->mTextures[index]->mWidth,
					textureType);
				materialTextures.push_back(embeddedIndexedTexture);
				break;
			}
			case TextureStorageType::EmbeddedCompressed:
			{
				const aiTexture* pTexture = pScene->GetEmbeddedTexture(path.C_Str());
				Texture embeddedTexture(this->device,
					reinterpret_cast<uint8_t*>(pTexture->pcData),
					pTexture->mWidth,
					textureType);
				materialTextures.push_back(embeddedTexture);
				break;
			}
			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();
				Texture diskTexture(this->device, filename, textureType);
				materialTextures.push_back(diskTexture);
				break;
			}
			}

		}
	}

	if (materialTextures.size() == 0)
	{
		materialTextures.push_back(Texture(this->device, Colors::UnHandledTextureColor, aiTextureType::aiTextureType_DIFFUSE));
	}
	return materialTextures;
}


int Model::GetTextureIndex(aiString* pStr)
{
	assert(pStr->length >= 2);
	return atoi(&pStr->C_Str()[1]);
}