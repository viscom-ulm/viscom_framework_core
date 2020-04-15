cmake_minimum_required(VERSION 3.9)
PROJECT("VISCOMCore")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-stdlib=libc++ -isystem /usr/include/c++/v1)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows.")
set(CMAKE_DEBUGWORKER_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows.")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows.")

set(G3_SHARED_LIB OFF CACHE BOOL "Build shared library." FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force compiler to create static libs." FORCE)
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Fatal (fatal-crashes/contract) examples " FORCE)
set(ADD_BUILD_WIN_SHARED OFF CACHE BOOL "Build shared library on Windows" FORCE)
set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Turn ON/OFF log levels. An disabled level will not push logs of that level to the sink. By default dynamic logging is disabled" FORCE)
set(CHANGE_G3LOG_DEBUG_TO_DBUG ON CACHE BOOL "Use DBUG logging level instead of DEBUG. By default DEBUG is the debugging level" FORCE)
set(VERSION "1.3.160-0")
add_subdirectory(extern/g3log EXCLUDE_FROM_ALL)
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(g3logger PUBLIC -fvisibility=hidden -fvisibility-inlines-hidden)
endif()

set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

find_package(Doxygen)

set(VISCOM_CLIENTGUI ON CACHE BOOL "Use ImGui on clients.")
set(VISCOM_SYNCINPUT OFF CACHE BOOL "Synchronize input from master to clients.")
set(VISCOM_CLIENTMOUSECURSOR ON CACHE BOOL "Show the mouse cursor on clients.")
set(VISCOM_APP_NAME "ProjectorFramework" CACHE STRING "Name of the application to be build.")
set(VISCOM_LOCAL_ONLY ON CACHE BOOL "Only do a local build without calibration information.")
set(VISCOM_USE_SGCT ON CACHE BOOL "Use SGCT for local builds.")
set(VISCOM_CONFIG_NAME "single" CACHE STRING "Name/directory of the configuration files to be used.")
set(VISCOM_VIRTUAL_SCREEN_X 1920 CACHE STRING "Virtual screen size in x direction.")
set(VISCOM_VIRTUAL_SCREEN_Y 1080 CACHE STRING "Virtual screen size in y direction.")
set(VISCOM_NEAR_PLANE_SIZE_X "1.7778" CACHE STRING "Size of the near plane in x direction.")
set(VISCOM_NEAR_PLANE_SIZE_Y "1.0" CACHE STRING "Size of the near plane in y direction.")
set(VISCOM_INSTALL_BASE_PATH "D:/LabShare/cluster/apps/" CACHE PATH "Path to install the project to (should be the shared apps directory).")
set(VISCOM_OPENGL_PROFILE "3.3" CACHE STRING "OpenGL profile version to use.")
# This could be changed to "off" if the default target doesn't need the tuio library
set(VISCOM_USE_TUIO ON CACHE BOOL "Use TUIO input library")
set(VISCOM_TUIO_PORT 3333 CACHE STRING "UDP Port for TUIO to listen on")
set(VISCOM_USE_OPEN_VR OFF CACHE BOOL "Use OpenVR library")
option(VISCOM_ENABLE_AVX "Enable AVX optimization for release build." OFF)
option(VISCOM_ENABLE_AVX2 "Enable AVX2 optimization for release build." OFF)

# Hide the "Use open vr" option under MacOS
if(APPLE)
    mark_as_advanced(FORCE VISCOM_USE_OPEN_VR)
else()
    mark_as_advanced(CLEAR VISCOM_USE_OPEN_VR)
endif()


# Build-flags.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND CORE_LIBS c++experimental c++abi c++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CORE_LIBS stdc++fs)
endif()

macro(set_build_flags TARGET_NAME USE_CONVERSION)
    if(UNIX)
        if(ENABLE_AVX2)
            target_compile_options(${TARGET_NAME} PUBLIC -mavx2)
        elseif(ENABLE_AVX)
            target_compile_options(${TARGET_NAME} PUBLIC -mavx)
        endif()
        target_compile_options(${TARGET_NAME} PUBLIC -Werror -O0 -Wall -Wno-unused-function -Wno-unused-parameter -Wextra -Wpedantic $<${USE_CONVERSION}:-Wconversion>)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${TARGET_NAME} PUBLIC -stdlib=libc++ -Wno-unused-command-line-argument)
            target_include_directories(${TARGET_NAME} SYSTEM BEFORE PUBLIC /usr/include/c++/v1)
            if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
                target_compile_definitions(${TARGET_NAME} PUBLIC VISCOM_NO_FILESYSTEM)
            endif()
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            target_compile_options(${TARGET_NAME} PUBLIC -fvisibility=hidden -fvisibility-inlines-hidden -Wno-unused-function -ferror-limit=0 -Wno-gnu)
            target_compile_definitions(${TARGET_NAME} PUBLIC GLM_FORCE_SILENT_WARNINGS)
            if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
                target_compile_definitions(${TARGET_NAME} PUBLIC VISCOM_NO_FILESYSTEM)
            endif()
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
                target_compile_definitions(${TARGET_NAME} PUBLIC VISCOM_NO_FILESYSTEM)
            elseif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.2)
                # can be removed if ImGUI is updated to a version newer than may 19th 2019 (commit f242cd4d8a516b8993fb55d61445b03cfde7fe78).
                target_compile_options(${TARGET_NAME} PUBLIC -Wno-class-memaccess)
            endif()
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(ENABLE_AVX2)
            target_compile_options(${TARGET_NAME} PUBLIC /arch:AVX2)
        elseif(ENABLE_AVX)
            target_compile_options(${TARGET_NAME} PUBLIC /arch:AVX)
        endif()
        target_compile_options(${TARGET_NAME} PUBLIC /WX /W3 /EHsc /MP)
    endif()
