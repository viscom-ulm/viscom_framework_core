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

    using Time = float;

    /**
     *  A channel representing one bone/ node etc.
     *  Holds positions, rotations and scaling of a bone/node etc. at a specific
     *  timestamp.
     */
    struct Channel
    {
        std::vector<std::pair<Time, glm::vec3>> positionFrames_;
        std::vector<std::pair<Time, glm::quat>> rotationFrames_;
        std::vector<std::pair<Time, glm::vec3>> scalingFrames_;
    };

    /** An animation for a model. */
    class Animation
    {
    public:
        Animation();
        Animation(aiAnimation* aiAnimation, const std::map<std::string, unsigned int>& boneNameToOffset);

        float GetFramesPerSecond() const;
        float GetDuration() const;
        const std::vector<Channel>& GetChannels() const;
        const Channel& GetChannel(std::size_t id) const;

        Animation GetSubSequence(Time start, Time end) const;

        glm::mat4 ComputePoseAtTime(std::size_t id, Time time) const;

        void Write(std::ostream& ofs) const;
        bool Read(std::istream& ifs);

    private:
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'A', 'N', 'M', 1000>;

        /// Holds the channels (position, rotation, scaling) for each bone.
        std::vector<Channel> channels_;
        /// Ticks per second.
        float framesPerSecond_ = 0.f;
        /// Duration of this animation.
        float duration_ = 0.f;
    };

    inline float Animation::GetFramesPerSecond() const { return framesPerSecond_; }

    inline float Animation::GetDuration() const { return duration_; }

    inline const std::vector<Channel>& Animation::GetChannels() const { return channels_; }

    inline const Channel& Animation::GetChannel(std::size_t id) const { return channels_.at(id); }

} // namespace get
