cmake_minimum_required(VERSION 3.7)
SET(APP_NAME ProjectorFramework)

PROJECT(${APP_NAME})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force compiler to create static libs." FORCE)
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Fatal (fatal-crashes/contract) examples " FORCE)
set(ADD_BUILD_WIN_SHARED OFF CACHE BOOL "Build shared library on Windows" FORCE)
set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Turn ON/OFF log levels. An disabled level will not push logs of that level to the sink. By default dynamic logging is disabled" FORCE)
set(CHANGE_G3LOG_DEBUG_TO_DBUG ON CACHE BOOL "Use DBUG logging level instead of DEBUG. By default DEBUG is the debugging level" FORCE)
add_subdirectory(extern/g3log)


set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
option(ASSIMP_NO_EXPORT ON)
# Disable all unused importers
option(ASSIMP_BUILD_3DS_IMPORTER OFF)
option(ASSIMP_BUILD_3D_IMPORTER OFF)
option(ASSIMP_BUILD_3MF_IMPORTER OFF)
option(ASSIMP_BUILD_AC_IMPORTER OFF)
option(ASSIMP_BUILD_AMF_IMPORTER OFF)
option(ASSIMP_BUILD_AMF_IMPORTER OFF)
option(ASSIMP_BUILD_ASE_IMPORTER OFF)
option(ASSIMP_BUILD_ASSBIN_IMPORTER OFF)
option(ASSIMP_BUILD_ASSXML_IMPORTER OFF)
option(ASSIMP_BUILD_B3D_IMPORTER OFF)
option(ASSIMP_BUILD_BLEND_IMPORTER OFF)
option(ASSIMP_BUILD_BVH_IMPORTER OFF)
option(ASSIMP_BUILD_COB_IMPORTER OFF)
option(ASSIMP_BUILD_CSM_IMPORTER OFF)
option(ASSIMP_BUILD_DXF_IMPORTER OFF)
option(ASSIMP_BUILD_GLTF_IMPORTER OFF)
option(ASSIMP_BUILD_HMP_IMPORTER OFF)
option(ASSIMP_BUILD_IFC_IMPORTER OFF)
option(ASSIMP_BUILD_IRRMESH_IMPORTER OFF)
option(ASSIMP_BUILD_IRR_IMPORTER OFF)
option(ASSIMP_BUILD_LWO_IMPORTER OFF)
option(ASSIMP_BUILD_LWS_IMPORTER OFF)
option(ASSIMP_BUILD_MD2_IMPORTER OFF)
option(ASSIMP_BUILD_MD3_IMPORTER OFF)
option(ASSIMP_BUILD_MD5_IMPORTER OFF)
option(ASSIMP_BUILD_MDC_IMPORTER OFF)
option(ASSIMP_BUILD_MDL_IMPORTER OFF)
option(ASSIMP_BUILD_MS3D_IMPORTER OFF)
option(ASSIMP_BUILD_NDO_IMPORTER OFF)
option(ASSIMP_BUILD_NFF_IMPORTER OFF)
option(ASSIMP_BUILD_NONFREE_C4D_IMPORTER OFF)
option(ASSIMP_BUILD_OFF_IMPORTER OFF)
option(ASSIMP_BUILD_OGRE_IMPORTER OFF)
option(ASSIMP_BUILD_OPENGEX_IMPORTER OFF)
option(ASSIMP_BUILD_Q3BSP_IMPORTER OFF)
option(ASSIMP_BUILD_Q3D_IMPORTER OFF)
option(ASSIMP_BUILD_RAW_IMPORTER OFF)
option(ASSIMP_BUILD_SIB_IMPORTER OFF)
option(ASSIMP_BUILD_SMD_IMPORTER OFF)
option(ASSIMP_BUILD_TERRAGEN_IMPORTER OFF)
option(ASSIMP_BUILD_X3D_IMPORTER OFF)
option(ASSIMP_BUILD_XGL_IMPORTER OFF)
option(ASSIMP_BUILD_X_IMPORTER OFF)
add_subdirectory(extern/assimp)


