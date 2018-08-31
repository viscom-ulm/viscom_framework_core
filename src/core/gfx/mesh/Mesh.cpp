/**
 * @file   Mesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the mesh class.
 */

#include "Mesh.h"
#include "SceneMeshNode.h"
#include "assimp_convert_helpers.h"
#include "core/FrameworkInternal.h"
#include "core/gfx/Material.h"
#include "core/open_gl.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#ifndef __APPLE_CC__
#include <filesystem>
#include <fstream>
#endif

namespace viscom {

    inline glm::vec3 GetMaterialColor(aiMaterial* material, const char* pKey, unsigned int type, unsigned int idx) {
        aiColor3D c;
        material->Get(pKey, type, idx, c);
        return glm::vec3{ c.r, c.g, c.b };
    }

    constexpr unsigned int ASSIMP_FLAGS = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs;

    constexpr unsigned int ASSIMP_FLAGS_FORCEGEN = aiProcess_CalcTangentSpace | aiProcess_GenNormals
        | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_SplitLargeMeshes
        | aiProcess_GenUVCoords | aiProcess_SortByPType | aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_FindInstances
        | aiProcess_ValidateDataStructure | aiProcess_OptimizeMeshes | aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_ForceGenNormals
        | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs;

    /**
     * Constructor, creates a mesh from file.
     * @param meshFilename the filename of the mesh file.
     */
    Mesh::Mesh(const std::string& meshFilename, FrameworkInternal* node, bool synchronize) :
        Resource(meshFilename, ResourceType::Mesh, node, synchronize),
        filename_{ meshFilename },
        indexBuffer_(0)
    {
    }

    /** Destructor. */
    Mesh::~Mesh() noexcept
    {
        if (indexBuffer_ != 0) glDeleteBuffers(1, &indexBuffer_);
        indexBuffer_ = 0;
    }

    void Mesh::Initialize(bool forceGenNormals)
    {
        forceGenNormals_ = forceGenNormals;
        InitializeFinished();
    }

