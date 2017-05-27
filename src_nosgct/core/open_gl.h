/**
 * @file   open_gl.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.24
 *
 * @brief  Include header for OpenGL headers in case SGCT is not used.
 */

#pragma once

// Make sure this header is included before GLFW. [5/27/2017 Sebastian Maisch]
#ifdef WITH_TUIO
#include <TuioListener.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
