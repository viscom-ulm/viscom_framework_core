/**
 * @file   glfw.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.24
 *
 * @brief  Include header for GLFW headers in case SGCT is not used.
 */

#pragma once

// Make sure this header is included before GLFW. [5/27/2017 Sebastian Maisch]
#ifdef VISCOM_USE_TUIO
#include <TuioListener.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
