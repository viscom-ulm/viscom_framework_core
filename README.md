# Core Library for the VISCOM Framework

[![Build Status](https://travis-ci.com/viscom-ulm/viscom_framework_core.svg?branch=master)](https://travis-ci.com/viscom-ulm/viscom_framework_core)

## Build (Windows, *nix)
- Use [conan](https://conan.io/)
- Setup the remotes for openvr and glfw (need to do this only once):

  ```conan remote add arsen-studio https://api.bintray.com/conan/arsen-studio/arsen-deps```

  ```conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan```
- From the project build folder run:

  ```conan install --build=missing --install-folder=./fwcore ../extern/fwcore/```

  ```conan install --build=missing --install-folder=./fwcore -s build_type=Debug ../extern/fwcore/```

**OR**
- From this folder run:

  ```conan install --build=missing --install-folder=./fwcore ..```

  ```conan install --build=missing --install-folder=./fwcore -s build_type=Debug ..```

## Build (OSX)
- Use [conan](https://conan.io/)
- Setup the remotes for glfw (need to do this only once):

  ```conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan```
- From the project build folder run:

  ```conan install --build=missing --install-folder=./fwcore ../extern/fwcore/conanfile-osx.txt```

  ```conan install --build=missing --install-folder=./fwcore -s build_type=Debug ../extern/fwcore/conanfile-osx.txt```

**OR**
- From this folder run:

  ```conan install --build=missing --install-folder=./fwcore ../conanfile-osx.txt```

  ```conan install --build=missing --install-folder=./fwcore -s build_type=Debug ../conanfile-osx.txt```
