/**
 * @file   config.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.23
 *
 * @brief  Declaration of configuration related structs and functions.
 */

#pragma once

 // ReSharper disable CppUnusedIncludeDirective
#include <string>
#include <vector>
#include <glm/glm.hpp>
 // ReSharper restore CppUnusedIncludeDirective

namespace viscom {

    /** Holds all the configuration values. */
    struct FWConfiguration
    {
        static constexpr const char* CALIBRATION_WALL_COORDINATES_IMAGE = "WallCoordinatesImage";
        static constexpr const char* CALIBRATION_WALL_COORDINATES_VIEWPLANE = "WallCoordinatesViewplane";
        static constexpr const char* CALIBRATION_PROJECTOR_COORDINATES_IMAGE = "ProjectorCoordinatesImage";
        static constexpr const char* CALIBRATION_QUAD_CORNERS_NAME = "screenQuadCoords";
        static constexpr const char* CALIBRATION_QUAD_TEX_COORDS_NAME = "screenQuadTexCoords";
        static constexpr const char* CALIBRATION_QUAD_RESOLUTION_SCALING_NAME = "resolutionScaling";
        static constexpr const char* CALIBRATION_VIEWPORT_NAME = "viewport";
        static constexpr const char* CALIBRATION_ALPHA_TEXTURE_NAME = "alphaTexture";

        std::string baseDirectory_;
        std::string viscomConfigName_;
        std::string programProperties_;
        std::string sgctConfig_;
        std::string projectorData_;
        std::string sgctLocal_;
        bool sgctSlave_ = false;
        int tuioPort_ = 3333;
        glm::vec2 virtualScreenSize_;
        glm::vec2 nearPlaneSize_;
        std::vector<std::string> resourceSearchPaths_;
        std::string openglProfile_;
    };

    /**
     *  Loads a config file to set all the configuration values.
     *  @param configFilename name of the config file.
     */
    FWConfiguration LoadConfiguration(const std::string& configFilename);
}