set(VISCOM_CLIENTGUI ON CACHE BOOL "Use ImGui on clients.")
set(VISCOM_SYNCINPUT ON CACHE BOOL "Synchronize input from master to clients.")
set(VISCOM_CLIENTMOUSECURSOR ON CACHE BOOL "Show the mouse cursor on clients.")
set(VISCOM_CONFIG_NAME "single" CACHE STRING "Name/directory of the configuration files to be used.")
set(VISCOM_APP_NAME "${APP_NAME}" CACHE STRING "Name of the application to be build.")
set(VISCOM_LOCAL_ONLY ON CACHE BOOL "Only do a local build without calibration information.")
set(VISCOM_VIRTUAL_SCREEN_X 1920 CACHE INTEGER "Virtual screen size in x direction.")
set(VISCOM_VIRTUAL_SCREEN_Y 1080 CACHE INTEGER "Virtual screen size in y direction.")

# Build-flags.
if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wno-unused-function -Wno-unused-parameter -Wextra -Wpedantic -std=c++14")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14")

  # add compiler-options for AppleClang to ignore linker warnings from assimp on macOS
  if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    add_compile_options(-fvisibility=hidden -fvisibility-inlines-hidden)
  endif()
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /MP")
endif()


file(GLOB_RECURSE CFG_FILES ${PROJECT_SOURCE_DIR}/config/*.*)
file(GLOB_RECURSE DATA_FILES ${PROJECT_SOURCE_DIR}/data/*.*)
file(GLOB_RECURSE SHADER_FILES ${PROJECT_SOURCE_DIR}/resources/shader/*.*)
file(GLOB EXTERN_SOURCES extern/imgui/imgui.cpp extern/imgui/imgui_draw.cpp extern/imgui/imgui_demo.cpp)
file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp)
source_group("shader" FILES ${SHADER_FILES})
source_group("extern" FILES ${EXTERN_SOURCES})

if (${VISCOM_LOCAL_ONLY})
    list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/core/SlaveNodeInternal.h")
    list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/core/SlaveNodeInternal.cpp")
    list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/core/OpenCVParserHelper.h")
    list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/core/OpenCVParserHelper.cpp")
endif()

foreach(f ${SRC_FILES})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR} ${f})
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

add_executable(${APP_NAME} ${SRC_FILES} ${SHADER_FILES} ${EXTERN_SOURCES})
set_target_properties(${APP_NAME} PROPERTIES OUTPUT_NAME ${VISCOM_APP_NAME})

find_package(OpenGL REQUIRED)
find_library(SGCT_RELEASE_LIBRARY NAMES sgct libsgct PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
find_library(SGCT_DEBUG_LIBRARY NAMES sgctd libsgctd PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
find_path(SGCT_INCLUDE_DIRECTORY NAMES sgct PATHS $ENV{SGCT_ROOT_DIR}/include NO_DEFAULT_PATH REQUIRED)

set(SGCT_LIBS
    debug ${SGCT_DEBUG_LIBRARY}
    optimized ${SGCT_RELEASE_LIBRARY}
)

include_directories(${SGCT_INCLUDE_DIRECTORY}
                    src
                    extern/g3log/src 
                    extern/imgui
                    extern/stb/
                    extern/assimp/include/)
target_link_libraries(${APP_NAME} ${SGCT_LIBS} ${OPENGL_LIBRARY} ws2_32 g3logger assimp)

configure_file("framework_ide.cfg" "framework.cfg")
configure_file("framework_ide_local_slave.cfg" "framework_local_slave.cfg")

if(MSVC)
    target_compile_definitions(${APP_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

if (${VISCOM_CLIENTMOUSECURSOR})
    target_compile_definitions(${APP_NAME} PRIVATE VISCOM_CLIENTGUI VISCOM_SYNCINPUT VISCOM_CLIENTMOUSECURSOR)
else()
    if(${VISCOM_SYNCINPUT})
        target_compile_definitions(${APP_NAME} PRIVATE VISCOM_SYNCINPUT)
    endif()
    if(${VISCOM_CLIENTGUI})
        target_compile_definitions(${APP_NAME} PRIVATE VISCOM_CLIENTGUI)
    endif()
endif()

if (${VISCOM_LOCAL_ONLY})
    target_compile_definitions(${APP_NAME} PRIVATE VISCOM_LOCAL_ONLY)
endif()
