/**
 * @file   AnimationState.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.03.26
 *
 * @brief  Definition of a class containing the state of an animated object.
 */

#include "AnimationState.h"
#include "Mesh.h"
#include "SceneMeshNode.h"

namespace viscom {

    /**
     *  Constructor of AnimationState.
     *  @param mesh the mesh containing animation data.
     *  @param mappings the animation mapping to use.
     *  @param startingAnimationIndex the starting index of the animation.
     *  @param isRepeating whether the animation should be repeating.
     */
    AnimationState::AnimationState(const Mesh* mesh, const SubAnimationMapping& mappings, std::size_t startingAnimationIndex, bool isRepeating) :
        mesh_{ mesh },
        animationIndex_{ startingAnimationIndex },
        isRepeating_{ isRepeating }
    {
        assert(mappings.size() > animationIndex_ && "there is no mapping for the starting-state");

        skinned_.resize(mesh_->GetNumberOfBones());

        for (const auto& mapping : mappings) {
            animations_.emplace_back(mesh_->GetAnimation(mapping.animationIndex_)->GetSubSequence(mapping.startTime_, mapping.endTime_));
            animationPlaybackSpeed_.emplace_back(mapping.playbackSpeed_);
        }

        localBonePoses_.resize(mesh_->GetNodes().size());
        globalBonePoses_.resize(mesh_->GetNodes().size());
        for (auto i = 0U; i < localBonePoses_.size(); ++i)
            localBonePoses_[i] = mesh_->GetNodes()[i]->GetLocalTransform();

    }

    /**
     *  Starts animation playback (again).
     *  @param currentTime the current timestamp.
     */
    void AnimationState::Play(double currentTime)
    {
        isPlaying_ = true;
        if (pauseTime_ != 0.0f) startTime_ += static_cast<float>(currentTime) - pauseTime_;
        else startTime_ = static_cast<float>(currentTime);
        pauseTime_ = 0.0f;
    }

    /**
     *  Updates the animation state.
     *  @param currentTime the current timestamp.
     */
    bool AnimationState::UpdateTime(double currentTime)
    {
        if (!isPlaying_) return false;

        // Advance time
        currentPlayTime_ = (static_cast<float>(currentTime) - startTime_) * GetFramesPerSecond() * GetSpeed();

        bool didAnimationStopOrRepeat = false;
        // Modulate time to fit within the interval of the animation
        if (currentPlayTime_ < 0.0f) {
            if (IsRepeating()) {
                auto time = currentPlayTime_ / GetDuration();
                currentPlayTime_ = (time - glm::floor(time)) * GetDuration();
            }
            else {
                currentPlayTime_ = 0.0f;
                Pause(currentTime);
            }
            didAnimationStopOrRepeat = true;
        }
        else if (currentPlayTime_ > GetDuration()) {

            if (IsRepeating()) {
                currentPlayTime_ = glm::mod(currentPlayTime_, GetDuration());
            }
            else {
                currentPlayTime_ = GetDuration();
                Pause(currentTime);
            }
            didAnimationStopOrRepeat = true;
        }
        return didAnimationStopOrRepeat;
    }

    /**
     *  Computes the final bone poses for an animation.
     */
    void AnimationState::ComputeAnimationsFinalBonePoses()
    {
        const auto& currentAnimation = animations_[animationIndex_];
        const auto& invBindPoseMatrices = mesh_->GetInverseBindPoseMatrices();

        for (auto i = 0U; i < mesh_->GetNodes().size(); ++i) {
            glm::mat4 pose;
            if (currentAnimation.ComputePoseAtTime(i, currentPlayTime_, pose)) localBonePoses_[i] = pose;
        }

        ComputeGlobalBonePose(mesh_->GetRootNode());

        for (const auto& node : mesh_->GetNodes()) {
            if (node->GetBoneIndex() == -1) {
                continue;
            }
            skinned_[node->GetBoneIndex()] = globalBonePoses_[node->GetNodeIndex()] * invBindPoseMatrices[node->GetBoneIndex()];
        }
    }

    void AnimationState::ComputeGlobalBonePose(const SceneMeshNode* node)
    {
        auto nodeParent = node->GetParent();
        while (nodeParent && node->GetBoneIndex() != -1 && (nodeParent->GetName().empty())) nodeParent = nodeParent->GetParent();

        if (!nodeParent) globalBonePoses_[node->GetNodeIndex()] = localBonePoses_[node->GetNodeIndex()];
        else globalBonePoses_[node->GetNodeIndex()] = globalBonePoses_[nodeParent->GetNodeIndex()] * localBonePoses_[node->GetNodeIndex()];

        for (auto i = 0U; i < node->GetNumberOfNodes(); ++i) {
            ComputeGlobalBonePose(node->GetChild(i));
        }
    }
}
