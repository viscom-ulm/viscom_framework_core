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
constexpr bool USE_TUIO = true;
#else
constexpr bool USE_TUIO = false;
#endif

