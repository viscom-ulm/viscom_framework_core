/**
 * @file   main.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.24
 *
 * @brief  Variables, definitions and includes used throughout the framework.
 */

#pragma once

// ReSharper disable CppUnusedIncludeDirective
#include <string>
#include <array>
#include <vector>
#include <glm/glm.hpp>

#include <g3log/g3log.hpp>
// ReSharper restore CppUnusedIncludeDirective

#include <core/config.h>
#include <core/utils/utils.h>

namespace viscom {

    class ApplicationNodeBase;
    class ApplicationNodeInternal;
    using InitNodeFunc = std::function<std::unique_ptr<ApplicationNodeBase>(ApplicationNodeInternal*)>;

#ifdef _DEBUG
    constexpr bool DEBUG_MODE = true;
#else
    constexpr bool DEBUG_MODE = false;
#endif

#ifdef VISCOM_USE_TUIO
    constexpr bool USE_TUIO = true;
#else
    constexpr bool USE_TUIO = false;
#endif

#ifdef VISCOM_USE_SGCT
    constexpr bool USE_SGCT = true;
#else
    constexpr bool USE_SGCT = false;
#endif

#ifdef VISCOM_LOCAL_ONLY
    constexpr bool USE_DISTORTION = false;
#else
    constexpr bool USE_DISTORTION = true;
#endif


#ifdef VISCOM_CLIENTGUI
    constexpr bool SHOW_CLIENT_GUI = true;
#else
    constexpr bool SHOW_CLIENT_GUI = false;
#endif

#ifdef VISCOM_SYNCINPUT
    constexpr bool SYNCHRONIZE_INPUT = true;
#else
    constexpr bool SYNCHRONIZE_INPUT = false;
#endif

#ifdef VISCOM_CLIENTMOUSECURSOR
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = true;
#else
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = false;
#endif


    enum class ResourceType : std::uint8_t {
        All_Resources,
        Texture,
        Mesh,
        GPUProgram
    };

    inline std::ostream& operator<<(std::ostream& str, ResourceType v) {
        switch (v)
        {
        case viscom::ResourceType::All_Resources:
            return str << "All Resources";
        case viscom::ResourceType::Texture:
            return str << "Texture";
        case viscom::ResourceType::Mesh:
            return str << "Mesh";
        case viscom::ResourceType::GPUProgram:
            return str << "GPUProgram";
        default:
            return str << "unknown Resource";
        }
    }
}
