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

    /** An animation for a model. */
    class Animation
    {
    public:
        Animation();
        Animation(aiAnimation* aiAnimation, const std::map<std::string, unsigned int>& boneNameToOffset);

        /** Returns the number of ticks per second. */
        float GetFramesPerSecond() const;
        /** Returns the duration of the animation in seconds. */
        float GetDuration() const;
        /** Returns the channels for each bone. */
        const std::vector<Channel>& GetChannels() const;
        /**
         *  Returns the channel of one bone.
         *  @param id id of the bone.
         */
        const Channel& GetChannel(std::size_t id) const;

        Animation GetSubSequence(Time start, Time end) const;

        glm::mat4 ComputePoseAtTime(std::size_t id, Time time) const;

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
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'A', 'N', 'M', 1000>;

        /** Holds the channels (position, rotation, scaling) for each bone. */
        std::vector<Channel> channels_;
        /** Ticks per second. */
        float framesPerSecond_ = 0.f;
        /** Duration of this animation. */
        float duration_ = 0.f;
    };

    inline float Animation::GetFramesPerSecond() const { return framesPerSecond_; }

    inline float Animation::GetDuration() const { return duration_; }

    inline const std::vector<Channel>& Animation::GetChannels() const { return channels_; }

    inline const Channel& Animation::GetChannel(std::size_t id) const { return channels_.at(id); }

} // namespace get