endmacro()

if (${VISCOM_USE_SGCT})
    list(APPEND COMPILE_TIME_DEFS VISCOM_USE_SGCT)
    set(VISCOM_SGCT_TAG "sgct")
    set(VISCOM_SGCT_WRAPPER_DIR ${PROJECT_SOURCE_DIR}/src_sgct/)
else()
    option(GLFW_BUILD_DOCS OFF)
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_INSTALL OFF)
    add_subdirectory(extern/glfw EXCLUDE_FROM_ALL)

    set(VISCOM_SGCT_TAG "nosgct")
    list(APPEND COMPILE_TIME_DEFS GLEW_STATIC)
    list(APPEND EXTERN_SOURCES_CORE src_nosgct/glew/src/glew.cc)
    set(VISCOM_SGCT_WRAPPER_DIR ${PROJECT_SOURCE_DIR}/src_nosgct/)
endif()


file(GLOB_RECURSE SHADER_FILES_CORE ${PROJECT_SOURCE_DIR}/resources/shader/*.*)
list(FILTER SHADER_FILES_CORE EXCLUDE REGEX ".*\.gen$")
file(GLOB_RECURSE SRC_FILES_CORE
    ${PROJECT_SOURCE_DIR}/src/core/*.h
    ${PROJECT_SOURCE_DIR}/src/core/*.cpp
    ${PROJECT_SOURCE_DIR}/src/core/*.inl
    ${VISCOM_SGCT_WRAPPER_DIR}/core/*.h
    ${VISCOM_SGCT_WRAPPER_DIR}/core/*.cpp
    ${VISCOM_SGCT_WRAPPER_DIR}/core/*.inl)
source_group("extern\\core" FILES ${EXTERN_SOURCES_CORE})
source_group("shader\\core" FILES ${SHADER_FILES_CORE})

if (${VISCOM_LOCAL_ONLY})
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/CalibrationVertices.h")
    list(REMOVE_ITEM SRC_FILES_CORE "${VISCOM_SGCT_WRAPPER_DIR}/core/app_internal/WorkerNodeCalibratedInternal.h")
    list(REMOVE_ITEM SRC_FILES_CORE "${VISCOM_SGCT_WRAPPER_DIR}/core/app_internal/WorkerNodeCalibratedInternal.cpp")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/OpenCVParserHelper.h")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/OpenCVParserHelper.cpp")
endif()

if(NOT ${VISCOM_USE_OPEN_VR})
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/app/OpenVRController.h")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/app/OpenVRController.cpp")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/app/OpenVR.cpp")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/app/DisplayPointerCalibrationController.h")
    list(REMOVE_ITEM SRC_FILES_CORE "${PROJECT_SOURCE_DIR}/src/core/app/DisplayPointerCalibrationController.cpp")
endif()

foreach(f ${SRC_FILES_CORE})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR} ${f})
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
    string(REPLACE / \\ SRCGR ${SRCGR})
    string(REPLACE src_sgct src SRCGR ${SRCGR})
    string(REPLACE src_nosgct src SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

file(GLOB TOP_SRC_FILES_CORE
    extern/glm/util/glm.natvis
    extern/imgui/misc/natvis/imgui.natvis)

if(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
endif()

if(MSVC)
    list(APPEND COMPILE_TIME_DEFS _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if (${VISCOM_USE_SGCT})
    find_package(OpenGL REQUIRED)
    find_library(SGCT_RELEASE_LIBRARY NAMES sgct libsgct PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
    find_library(SGCT_DEBUG_LIBRARY NAMES sgctd libsgctd PATHS $ENV{SGCT_ROOT_DIR}/lib REQUIRED)
    find_path(SGCT_INCLUDE_DIRECTORY NAMES sgct PATHS $ENV{SGCT_ROOT_DIR}/include NO_DEFAULT_PATH REQUIRED)

    set(SGCT_LIBS
        debug ${SGCT_DEBUG_LIBRARY}
        optimized ${SGCT_RELEASE_LIBRARY})

    list(APPEND CORE_LIBS ${SGCT_LIBS})
    if(MSVC)
        list(APPEND CORE_LIBS OpenGL::GL ws2_32)
    elseif(UNIX)
        find_package(Threads REQUIRED)
        if(OpenGL_GLX_FOUND)
            list(APPEND CORE_LIBS OpenGL::OpenGL OpenGL::GLX)
        else()
            list(APPEND CORE_LIBS OpenGL::GL)
        endif()
        list(APPEND CORE_LIBS dl X11 Xrandr Xcursor Xinerama Xxf86vm Threads::Threads)
    endif()
    add_library(SGCTWrapper ${VISCOM_SGCT_WRAPPER_DIR}/wrapper/sgct_wrapper.cpp)
    set_property(TARGET SGCTWrapper PROPERTY CXX_STANDARD 17)
    target_include_directories(SGCTWrapper PUBLIC ${VISCOM_SGCT_WRAPPER_DIR}/wrapper)
    target_include_directories(SGCTWrapper SYSTEM PUBLIC ${SGCT_INCLUDE_DIRECTORY})
    target_link_libraries(SGCTWrapper ${CORE_LIBS})
    target_compile_definitions(SGCTWrapper PUBLIC ${COMPILE_TIME_DEFS})
    set_build_flags(SGCTWrapper 0)

    list(APPEND CORE_LIBS SGCTWrapper)
    list(APPEND CORE_INCLUDE_DIRS extern/glm ${SGCT_INCLUDE_DIRECTORY})
else()
    find_package(OpenGL REQUIRED)
    list(APPEND CORE_LIBS glfw ${GLFW_LIBRARIES})
    if(MSVC)
        list(APPEND CORE_LIBS OpenGL::GL)
    elseif(UNIX)
        if(OpenGL_GLX_FOUND)
            list(APPEND CORE_LIBS OpenGL::OpenGL OpenGL::GLX)
        else()
            list(APPEND CORE_LIBS OpenGL::GL)
        endif()
    endif()
    list(APPEND CORE_INCLUDE_DIRS extern/glm extern/glfw/include)
endif()

list(APPEND CORE_INCLUDE_DIRS
    ${VISCOM_SGCT_WRAPPER_DIR}
    extern/g3log/src
    ${CMAKE_CURRENT_BINARY_DIR}/extern/g3log/include)

list(APPEND CORE_LIBS g3logger)


if (${VISCOM_CLIENTMOUSECURSOR})
    list(APPEND COMPILE_TIME_DEFS VISCOM_CLIENTGUI VISCOM_SYNCINPUT VISCOM_CLIENTMOUSECURSOR)
else()
    if(${VISCOM_SYNCINPUT})
        list(APPEND COMPILE_TIME_DEFS VISCOM_SYNCINPUT)
    endif()
    if(${VISCOM_CLIENTGUI})
        list(APPEND COMPILE_TIME_DEFS VISCOM_CLIENTGUI)
    endif()
endif()

if (${VISCOM_LOCAL_ONLY})
    list(APPEND COMPILE_TIME_DEFS VISCOM_LOCAL_ONLY)
endif()

if(${VISCOM_USE_TUIO})
    add_subdirectory(extern/tuio EXCLUDE_FROM_ALL)
    list(APPEND COMPILE_TIME_DEFS VISCOM_USE_TUIO)
    list(APPEND CORE_LIBS libTUIO)
    list(APPEND CORE_INCLUDE_DIRS extern/tuio/TUIO extern/tuio/oscpack)
endif()

if(TARGET Doxygen::doxygen)
    get_property(DOXYGEN_EXECUTABLE TARGET Doxygen::doxygen PROPERTY IMPORTED_LOCATION)

    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen ALL COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc COMMENT "Generating API documentation with Doxygen" VERBATIM)
    ## doxygen_add_docs(VISCOMCoreDoc src)
endif()


# Loading conan
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)


conan_cmake_run(REQUIRES assimp/5.0.1 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES imgui/1.75 BASIC_SETUP CMAKE_TARGETS BUILD missing)
conan_cmake_run(REQUIRES stb/20200203 BASIC_SETUP CMAKE_TARGETS BUILD missing)

# Download imgui.natvis
if(NOT EXISTS "${CMAKE_BINARY_DIR}/imgui.natvis")
   file(DOWNLOAD "https://github.com/ocornut/imgui/raw/master/misc/natvis/imgui.natvis" "${CMAKE_BINARY_DIR}/imgui.natvis")
endif()


if (${VISCOM_USE_OPEN_VR})
    conan_add_remote(NAME arsen-studio URL https://api.bintray.com/conan/arsen-studio/arsen-deps)
    conan_cmake_run(REQUIRES openvr/1.0.17@arsen-studio/stable BASIC_SETUP CMAKE_TARGETS BUILD missing)
    if(UNIX AND NOT APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SYS_NAME_STR "linux64")
        else()
            set(SYS_NAME_STR "linux32")
        endif()
    elseif(UNIX AND APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SYS_NAME_STR "osx64")
        else()
            set(SYS_NAME_STR "osx32")
        endif()
    elseif(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SYS_NAME_STR "win64")
        else()
            set(SYS_NAME_STR "win32")
        endif()
    endif()
    list(APPEND COMPILE_TIME_DEFS VISCOM_USE_OPEN_VR)
endif()

add_library(VISCOMExternal ${EXTERN_SOURCES_CORE})
set_property(TARGET VISCOMExternal PROPERTY CXX_STANDARD 17)
target_include_directories(VISCOMExternal SYSTEM PUBLIC ${CORE_INCLUDE_DIRS})
target_link_libraries(VISCOMExternal PUBLIC ${CORE_LIBS})
target_compile_definitions(VISCOMExternal PUBLIC ${COMPILE_TIME_DEFS} GLFW_INCLUDE_NONE)
set_build_flags(VISCOMExternal 0)

add_library(VISCOMCore ${SRC_FILES_CORE} ${SHADER_FILES_CORE} ${TOP_SRC_FILES_CORE})
set_property(TARGET VISCOMCore PROPERTY CXX_STANDARD 17)
target_include_directories(VISCOMCore PUBLIC src)
target_include_directories(VISCOMCore SYSTEM PUBLIC ${CORE_INCLUDE_DIRS})
target_link_libraries(VISCOMCore PUBLIC VISCOMExternal)
target_link_libraries(VISCOMCore PUBLIC CONAN_PKG::assimp)
target_link_libraries(VISCOMCore PUBLIC CONAN_PKG::imgui)
target_link_libraries(VISCOMCore PUBLIC CONAN_PKG::stb)

if (${VISCOM_USE_OPEN_VR})
    target_link_libraries(VISCOMCore PUBLIC CONAN_PKG::openvr)
endif()

target_compile_definitions(VISCOMCore PUBLIC ${COMPILE_TIME_DEFS} GLFW_INCLUDE_NONE)
set_build_flags(VISCOMCore 1)

macro(copy_core_lib_dlls APP_NAME)
    if (${VISCOM_USE_TUIO})
        add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:libTUIO> ${PROJECT_BINARY_DIR})
        install(FILES $<TARGET_FILE:libTUIO> DESTINATION ${VISCOM_INSTALL_BASE_PATH}/${VISCOM_APP_NAME})
    endif()
endmacro()

install(DIRECTORY resources/ DESTINATION ${VISCOM_INSTALL_BASE_PATH}/${VISCOM_APP_NAME}/resources)
