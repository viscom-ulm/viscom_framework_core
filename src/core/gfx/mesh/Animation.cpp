/**
 * @file   Animation.cpp
 * @author Christian van Onzenoodt <christian.van-onzenoodt@uni-ulm.de>
 * @date   10.05.2017
 *
 * @brief  Data type for an animation.
 */

#include "Animation.h"

#include <cassert>
#include <glm/glm.hpp>
#include <limits>
#include <utility>

#include "animation_convert_helpers.h"

#include "core/main.h"

namespace viscom {

    /** Default constructor for animations. */
    Animation::Animation() = default;

    /**
     *  Constructor for animations.
     * 
     *  Takes an assimp animation and converts the data into our data-structure.
     * 
     *  @param aiAnimation Assimp animation
     */
    Animation::Animation(aiAnimation* aiAnimation) :
        name_{ aiAnimation->mName.C_Str() },
        framesPerSecond_{aiAnimation->mTicksPerSecond > 0.0 ? static_cast<float>(aiAnimation->mTicksPerSecond)
                                                              : 24.0f},
        duration_{static_cast<float>(aiAnimation->mDuration)}
    {
        for (auto c = 0U; c < aiAnimation->mNumChannels; ++c) {
            const auto aiChannel = aiAnimation->mChannels[c];

            auto channel = Channel();

            // Copy position data for this channel
            for (auto p = 0U; p < aiChannel->mNumPositionKeys; ++p) {
                channel.positionFrames_.emplace_back(static_cast<Time>(aiChannel->mPositionKeys[p].mTime),
                                   *reinterpret_cast<glm::vec3*>(&aiChannel->mPositionKeys[p].mValue));
            }

            // Copy rotation data for this channel
            for (auto r = 0U; r < aiChannel->mNumRotationKeys; ++r) {
                const auto& aiQuat = aiChannel->mRotationKeys[r].mValue;

                channel.rotationFrames_.emplace_back(static_cast<Time>(aiChannel->mRotationKeys[r].mTime),
                                                                 glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z));
            }

            // Copy scaling data for this channel
            for (auto s = 0U; s < aiChannel->mNumScalingKeys; ++s) {
                channel.scalingFrames_.emplace_back(static_cast<Time>(aiChannel->mScalingKeys[s].mTime),
                                   *reinterpret_cast<glm::vec3*>(&aiChannel->mScalingKeys[s].mValue));
            }

            channelMap_[aiChannel->mNodeName.C_Str()] = channel;
        }
    }

    /**
     *  Flattens the node hierarchy for animation channels.
     *  @param numNodes the number of nodes in the mesh.
     *  @param nodeNamesMap the mapping from node names to node indices.
     */
    void Animation::FlattenHierarchy(std::size_t numNodes, const std::map<std::string, std::size_t>& nodeNamesMap)
    {
        channels_.resize(numNodes);

        for (const auto& node : nodeNamesMap) {
            auto nodeChannelIndex = channelMap_.find(node.first);
            if (nodeChannelIndex != channelMap_.end()) {
                channels_[node.second] = channelMap_[node.first];
            }
        }

        channelMap_.clear();
    }

    /**
     *  Returns a sub-sequence of this animation.
     * 
     *  Timestamps of the new animations are normalized in the following way:
     *  \verbatim
     *           Original animation
     *                 v
     *  x                                     y
     *  |-------------------------------------|
     * 
     *       |-------|
     *       a       b
     *          ^
     *    Desired subsequence
     * 
     * 
     *  New (sub-)animation:
     * 
     *  0     (b-a)
     *  |-------|
     *  \endverbatim
     *  @param start Start-time of the sub-sequence
     *  @param end End-time of the sub-sequence
     * 
     *  @return New animation
     */
    Animation Animation::GetSubSequence(const std::string& name, Time start, Time end) const
    {
        assert(start < end && "Start time must be less then stop time");

        Animation subSequence;
        subSequence.name_ = name;
        subSequence.framesPerSecond_ = framesPerSecond_;
        subSequence.duration_ = end - start;

        // Copy data from the sequence and ensure there is a keyframe at start and
        // end timestamp
        for (const auto& channel : channels_) {

            auto newChannel = Channel();
            // copy positions
            newChannel.positionFrames_ = CopyFrameData(channel.positionFrames_, start, end);
            // copy rotations
            newChannel.rotationFrames_ = CopyFrameData(channel.rotationFrames_, start, end);
            // copy scaling
            newChannel.scalingFrames_ = CopyFrameData(channel.scalingFrames_, start, end);

            // insert the channel into the new animation
            // subSequence.channels_[keyId] = newChannel;
            subSequence.channels_.emplace_back(newChannel);
        }

        return subSequence;
    }

    /**
     *  Computes the transformation of a given bone/node, at a given time.
     * 
     *  @param id Index of the bone/node
     *  @param time Desired time
     *  @param pose Transform of this bone/node.
     *
     *  @return true if there is an animation.
     */
    bool Animation::ComputePoseAtTime(std::size_t id, Time time, glm::mat4& pose) const
    {
        time = glm::clamp(time, 0.0f, duration_);

        const auto& channel = channels_.at(id);

        const auto& positionFrames = channel.positionFrames_;
        const auto& rotationFrames = channel.rotationFrames_;
        const auto& scalingFrames = channel.scalingFrames_;

        glm::quat rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        glm::vec3 translation{ 0.0f };
        glm::vec3 scale{ 1.0f };

        if (positionFrames.empty() || rotationFrames.empty() || scalingFrames.empty()) return false;

        // There is just one frame
        if (positionFrames.size() == 1) {
            translation = positionFrames[0].second;
        }

        if (rotationFrames.size() == 1) {
            rotation = rotationFrames[0].second;
        }

        if (scalingFrames.size() == 1) {
            scale = scalingFrames[0].second;
        }

        // There is more than one frame -> interpolate
        if (positionFrames.size() > 1) {
            auto frameIndex = FindFrameAtTimeStamp(positionFrames, time);
            auto nextFrameIndex = (frameIndex + 1) % positionFrames.size();

            translation = InterpolateFrames(positionFrames[frameIndex], positionFrames[nextFrameIndex], time).second;
        }

        if (rotationFrames.size() > 1) {
            auto frameIndex = FindFrameAtTimeStamp(rotationFrames, time);
            auto nextFrameIndex = (frameIndex + 1) % rotationFrames.size();

            rotation = InterpolateFrames(rotationFrames[frameIndex], rotationFrames[nextFrameIndex], time).second;
        }

        if (scalingFrames.size() > 1) {
            auto frameIndex = FindFrameAtTimeStamp(scalingFrames, time);
            auto nextFrameIndex = (frameIndex + 1) % scalingFrames.size();

            scale = InterpolateFrames(scalingFrames[frameIndex], scalingFrames[nextFrameIndex], time).second;
        }


        pose = glm::mat4_cast(rotation);
        pose[0] *= scale.x;
        pose[1] *= scale.y;
        pose[2] *= scale.z;
        pose[3] = glm::vec4(translation, 1);
        return true;
    }

    void Animation::Write(std::ostream& ofs) const
    {
        VersionableSerializerType::writeHeader(ofs);
        serializeHelper::write(ofs, name_);
        serializeHelper::write(ofs, channelMap_.size());
        for (const auto& channel : channelMap_) {
            serializeHelper::write(ofs, channel.first);
            serializeHelper::writeV(ofs, channel.second.positionFrames_);
            serializeHelper::writeV(ofs, channel.second.rotationFrames_);
            serializeHelper::writeV(ofs, channel.second.scalingFrames_);
        }
        serializeHelper::write(ofs, framesPerSecond_);
        serializeHelper::write(ofs, duration_);
    }

    bool Animation::Read(std::istream& ifs)
    {
        bool correctHeader;
        unsigned int actualVersion;
        std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(ifs);
        if (correctHeader || actualVersion > 1000) {
            if (actualVersion >= 1002) {
                serializeHelper::read(ifs, name_);
            }
            std::size_t numChannels;
            serializeHelper::read(ifs, numChannels);
            channelMap_.clear();
            for (std::size_t i = 0; i < numChannels; ++i) {
                std::string channelName;
                serializeHelper::read(ifs, channelName);
                serializeHelper::readV(ifs, channelMap_[channelName].positionFrames_);
                serializeHelper::readV(ifs, channelMap_[channelName].rotationFrames_);
                serializeHelper::readV(ifs, channelMap_[channelName].scalingFrames_);
            }
            serializeHelper::read(ifs, framesPerSecond_);
            serializeHelper::read(ifs, duration_);
            return true;
        }
        return false;
    }

} // namespace viscom
