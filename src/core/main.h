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


#ifdef VISCOM_USE_TUIO
namespace TUIO {
    class TuioCursor;
}
#else
namespace TUIO {
    /** Defines the Tuio cursor as void if Tuio ist not used. */
    using TuioCursor = void;
}
#endif

namespace viscom {

    class ApplicationNodeBase;
    class ApplicationNodeInternal;

    /** The function that will create a coordinator or worker node. */
    using InitNodeFunc = std::function<std::unique_ptr<ApplicationNodeBase>(ApplicationNodeInternal*)>;

#ifdef _DEBUG
    /** Defines if the framework should run in debug mode. */
    constexpr bool DEBUG_MODE = true;
#else
    /** Defines if the framework should run in debug mode. */
    constexpr bool DEBUG_MODE = false;
#endif

#ifdef VISCOM_USE_TUIO
    /** Defines if Tuio should be used for touch screens. */
    constexpr bool USE_TUIO = true;
#else
    /** Defines if Tuio should be used for touch screens. */
    constexpr bool USE_TUIO = false;
#endif

#ifdef VISCOM_USE_SGCT
    /** Defines if SGCT should be used. */
    constexpr bool USE_SGCT = true;
#else
    /** Defines if SGCT should be used. */
    constexpr bool USE_SGCT = false;
#endif

#ifdef VISCOM_LOCAL_ONLY
    /** Defines if distortion should be used for the screens. */
    constexpr bool USE_DISTORTION = false;
#else
    /** Defines if distortion should be used for the screens. */
    constexpr bool USE_DISTORTION = true;
#endif


#ifdef VISCOM_CLIENTGUI
    /** Defines if the client GUI should be displayed. */
    constexpr bool SHOW_CLIENT_GUI = true;
#else
    /** Defines if the client GUI should be displayed. */
    constexpr bool SHOW_CLIENT_GUI = false;
#endif

#ifdef VISCOM_SYNCINPUT
    /** Defines if the input should be synchronized. */
    constexpr bool SYNCHRONIZE_INPUT = true;
#else
    /** Defines if the input should be synchronized. */
    constexpr bool SYNCHRONIZE_INPUT = false;
#endif

#ifdef VISCOM_CLIENTMOUSECURSOR
    /** Defines if the client mouse cursor should be displayed. */
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = true;
#else
    /** Defines if the client mouse cursor should be displayed. */
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = false;
#endif

    /** Defines the resource types. */
    enum class ResourceType : std::uint8_t {
        All_Resources,
        Texture,
        Mesh,
        GPUProgram
    };

    /**
     *  Writes the resource type to a stream.
     *  @param str stream to write to.
     *  @param v resource type.
     */
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

#ifdef _MSC_VER
#define PUSH_DISABLE_DEPRECATED_WARNINGS __pragma(warning( push )) \
__pragma(warning( disable: 4996 ))
#define POP_WARNINGS __pragma(warning( pop ))
#elif __GNUC__
#define PUSH_DISABLE_DEPRECATED_WARNINGS _Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define POP_WARNINGS _Pragma("GCC diagnostic pop")
#elif __clang__
#define PUSH_DISABLE_DEPRECATED_WARNINGS _Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define POP_WARNINGS _Pragma("clang diagnostic pop")
#endif

#define _unused(x) ((void)(x))