    void Mesh::Load(std::optional<std::vector<std::uint8_t>>& data)
    {
        auto filename = FindResourceLocation(GetId());
        auto binFilename = filename + ".viscombin";

        if (!Load(filename, binFilename, GetAppNode())) LoadAssimpMeshFromFile(filename, binFilename, GetAppNode());

        rootNode_->FlattenNodeTree(nodes_);

        glGenBuffers(1, &indexBuffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (data.has_value()) {
            data->clear();
            LOG(WARNING) << "Sending memory versions of meshes will most probably not work. Do not use this!!!";
            auto hint = filename.substr(filename.find_last_of(".") + 1);
            auto hintSize = hint.size() * sizeof(std::remove_reference_t<decltype(hint)>::value_type);
            std::ifstream meshFile(filename, std::ios::binary | std::ios::ate);
            std::size_t meshFileSize = meshFile.tellg();
            data->resize(sizeof(std::size_t) + hintSize + sizeof(bool) + meshFileSize);

            reinterpret_cast<std::size_t*>(data->data())[0] = hint.size();
            auto dataptr = data->data() + sizeof(std::size_t);
            memcpy(dataptr, hint.data(), hintSize);
            dataptr += hintSize;
            memcpy(dataptr, &forceGenNormals_, sizeof(bool));
            dataptr += sizeof(bool);

            meshFile.seekg(0);
            meshFile.read(reinterpret_cast<char*>(dataptr), meshFileSize);
        }
    }

    void Mesh::LoadFromMemory(const void* data, std::size_t size)
    {
        LOG(WARNING) << "Loading meshes from memory will most probably not work. Do not use this!!!";

        auto hintSize = reinterpret_cast<const std::size_t*>(data);
        std::string hint;
        hint.resize(hintSize[0]);
        memcpy(hint.data(), &hintSize[1], hintSize[0]);
        auto dataptr = reinterpret_cast<const std::uint8_t*>(data) + sizeof(std::size_t) + hintSize[0] * sizeof(char);
        memcpy(&forceGenNormals_, dataptr, sizeof(bool));
        dataptr += sizeof(bool);
        auto meshSize = size - (sizeof(std::size_t) + hintSize[0] * sizeof(char) + sizeof(bool));
        Assimp::Importer loader;
        auto scene = loader.ReadFileFromMemory(dataptr, meshSize, forceGenNormals_ ? ASSIMP_FLAGS_FORCEGEN : ASSIMP_FLAGS, hint.c_str());

        LoadAssimpMesh(scene, GetAppNode());
    }

    void Mesh::LoadAssimpMeshFromFile(const std::string& filename, const std::string& binFilename, FrameworkInternal* node)
    {
        auto fullFilename = FindResourceLocation(filename);
        // Load a Model from File
        Assimp::Importer loader;
        auto scene = loader.ReadFile(fullFilename, forceGenNormals_ ? ASSIMP_FLAGS_FORCEGEN : ASSIMP_FLAGS);

        LoadAssimpMesh(scene, node);
        Save(binFilename);
    }

    void Mesh::LoadAssimpMesh(const aiScene * scene, FrameworkInternal* node)
    {
        unsigned int maxUVChannels = 0, maxColorChannels = 0, numVertices = 0, numIndices = 0;
        std::vector<std::vector<unsigned int>> indices;
        indices.resize(static_cast<size_t>(scene->mNumMeshes));
        auto numMeshes = static_cast<std::size_t>(scene->mNumMeshes);
        for (std::size_t i = 0; i < numMeshes; ++i) {
            maxUVChannels = glm::max(maxUVChannels, scene->mMeshes[i]->GetNumUVChannels());
            maxColorChannels = glm::max(maxColorChannels, scene->mMeshes[i]->GetNumColorChannels());
            numVertices += scene->mMeshes[i]->mNumVertices; //-V127
            auto numFaces = static_cast<std::size_t>(scene->mMeshes[i]->mNumFaces);
            for (std::size_t fi = 0; fi < numFaces; ++fi) {
                auto faceIndices = scene->mMeshes[i]->mFaces[fi].mNumIndices;
                // TODO: currently lines and points are ignored. [12/14/2016 Sebastian Maisch]
                if (faceIndices == 3) {
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[0]);
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[1]);
                    indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[2]);
                    numIndices += faceIndices; //-V127
                }
            }
        }

        vertices_.resize(static_cast<std::size_t>(numVertices));
        normals_.resize(static_cast<std::size_t>(numVertices));
        texCoords_.resize(static_cast<std::size_t>(maxUVChannels));
        for (auto& texCoords : texCoords_) texCoords.resize(static_cast<std::size_t>(numVertices));
        tangents_.resize(static_cast<std::size_t>(numVertices));
        binormals_.resize(static_cast<std::size_t>(numVertices));
        colors_.resize(static_cast<std::size_t>(maxColorChannels));
        for (auto& colors : colors_) colors.resize(static_cast<std::size_t>(numVertices));
        indices_.resize(static_cast<std::size_t>(numIndices));
        materials_.resize(static_cast<std::size_t>(scene->mNumMaterials));
        materialTextures_.resize(static_cast<std::size_t>(scene->mNumMaterials));

        auto numMaterials = static_cast<std::size_t>(scene->mNumMaterials);
        for (std::size_t i = 0; i < numMaterials; ++i) {
            auto material = scene->mMaterials[i];
            auto& mat = materials_[i];
            auto& matTex = materialTextures_[i];
            mat.ambient = GetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT);
            mat.diffuse = GetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE);
            mat.specular = GetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR);
            material->Get(AI_MATKEY_OPACITY, mat.alpha);
            material->Get(AI_MATKEY_SHININESS, mat.specularExponent);
            material->Get(AI_MATKEY_REFRACTI, mat.refraction);
            aiString diffuseTexPath, bumpTexPath;
            if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexPath)) {
                matTex.diffuseTex = LoadTexture(diffuseTexPath.C_Str(), node);
            }

            if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), bumpTexPath)) {
                matTex.bumpTex = LoadTexture(bumpTexPath.C_Str(), node);
                material->Get(AI_MATKEY_TEXBLEND(aiTextureType_HEIGHT, 0), mat.bumpMultiplier);
            }
            else if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), bumpTexPath)) {
                matTex.bumpTex = LoadTexture(diffuseTexPath.C_Str(), node);
                material->Get(AI_MATKEY_TEXBLEND(aiTextureType_NORMALS, 0), mat.bumpMultiplier);
            }
        }

        unsigned int currentMeshIndexOffset = 0;
        std::size_t currentMeshVertexOffset = 0;

        // using Bone = unsigned int;

        std::map<std::string, unsigned int> bones;
        std::vector<std::vector<std::pair<unsigned int, float>>> boneWeights;
        boneWeights.resize(numVertices);

        for (std::size_t i = 0; i < numMeshes; ++i) {
            auto mesh = scene->mMeshes[i];

            if (mesh->HasPositions()) {
                std::copy(mesh->mVertices, &mesh->mVertices[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&vertices_[currentMeshVertexOffset])); //-V108
            }
            if (mesh->HasNormals()) {
                std::copy(mesh->mNormals, &mesh->mNormals[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&normals_[currentMeshVertexOffset])); //-V108
            }
            for (unsigned int ti = 0; ti < mesh->GetNumUVChannels(); ++ti) {
                std::copy(mesh->mTextureCoords[ti], &mesh->mTextureCoords[ti][mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&texCoords_[ti][currentMeshVertexOffset])); //-V108
            }
            if (mesh->HasTangentsAndBitangents()) {
                std::copy(mesh->mTangents, &mesh->mTangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&tangents_[currentMeshVertexOffset])); //-V108
                std::copy(mesh->mBitangents, &mesh->mBitangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&binormals_[currentMeshVertexOffset])); //-V108
            }
            for (unsigned int ci = 0; ci < mesh->GetNumColorChannels(); ++ci) {
                std::copy(mesh->mColors[ci], &mesh->mColors[ci][mesh->mNumVertices], reinterpret_cast<aiColor4D*>(&colors_[ci][currentMeshVertexOffset])); //-V108
            }

            if (mesh->HasBones()) {

                // Walk all bones of this mesh
                for (auto b = 0U; b < mesh->mNumBones; ++b) {
                    auto aiBone = mesh->mBones[b];

                    auto bone = bones.find(aiBone->mName.C_Str());

                    // We don't have this bone, yet -> insert into mesh datastructure
                    if (bone == bones.end()) {
                        bones[aiBone->mName.C_Str()] = static_cast<unsigned int>(inverseBindPoseMatrices_.size());

                        inverseBindPoseMatrices_.push_back(AiMatrixToGLM(aiBone->mOffsetMatrix));
                    }

                    unsigned int indexOfCurrentBone = bones[aiBone->mName.C_Str()];

                    for (auto w = 0U; w < aiBone->mNumWeights; ++w) {

                        boneWeights[currentMeshVertexOffset + aiBone->mWeights[w].mVertexId].emplace_back(
                            indexOfCurrentBone, aiBone->mWeights[w].mWeight);
                    }
                }
            }
            else {
                for (std::size_t i = 0; i < mesh->mNumVertices; ++i) {
                    boneWeights[currentMeshVertexOffset + i].emplace_back(std::make_pair(0, 0.0f));
                }
            }

            if (!indices[i].empty()) {
                std::transform(indices[i].begin(), indices[i].end(), &indices_[currentMeshIndexOffset],
                    [currentMeshVertexOffset](unsigned int idx) { return static_cast<unsigned int>(idx + currentMeshVertexOffset); }); //-V108
            }

            subMeshes_.emplace_back(this, mesh->mName.C_Str(), currentMeshIndexOffset, static_cast<unsigned int>(indices[i].size()), mesh->mMaterialIndex);
            currentMeshVertexOffset += mesh->mNumVertices; //-V127
            currentMeshIndexOffset += static_cast<unsigned int>(indices[i].size()); //-V127
        }

        // Loading animations
        if (scene->HasAnimations()) {
            for (auto a = 0U; a < scene->mNumAnimations; ++a) {
                animations_.emplace_back(scene->mAnimations[a], bones);
            }
        }

        // Parse parent information for each bone.
        boneParent_.resize(bones.size(), std::numeric_limits<std::size_t>::max());
        // Root node has a parent index of max value of size_t
        ParseBoneHierarchy(bones, scene->mRootNode, std::numeric_limits<std::size_t>::max(), glm::mat4(1.0f));

        // Iterate all weights for each vertex
        for (auto& weights : boneWeights) {
            // sort the weights.
            std::sort(weights.begin(), weights.end(),
                [](const std::pair<unsigned int, float>& left, const std::pair<unsigned int, float>& right) {
                return left.second > right.second;
            });

            // resize the weights, because we only take 4 bones per vertex into account.
            weights.resize(4);

            // build vec's with up to 4 components, one for each bone, influencing
            // the current vertex.
            glm::uvec4 newIndices;
            glm::vec4 newWeights;
            float sumWeights = 0.0f;
            for (auto i = 0U; i < weights.size(); ++i) {
                newIndices[i] = weights[i].first;
                newWeights[i] = weights[i].second;
                sumWeights += newWeights[i];
            }

            boneOffsetMatrixIndices_.push_back(newIndices);
            // normalize the bone weights.
            boneWeights_.push_back(newWeights / glm::max(sumWeights, 0.000000001f));
        }

        rootNode_ = std::make_unique<SceneMeshNode>(scene->mRootNode, nullptr, bones);

        rootNode_->GenerateBoundingBoxes(*this);

        GenerateBoneBoundingBoxes();

        globalInverse_ = glm::inverse(rootNode_->GetLocalTransform());
    }

    std::shared_ptr<const Texture> Mesh::LoadTexture(const std::string& relFilename, FrameworkInternal* node) const
    {
#ifndef __APPLE_CC__
        namespace fs = std::filesystem;
        auto path =  fs::path(filename_).parent_path().string() + '/';
#else
        auto path = filename_.substr(0, filename_.find_last_of('/') + 1);
#endif
        std::shared_ptr<const Texture> texture;
        try {
            auto texFilename = path + relFilename;
            texture = std::move(node->GetTextureManager().GetResource(texFilename));
        }
        catch (resource_loading_error&) {
#ifndef __APPLE_CC__
            namespace fs = std::filesystem;
            auto textureFilename = fs::path(filename_).filename().string();
#else
            auto textureFilename = relFilename.substr(relFilename.find_last_of("/") + 1);
#endif
            auto texFilename = path + textureFilename;

            texture = std::move(node->GetTextureManager().GetResource(texFilename));
        }

        glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        return std::move(texture);
    }

    void Mesh::Save(const std::string& filename) const
    {
#ifndef __APPLE_CC__
        std::ofstream ofs(filename, std::ios::out | std::ios::binary);
        VersionableSerializerType::writeHeader(ofs);
        Write(ofs);
#endif
    }

    void Mesh::Write(std::ostream& ofs) const
    {
        serializeHelper::writeV(ofs, vertices_);
        serializeHelper::writeV(ofs, normals_);
        serializeHelper::writeVV(ofs, texCoords_);
        serializeHelper::writeV(ofs, tangents_);
        serializeHelper::writeV(ofs, binormals_);
        serializeHelper::writeVV(ofs, colors_);
        serializeHelper::writeV(ofs, boneOffsetMatrixIndices_);
        serializeHelper::writeV(ofs, boneWeights_);
        serializeHelper::writeVV(ofs, indexVectors_);
        serializeHelper::writeV(ofs, inverseBindPoseMatrices_);
        serializeHelper::writeV(ofs, boneParent_);

        serializeHelper::writeV(ofs, indices_);

        serializeHelper::write(ofs, globalInverse_);
        serializeHelper::writeV(ofs, boneBoundingBoxes_);

        serializeHelper::writeV(ofs, materials_);

        for (const auto& matTex : materialTextures_) {
            if (matTex.diffuseTex) serializeHelper::write(ofs, matTex.diffuseTex->GetId());
            else serializeHelper::write(ofs, std::string());

            if (matTex.bumpTex) serializeHelper::write(ofs, matTex.bumpTex->GetId());
            else serializeHelper::write(ofs, std::string());
        }

        serializeHelper::write(ofs, subMeshes_.size());
        for (const auto& mesh : subMeshes_) mesh.Write(ofs);

        serializeHelper::write(ofs, animations_.size());
        for (const auto& animation : animations_) animation.Write(ofs);

        rootNode_->Write(ofs);
    }

    bool Mesh::Load(const std::string& filename, const std::string& binFilename, FrameworkInternal* node)
    {
#ifndef __APPLE_CC__
        if (std::experimental::filesystem::exists(binFilename)) {
            if (!VersionableSerializerType::checkFileDate(filename, binFilename)) return false;

            std::ifstream inBinFile(binFilename, std::ios::binary);
            if (inBinFile.is_open()) {
                bool correctHeader;
                unsigned int actualVersion;
                std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(inBinFile);
                if (correctHeader) return Read(inBinFile, node);
            }
        }
#endif
        return false;
    }

    bool Mesh::Read(std::istream& ifs, FrameworkInternal* node)
    {
        serializeHelper::readV(ifs, vertices_);
        serializeHelper::readV(ifs, normals_);
        serializeHelper::readVV(ifs, texCoords_);
        serializeHelper::readV(ifs, tangents_);
        serializeHelper::readV(ifs, binormals_);
        serializeHelper::readVV(ifs, colors_);
        serializeHelper::readV(ifs, boneOffsetMatrixIndices_);
        serializeHelper::readV(ifs, boneWeights_);
        serializeHelper::readVV(ifs, indexVectors_);
        serializeHelper::readV(ifs, inverseBindPoseMatrices_);
        serializeHelper::readV(ifs, boneParent_);

        serializeHelper::readV(ifs, indices_);

        serializeHelper::read(ifs, globalInverse_);
        serializeHelper::readV(ifs, boneBoundingBoxes_);

        serializeHelper::readV(ifs, materials_);

        materialTextures_.resize(materials_.size());
        for (std::size_t i = 0; i < materials_.size(); ++i) {
            auto& matTex = materialTextures_[i];

            std::string diffuseTexId, bumpTexId;
            serializeHelper::read(ifs, diffuseTexId);
            serializeHelper::read(ifs, bumpTexId);
            if (!diffuseTexId.empty()) matTex.diffuseTex = LoadTexture(diffuseTexId, node);
            if (!bumpTexId.empty()) matTex.bumpTex = LoadTexture(bumpTexId, node);
        }

        std::size_t numMeshes;
        serializeHelper::read(ifs, numMeshes);
        subMeshes_.resize(numMeshes);
        for (auto& mesh : subMeshes_) {
            if (!mesh.Read(ifs)) return false;
        }

        std::size_t numAnimations;
        serializeHelper::read(ifs, numAnimations);
        animations_.resize(numAnimations);
        for (auto& animation : animations_) {
            if (!animation.Read(ifs)) return false;
        }

        std::unordered_map<std::uint64_t, SceneMeshNode*> nodeMap;
        rootNode_ = std::make_unique<SceneMeshNode>();
        nodeMap[0] = nullptr;
        if (!rootNode_->Read(ifs, nodeMap)) return false;

        return true;
    }

    ///
    /// This function walks the hierarchy of bones and does two things:
    /// - set the parent of each bone into `boneParent_`
    /// - update the boneOffsetMatrices_, so each matrix also includes the
    ///   transformations of the child bones.
    ///
    /// \param map from name of bone to index in boneOffsetMatrices_
    /// \param current node in
    /// \param index of the parent in boneOffsetMatrices_
    /// \param Matrix including all transformations from the parents of the
    ///        current node.
    ///
    void Mesh::ParseBoneHierarchy(const std::map<std::string, unsigned int>& bones, const aiNode* node,
        std::size_t parent, glm::mat4 parentMatrix)
    {
        auto bone = bones.find(node->mName.C_Str());
        if (bone != bones.end()) {
            // This node is a bone. Set the parent for this node to the current parent
            // node.
            boneParent_[bone->second] = parent;
            // Set the new parent
            parent = bone->second;
        }

        for (auto i = 0U; i < node->mNumChildren; ++i) {
            ParseBoneHierarchy(bones, node->mChildren[i], parent, parentMatrix);
        }
    }

    ///
    /// Generate all BoundingBoxes for the bones.
    ///
    void Mesh::GenerateBoneBoundingBoxes()
    {
        if (inverseBindPoseMatrices_.empty()) {
            return;
        }

        boneBoundingBoxes_.resize(inverseBindPoseMatrices_.size());

        bool hasVertexWithoutBone = false;

        for (auto i = 0U; i < boneOffsetMatrixIndices_.size(); i++) {

            bool vertexHasBone = false;

            for (auto b = 0; b < 4; b++) {
                auto boneI = boneOffsetMatrixIndices_[i][b];
                auto boneW = boneWeights_[i][b];
                if (boneW > 0) {
                    vertexHasBone = true;
                    math::AABB3<float>& box = boneBoundingBoxes_[boneI];
                    box.AddPoint(vertices_[i]);
                }
            }
            if (!vertexHasBone) {
                hasVertexWithoutBone = true;
            }
        }

        if (hasVertexWithoutBone) {
            std::cout << "[Warning] You are using a model where not all vertices in the "
                "model are associated with a bone! This can lead to "
                "errors in the collision detection!"
                << "\n"
                << "Model-path: " << filename_ << std::endl;
        }
    }
}
