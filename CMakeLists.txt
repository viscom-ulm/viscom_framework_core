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

set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Fatal (fatal-crashes/contract) examples " FORCE)
set(ADD_BUILD_WIN_SHARED OFF CACHE BOOL "Build shared library on Windows" FORCE)
set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Turn ON/OFF log levels. An disabled level will not push logs of that level to the sink. By default dynamic logging is disabled" FORCE)
add_subdirectory(extern/g3log)

find_package(OpenCV REQUIRED)
file(GLOB OPENCV_DEBUG_DLLs $ENV{OPENCV_DIR}/bin/Debug/*.dll)
file(GLOB OPENCV_RELEASE_DLLs $ENV{OPENCV_DIR}/bin/Release/*.dll)
file(COPY ${OPENCV_DEBUG_DLLs} DESTINATION ${PROJECT_SOURCE_DIR})
file(COPY ${OPENCV_RELEASE_DLLs} DESTINATION ${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE CFG_FILES ${PROJECT_SOURCE_DIR}/config/*.*)
file(GLOB_RECURSE DATA_FILES ${PROJECT_SOURCE_DIR}/data/*.*)
file(GLOB_RECURSE SHADER_FILES ${PROJECT_SOURCE_DIR}/shader/*.*)
file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp)
source_group("config" FILES ${CFG_FILES})
source_group("data" FILES ${DATA_FILES})
source_group("shader" FILES ${SHADER_FILES})

foreach(f ${SRC_FILES})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR} ${f})
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)

add_executable(${APP_NAME} ${SRC_FILES} ${CFG_FILES} ${DATA_FILES} ${SHADER_FILES})

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

include_directories(${SGCT_INCLUDE_DIRECTORY} extern/g3log/src)

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

target_link_libraries(${APP_NAME} ${LIBS} ${OpenCV_LIBS} g3logger)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
configure_file("framework_ide.cfg" "framework.cfg" COPYONLY)

