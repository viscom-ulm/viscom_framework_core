macro(run_conan)
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_add_remote(NAME bincrafters URL
                 https://api.bintray.com/conan/bincrafters/public-conan)
conan_add_remote(NAME conan-center URL https://conan.bintray.com)
conan_add_remote(NAME conan-community URL https://api.bintray.com/conan/conan-community/conan)

set(VISCOM_CONAN_BUILD_ALL ON CACHE BOOL "Builds all conan modules from source.")
if (${VISCOM_CONAN_BUILD_ALL})
    set(VISCOM_INTERNAL_CONAN_BUILD "all")
else()
    set(VKFW_INTERNAL_CONAN_BUILD "missing")
endif()

conan_cmake_run(
  REQUIRES ${CONAN_EXTRA_REQUIRES}
  OPTIONS ${CONAN_EXTRA_OPTIONS}
  BASIC_SETUP
  CMAKE_TARGETS # individual targets to link to
  BUILD ${VKFW_INTERNAL_CONAN_BUILD})
endmacro()
