/**
 * @file   Animation.h
 * @author Christian van Onzenoodt <christian.van-onzenoodt@uni-ulm.de>
 * @date   10.05.2017
 *
 * @brief  Data type for an animation.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/utils/serializationHelper.h"
#include <assimp/scene.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace viscom {

    /** Type alias to describe animation time values. */
    using Time = float;

    /**
     *  A channel representing one bone/ node etc.
     *  Holds positions, rotations and scaling of a bone/node etc. at a specific
     *  timestamp.
     */
    struct Channel
    {
        /** Nodes position at specific timestamps. */
        std::vector<std::pair<Time, glm::vec3>> positionFrames_;
        /** Nodes rotation at specific timestamps. */
        std::vector<std::pair<Time, glm::quat>> rotationFrames_;
        /** Nodes scale at specific timestamps. */
        std::vector<std::pair<Time, glm::vec3>> scalingFrames_;
    };

    /**
     *  Information about the animation.
     */
    struct AnimationInfo
    {
        AnimationInfo() = default;

        AnimationInfo(float start, float end, float playback = 1.0f)
            : startTime_{ start }, endTime_{ end }, playbackSpeed_{ playback }
        {
        }

        AnimationInfo(std::size_t animationIndex, float start, float end, float playback = 1.0f)
            : animationIndex_{animationIndex}, startTime_ { start }, endTime_{ end }, playbackSpeed_{ playback }
        {
        }

        /** Index of the animation. */
        std::size_t animationIndex_ = 0;
        /** Start time of the animation. */
        float startTime_ = 0.0f;
        /** End time of the animation. */
        float endTime_ = 1.0f;
        /** Playback speed of the animation. */
        float playbackSpeed_ = 1.0f;
    };

    /**
     *  Mapping between AnimationState and AnimationInfo.
     */
    using SubAnimationMapping = std::vector<AnimationInfo>;



    /** An animation for a model. */
    class Animation
    {
    public:
        Animation();
        Animation(aiAnimation* aiAnimation);

        void FlattenHierarchy(std::size_t numNodes, const std::map<std::string, std::size_t>& nodeNamesMap);

        /** Returns the number of ticks per second. */
        float GetFramesPerSecond() const;
        /** Returns the duration of the animation in seconds. */
        float GetDuration() const;

        Animation GetSubSequence(Time start, Time end) const;

        bool ComputePoseAtTime(std::size_t id, Time time, glm::mat4& pose) const;

        /**
         *  Writes the channels of the animation to a stream.
         *  @param ofs stream to write to.
         */
        void Write(std::ostream& ofs) const;
        /**
         *  Reads the channels of the animation from a stream.
         *  @param ifs stream to read from.
         */
        bool Read(std::istream& ifs);

    private:
        /** Defines the type of the VersionableSerializer for the animation class. */
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'A', 'N', 'M', 1001>;

        /** Holds the channels during loading. */
        std::map<std::string, Channel> channelMap_;
        /** Holds the channels (position, rotation, scaling) for each node. */
        std::vector<Channel> channels_;
        /** Ticks per second. */
        float framesPerSecond_ = 0.f;
        /** Duration of this animation. */
        float duration_ = 0.f;
    };

    inline float Animation::GetFramesPerSecond() const { return framesPerSecond_; }

    inline float Animation::GetDuration() const { return duration_; }

} // namespace get
