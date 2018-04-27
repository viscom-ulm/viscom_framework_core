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

#ifdef VISCOM_CLIENTMOUSECURSOR
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = true;
#else
    constexpr bool SHOW_CLIENT_MOUSE_CURSOR = false;
#endif

#ifdef VISCOM_CLIENTGUI
    constexpr bool SHOW_CLIENT_GUI = true;
#else
    constexpr bool SHOW_CLIENT_GUI = false;
#endif

#ifdef VISCOM_CLIENTGUI
    constexpr bool SHOW_CLIENT_GUI = true;
#else
    constexpr bool SHOW_CLIENT_GUI = false;
#endif
}
