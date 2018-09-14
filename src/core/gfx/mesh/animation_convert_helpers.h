/**
 *  @file   animation_convert_helpers.h
 *  @author Christian van Onzenoodt
 *  @date   13.06.2017
 *  @brief  Helper function for converting animations.
 */
#pragma once

#include <cstddef>
#include <vector>

#include <glm/gtc/quaternion.hpp>

namespace viscom {

    /**
     *  Find the frame at a given timestamp. If there is no frame at this
     *  timestamp, this method returns the frame just before the given time.
     * 
     *  @param frames frames to search
     *  @param time time to search
     *  @param maxSearch maximum number of frames to search
     * 
     *  @return the frame at (or just before) the given timestamp
     */
    template<typename Time, typename Transform>
    std::size_t FindFrameAtTimeStamp(const std::vector<std::pair<Time, Transform>>& frames, Time time,
                                     std::size_t maxSearch)
    {
        if (time < 0.0) {
            return 0;
        }

        for (std::size_t f = 0U; f < maxSearch; ++f) {
            if (frames[f].first > time) {
                if (f == 0) return 0;
                return f - 1;
            }
        }

        return maxSearch - 1;
    }

    /**
     *  Find the frame at a given timestamp. If there is no frame at this
     *  timestamp, this method returns the frame just before the given time.
     * 
     *  @param frames frames to search
     *  @param time time to search
     * 
     *  @return the frame at (or just before) the given timestamp
     */
    template<typename Time, typename Transform>
    std::size_t FindFrameAtTimeStamp(const std::vector<std::pair<Time, Transform>>& frames, Time time)
    {
        return FindFrameAtTimeStamp(frames, time, frames.size());
    }

    /**
     *  Interpolate between two given frames. The time needs to be between the
     *  timestamps of the two given frames.
     * 
     *  @param f1 First frame
     *  @param f2 Second frame
     *  @param t Time between the two frames
     * 
     *  @return interpolated frame
     */
    template<typename Time, typename T>
    std::pair<Time, T> InterpolateFrames(std::pair<Time, T> f1, std::pair<Time, T> f2, Time t)
    {
        auto normalizedTime = (t - f1.first) / (f2.first - f1.first);
        return std::make_pair(t, glm::mix(f1.second, f2.second, normalizedTime));
    }

    /**
     *  Interpolate between two given frames. The time needs to be between the
     *  timestamps of the two given frames.
     *  This is a specialized function for interpolating frames, holding
     *  rotations as glm::quat.
     * 
     *  @param f1 first frame
     *  @param f2 second frame
     *  @param t time between the two frames
     * 
     *  @return interpolated frame
     */
    template<typename Time>
    std::pair<Time, glm::quat> InterpolateFrames(std::pair<Time, glm::quat> f1, std::pair<Time, glm::quat> f2, Time t)
    {
        auto normalizedTime = glm::abs((t - f1.first) / (f2.first - f1.first));
        return std::make_pair(t, glm::slerp(f1.second, f2.second, normalizedTime));
    }

    /**
     *  Copies all frames from a vector starting at start, and ending at end and
     *  returns the result. End-time need to be _after_ start-time. If there is
     *  no frame after the end timestamp, we take the first frame for
     *  interpolation.
     * 
     *  @param orig Vector containing pairs of Timestamp and transform (position,
     *  rotation, scaling).
     *  @param start Starting time of the sub-animation.
     *  @param end End time of the sub-animation.
     * 
     *  @return Vector containing all frames between start and end, as well as a
     *  new generated (interpolated) frames at start and end.
     */
    template<typename Time, typename Transform>
    std::vector<std::pair<Time, Transform>> CopyFrameData(const std::vector<std::pair<Time, Transform>>& orig,
                                                          Time start, Time end)
    {
        auto newChannelData = std::vector<std::pair<Time, Transform>>();
        if (orig.size() == 0) {
            return newChannelData;
        }

        if (orig.size() == 1) {
            newChannelData.emplace_back(0.0f, orig[0].second);
            return newChannelData;
        }

        // find the two frames before and after the start
        auto startFrameIndex = FindFrameAtTimeStamp(orig, start);
        auto startNextFrameIndex = (startFrameIndex + 1) % orig.size();

        auto interpolatedStartFrame = InterpolateFrames(orig[startFrameIndex], orig[startNextFrameIndex], start);

        // find the two frames before and after the end
        auto endFrameIndex = FindFrameAtTimeStamp(orig, end);
        auto endNextFrameIndex = (endFrameIndex + 1) % orig.size();

        auto interpolatedEndFrame = InterpolateFrames(orig[endFrameIndex], orig[endNextFrameIndex], end);

        // push the new frames into the resulting vector
        newChannelData.emplace_back(interpolatedStartFrame.first - start, interpolatedStartFrame.second);

        for (auto f = startNextFrameIndex; f < endFrameIndex; ++f) {
            newChannelData.emplace_back(orig[f].first - start, orig[f].second);
        }

        newChannelData.emplace_back(interpolatedEndFrame.first - start, interpolatedEndFrame.second);

        return newChannelData;
    }

} // namespace get
