/**
 * @file   Mesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the mesh class.
 */

#include "Mesh.h"
#include <assimp/postprocess.h>
#include "core/ApplicationNode.h"
#include "SceneMeshNode.h"
#include "core/gfx/Material.h"

#undef max
#undef min

namespace viscom {

    /**
     * Constructor, creates a mesh from file.
     * @param meshFilename the filename of the mesh file.
     */
    Mesh::Mesh(const std::string& meshFilename, ApplicationNode* node) :
        Resource(meshFilename, node),
        indexBuffer_(0)
    {
        auto fullFilename = node->GetConfig().baseDirectory_ + resourceBasePath + meshFilename;
        // Load a Model from File
        Assimp::Importer loader;
        auto scene = loader.ReadFile(fullFilename,
            aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_JoinIdenticalVertices
            | aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_ImproveCacheLocality
            | aiProcess_RemoveRedundantMaterials | aiProcess_OptimizeGraph | aiProcess_FlipUVs
            | aiProcess_CalcTangentSpace );


        unsigned int maxUVChannels = 0, maxColorChannels = 0, numVertices = 0, numIndices = 0;
        std::vector<std::vector<unsigned int>> indices;
        indices.resize(scene->mNumMeshes);
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            maxUVChannels = glm::max(maxUVChannels, scene->mMeshes[i]->GetNumUVChannels());
            maxColorChannels = glm::max(maxColorChannels, scene->mMeshes[i]->GetNumUVChannels());
            numVertices += scene->mMeshes[i]->mNumVertices;
            for (unsigned int fi = 0; fi < scene->mMeshes[i]->mNumFaces; ++fi) {
                auto faceIndices = scene->mMeshes[i]->mFaces[fi].mNumIndices;
                // TODO: currently lines and points are ignored. [12/14/2016 Sebastian Maisch]
                if (faceIndices == 3) {
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[0]);
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[1]);
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[2]);
                    numIndices += faceIndices;
                }
            }
        }

        vertices_.resize(numVertices);
        normals_.resize(numVertices);
        texCoords_.resize(maxUVChannels);
        for (auto& texCoords : texCoords_) texCoords.resize(numVertices);
        tangents_.resize(numVertices);
        binormals_.resize(numVertices);
        colors_.resize(maxColorChannels);
        for (auto& colors : colors_) colors.resize(numVertices);
        indices_.resize(numIndices);
        materials_.resize(scene->mNumMaterials);

        for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
            auto material = scene->mMaterials[i];
            auto& mat = materials_[i];
            material->Get(AI_MATKEY_COLOR_AMBIENT, mat.ambient);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, mat.diffuse);
            material->Get(AI_MATKEY_COLOR_SPECULAR, mat.specular);
            material->Get(AI_MATKEY_OPACITY, mat.alpha);
            material->Get(AI_MATKEY_SHININESS, mat.specularExponent);
            material->Get(AI_MATKEY_REFRACTI, mat.refraction);
            aiString diffuseTexPath, bumpTexPath;
            if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexPath)) {
                mat.diffuseTex = loadTexture(diffuseTexPath.C_Str(), node);
            }

            if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), bumpTexPath)) {
                mat.bumpTex = loadTexture(bumpTexPath.C_Str(), node);
                material->Get(AI_MATKEY_TEXBLEND(aiTextureType_HEIGHT, 0), mat.bumpMultiplier);
            } else if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), bumpTexPath)) {
                mat.bumpTex = loadTexture(diffuseTexPath.C_Str(), node);
                material->Get(AI_MATKEY_TEXBLEND(aiTextureType_NORMALS, 0), mat.bumpMultiplier);
            }
        }

        unsigned int currentMeshIndexOffset = 0;
        unsigned int currentMeshVertexOffset = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            auto mesh = scene->mMeshes[i];

            if (mesh->HasPositions()) {
                std::copy(mesh->mVertices, &mesh->mVertices[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&vertices_[currentMeshVertexOffset]));
            }
            if (mesh->HasNormals()) {
                std::copy(mesh->mNormals, &mesh->mNormals[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&normals_[currentMeshVertexOffset]));
            }
            for (unsigned int ti = 0; ti < mesh->GetNumUVChannels(); ++ti) {
                std::copy(mesh->mTextureCoords[ti], &mesh->mTextureCoords[ti][mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&texCoords_[ti][currentMeshVertexOffset]));
            }
            if (mesh->HasTangentsAndBitangents()) {
                std::copy(mesh->mTangents, &mesh->mTangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&tangents_[currentMeshVertexOffset]));
                std::copy(mesh->mBitangents, &mesh->mBitangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&binormals_[currentMeshVertexOffset]));
            }
            for (unsigned int ci = 0; ci < mesh->GetNumColorChannels(); ++ci) {
                std::copy(mesh->mColors[ci], &mesh->mColors[ci][mesh->mNumVertices], reinterpret_cast<aiColor4D*>(&colors_[ci][currentMeshVertexOffset]));
            }
            if (mesh->GetNumUVChannels() > 0) {
                std::copy(mesh->mTextureCoords[1], &mesh->mTextureCoords[1][mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&texCoords_[currentMeshVertexOffset]));
            }

            std::transform(indices[i].begin(), indices[i].end(), &indices_[currentMeshIndexOffset], [currentMeshVertexOffset](unsigned int idx) { return idx + currentMeshVertexOffset; });

            auto& material = materials_[mesh->mMaterialIndex];

            subMeshes_.emplace_back(this, mesh->mName.C_Str(), currentMeshIndexOffset, static_cast<unsigned int>(indices[i].size()), &material);
            currentMeshVertexOffset += mesh->mNumVertices;
            currentMeshIndexOffset += static_cast<unsigned int>(indices[i].size());
        }

        rootNode_ = std::make_unique<SceneMeshNode>(scene->mRootNode, nullptr, subMeshes_);

        glGenBuffers(1, &indexBuffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /**
     *  Move constructor.
     */
    Mesh::Mesh(Mesh&& rhs) noexcept :
        Resource(std::move(rhs)),
        vertices_(std::move(rhs.vertices_)),
        normals_(std::move(rhs.normals_)),
        texCoords_(std::move(rhs.texCoords_)),
        indices_(std::move(rhs.indices_)),
        subMeshes_(std::move(rhs.subMeshes_)),
        indexBuffer_(rhs.indexBuffer_)
    {
        rhs.indexBuffer_ = 0;
    }

    /**
     *  Move assignment operator.
     */
    Mesh& Mesh::operator=(Mesh&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~Mesh();
            Resource* tRes = this;
            *tRes = static_cast<Resource&&>(std::move(rhs));
            vertices_ = std::move(rhs.vertices_);
            normals_ = std::move(rhs.normals_);
            texCoords_ = std::move(rhs.texCoords_);
            indices_ = std::move(rhs.indices_);
            subMeshes_ = std::move(rhs.subMeshes_);
            indexBuffer_ = rhs.indexBuffer_;
            rhs.indexBuffer_ = 0;
        }
        return *this;
    }

    /** Destructor. */
    Mesh::~Mesh() noexcept
    {
        if (indexBuffer_ != 0) glDeleteBuffers(1, &indexBuffer_);
        indexBuffer_ = 0;
    }


    Mesh::Mesh(const std::string& path, const aiMesh* mesh, const aiScene* scene) :
        subMeshes_(),
        indices_(),
        vertices_(),
        indexBuffer_(0)
    {
        // Create Vertex Data from Mesh Node
        MeshVertex vertex;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            if (mesh->mTextureCoords[0])
                vertex.textureCoordinate = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            vertex.position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vertices_.push_back(vertex);
        }

        // Create Mesh Indices for Indexed Drawing
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
                indices_.push_back(mesh->mFaces[i].mIndices[j]);

        // Bind a Vertex Array Object
        glGenVertexArrays(1, &vertexArray_);
        glBindVertexArray(vertexArray_);

        // Copy Vertex Buffer Data
        glGenBuffers(1, &vertexBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(MeshVertex), vertices_.data(), GL_STATIC_DRAW);

        // Copy Index Buffer Data
        glGenBuffers(1, &indexBuffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);

        // Set Shader Attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, position)));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, normal)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, textureCoordinate)));
        glEnableVertexAttribArray(0); // Vertex Positions
        glEnableVertexAttribArray(1); // Vertex Normals
        glEnableVertexAttribArray(2); // Vertex UVs

        glBindVertexArray(0);
    }

    std::shared_ptr<const Texture> Mesh::loadTexture(const std::string& relFilename, ApplicationNode* node) const
    {
        auto path = GetId().substr(0, GetId().find_last_of("/") + 1);
        auto texFilename = path + relFilename;
        return std::move(node->GetTextureManager().GetResource(texFilename));
    }
}
