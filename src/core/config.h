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
        /** Configuration names for calibration. */
        static constexpr const char* CALIBRATION_WALL_COORDINATES_IMAGE = "WallCoordinatesImage";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_WALL_COORDINATES_VIEWPLANE = "WallCoordinatesViewplane";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_PROJECTOR_COORDINATES_IMAGE = "ProjectorCoordinatesImage";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_QUAD_CORNERS_NAME = "screenQuadCoords";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_QUAD_TEX_COORDS_NAME = "screenQuadTexCoords";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_QUAD_RESOLUTION_SCALING_NAME = "resolutionScaling";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_VIEWPORT_NAME = "viewport";
        /** @see CALIBRATION_WALL_COORDINATES_IMAGE */
        static constexpr const char* CALIBRATION_ALPHA_TEXTURE_NAME = "alphaTexture";

        /** The frameworks base directory path. */
        std::string baseDirectory_;
        /** The name of the configuration file. */
        std::string viscomConfigName_;
        /** The path to the properties file. */
        std::string programProperties_;
        /** The path to the SGCT configuration file. */
        std::string sgctConfig_;
        /** The path to the projector data file. */
        std::string projectorData_;
        /** The path to the projector colors data file. */
        std::string projectorColorData_;
        /** Index to node to use settings from. */
        std::string sgctLocal_;
        /** Defines if the node is a worker or coordinator. */
        bool sgctSlave_ = false;
        /** TUIO port for touch screen. */
        int tuioPort_ = 3333;
        /** Virtual size of the screen. If only one screen exists the virtual screen size is the actual size of this screen. */
        glm::vec2 virtualScreenSize_ = glm::vec2{ 0.0f };
        /** Relative size of the near plane. Y is always 1.0 while X is set to match the screens aspect ratio. */
        glm::vec2 nearPlaneSize_ = glm::vec2{ 0.0f };
        /** All paths that should be searched in for resource loading. */
        std::vector<std::string> resourceSearchPaths_;
        /** Defines the OpenGL Version. */
        std::string openglProfile_;
        /** A prefix for the shader search path in the resource directory. */
        std::string shaderSearchPrefix = "shader";
    };

    /**
     *  Loads a config file to set all the configuration values.
     *  @param configFilename name of the config file.
     */
    FWConfiguration LoadConfiguration(const std::string& configFilename);
}
