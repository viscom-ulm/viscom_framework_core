cmake_minimum_required(VERSION 3.7)
SET(APP_NAME ProjectorFramework)
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
if( WIN32 )
    SET(CMAKE_FIND_LIBRARY_PREFIXES "")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
else()
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
endif()

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
set(VISCOM_CONFIG_NAME "quad" CACHE STRING "Name/directory of the configuration files to be used.")
set(VISCOM_APP_NAME "${APP_NAME}" CACHE STRING "Name of the application to be build.")

file(GLOB_RECURSE CFG_FILES ${PROJECT_SOURCE_DIR}/config/*.*)
file(GLOB_RECURSE DATA_FILES ${PROJECT_SOURCE_DIR}/data/*.*)
file(GLOB_RECURSE SHADER_FILES ${PROJECT_SOURCE_DIR}/resources/shader/*.*)
file(GLOB EXTERN_SOURCES extern/imgui/imgui.cpp extern/imgui/imgui_draw.cpp extern/imgui/imgui_demo.cpp)
file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp)
source_group("shader" FILES ${SHADER_FILES})
source_group("extern" FILES ${EXTERN_SOURCES})

foreach(f ${SRC_FILES})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR} ${f})
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)

add_executable(${APP_NAME} ${SRC_FILES} ${SHADER_FILES} ${EXTERN_SOURCES})
set_target_properties(${APP_NAME} PROPERTIES OUTPUT_NAME ${VISCOM_APP_NAME})

if( APPLE )
    option(SGCT_CPP11 "Use libc++ instead of libstdc++" ON)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    if(CMAKE_GENERATOR STREQUAL Xcode)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
    endif()
endif()

if(SGCT_CPP11)
    find_library(SGCT_RELEASE_LIBRARY NAMES sgct_cpp11 libsgct_cpp11 PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
    find_library(SGCT_DEBUG_LIBRARY NAMES sgct_cpp11d libsgct_cpp11d PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
else()
    find_library(SGCT_RELEASE_LIBRARY NAMES sgct libsgct PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
    find_library(SGCT_DEBUG_LIBRARY NAMES sgctd libsgctd PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
endif()

if( WIN32 )
    find_path(SGCT_INCLUDE_DIRECTORY NAMES sgct PATHS $ENV{SGCT_ROOT_DIR}/include NO_DEFAULT_PATH REQUIRED)
else()
    find_path(SGCT_INCLUDE_DIRECTORY NAMES sgct PATH_SUFFIXES sgct PATHS $ENV{SGCT_ROOT_DIR}/include REQUIRED)
endif()

set(SGCT_LIBS
    debug ${SGCT_DEBUG_LIBRARY}
    optimized ${SGCT_RELEASE_LIBRARY}
)

find_package(OpenGL REQUIRED)

include_directories(${SGCT_INCLUDE_DIRECTORY}
                    src
                    extern/g3log/src 
                    extern/imgui
                    extern/stb/
                    extern/assimp/include/)

if( MSVC )
    set(LIBS ${SGCT_LIBS} ${OPENGL_LIBRARY} ws2_32)
elseif( WIN32 ) #MINGW or similar
    set(LIBS ${SGCT_LIBS} ${OPENGL_LIBRARY} ws2_32)
elseif( APPLE )
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    set(LIBS ${SGCT_LIBS} ${OPENGL_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
else() #linux
    find_library(X11_LIBRARY X11 REQUIRED)
    find_library(XRANDR_LIBRARY Xrandr REQUIRED)
    find_library(XI_LIBRARY Xi REQUIRED)
    find_library(XXF86VM_LIBRARY Xxf86vm)
    find_library(PTHREAD pthread REQUIRED)

    set(LIBS ${SGCT_LIBS} ${OPENGL_LIBRARY} ${X11_LIBRARY} ${XRANDR_LIBRARY} ${XI_LIBRARY} ${XXF86VM_LIBRARY} ${PTHREAD})
endif()

if( MSVC )
    if( "${MSVC_VERSION}" LESS 1600 ) #less than visual studio 2010
        STRING(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${replacementFlags}" )

        STRING(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
        SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${replacementFlags}" )
    else()
        STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
        SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${replacementFlags}" )

        STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
        SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${replacementFlags}" )
    endif()
endif()

if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
    add_definitions( "/MP" )
endif()

if( WIN32 )
    add_definitions(-D__WIN32__)
    if( MINGW )
        set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
elseif( APPLE  )
    add_definitions(-D__APPLE__)
    if(SGCT_CPP11)
        set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
        set_xcode_property(${APP_NAME} CLANG_CXX_LANGUAGE_STANDARD "c++11")
        set_xcode_property(${APP_NAME} CLANG_CXX_LIBRARY "libc++")
    else()
        set(CMAKE_CXX_FLAGS "-std=c++0x -stdlib=libstdc++ ${CMAKE_CXX_FLAGS}")
        set_xcode_property(${APP_NAME} CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        set_xcode_property(${APP_NAME} CLANG_CXX_LIBRARY "libstdc++")
    endif()
else()
    add_definitions(-D__LINUX__)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

if( CMAKE_COMPILER_IS_GNUCXX )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

target_link_libraries(${APP_NAME} ${LIBS} g3logger assimp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
configure_file("framework_ide.cfg" "framework.cfg")
target_compile_definitions(${APP_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)

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
