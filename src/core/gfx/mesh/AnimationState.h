/**
 * @file   AnimationState.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.03.26
 *
 * @brief  Definition of a class containing the state of an animated object.
 */

#pragma once

#include <vector>
#include <glm/mat4x4.hpp>
#include "Animation.h"

namespace viscom {

    class Mesh;
    class SceneMeshNode;

    /**
     *  Contains the current animation state of an animated object.
     */
    class AnimationState final
    {
    public:
        AnimationState(const Mesh* mesh, const SubAnimationMapping& mappings, std::size_t startingAnimationIndex = 0, bool isRepeating = true);

        void Play(double currentTime);
        /**
         *  Pauses the animation.
         *  @param currentTime the current timestamp.
         */
        void Pause(double currentTime) { isPlaying_ = false; pauseTime_ = static_cast<float>(currentTime); }
        /** Stops the animation. */
        void Stop() { isPlaying_ = false; currentPlayTime_ = 0.0f; pauseTime_ = 0.0f; }

        /** Accessor for animation speed. */
        float GetSpeed() const { return animationPlaybackSpeed_.at(animationIndex_); }
        /** Accessor for animation frames per second. */
        float GetFramesPerSecond() const { return animations_.at(animationIndex_).GetFramesPerSecond(); }
        /** Accessor for animation duration. */
        float GetDuration() const { return animations_.at(animationIndex_).GetDuration(); }
        /** Accessor for animation time. */
        float GetTime() const { return currentPlayTime_; }

        /** Checks whether the animation is playing. */
        bool IsPlaying() const { return isPlaying_; }
        /** Checks whether the animation is repeating. */
        bool IsRepeating() const { return isRepeating_; }

        /** Updates the current time. */
        bool UpdateTime(double currentTime);
        /** Sets the current play time (internal animation time). */
        void SetCurrentPlayTime(float currentPlayTime) { currentPlayTime_ = currentPlayTime; }
        /** Sets the current relative frame (internal animation time relative to the duration). */
        void SetCurrentFrameRelative(float relativeFrame) { currentPlayTime_ = relativeFrame * GetDuration(); }
        void ComputeAnimationsFinalBonePoses();

        /**
         *  Sets the current animation speed.
         *  @param speed the new animation speed.
         */
        void SetSpeed(float speed) { animationPlaybackSpeed_[animationIndex_] = speed; }
        /**
         *  Sets whether the animation should be repeated.
         *  @param repeat whether the animation should be repeated.
         */
        void SetRepeat(bool repeat) { isRepeating_ = repeat; }

        /**
         *  Returns the current animation index.
         *  @return the current animation index.
         */
        std::size_t GetCurrentAnimationIndex() const { return animationIndex_; }
        /**
         *  Sets the current animation index.
         *  @param animationIndex the new current animation index.
         */
        void SetCurrentAnimationIndex(std::size_t animationIndex) { animationIndex_ = animationIndex; }

        /** Returns the global bone pose for a node. */
        const glm::mat4& GetGlobalBonePose(std::size_t index) const { return globalBonePoses_[index]; }
        /** Returns the local bone pose for a node. */
        const glm::mat4& GetLocalBonePose(std::size_t index) const { return localBonePoses_[index]; }
        /** Returns the skinning matrices. */
        const std::vector<glm::mat4>& GetSkinningMatrices() const { return skinned_; }

    private:
        void ComputeGlobalBonePose(const SceneMeshNode* node);

        /** Holds the mesh to render. */
        const Mesh* mesh_;
        /** The current animation id. **/
        std::size_t animationIndex_;

        /** Holds the playback speed for each AnimationState. */
        std::vector<float> animationPlaybackSpeed_;
        /** Holds the actual animation for each AnimationState. */
        std::vector<Animation> animations_;


        /** Is the animation playing. */
        bool isPlaying_ = false;
        /** Is the animation repeating. */
        bool isRepeating_ = true;

        /** The starting time of the animation. */
        float startTime_ = 0.0f;
        /** The starting time of the current pause of the animation. */
        float pauseTime_ = 0.0f;
        /** The starting playback time of the animation. */
        float currentPlayTime_ = 0.0f;

        /** The local bone poses. */
        std::vector<glm::mat4> localBonePoses_;
        /** The global bone poses. */
        std::vector<glm::mat4> globalBonePoses_;
        /** The skinning matrices used for rendering. */
        std::vector<glm::mat4> skinned_;
    };

}
