/**
 * @file   config.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Implementation of configuration related structs and functions.
 */

#include "config.h"
#include "main.h"
#include <fstream>
#include <iostream>

namespace viscom {

    FWConfiguration LoadConfiguration(const std::string& configFilename)
    {
        LOG(INFO) << "Loading configuration.";
        std::ifstream ifs(configFilename);
        if (!ifs.is_open()) {
            LOG(INFO) << "Could not open config file (" << configFilename << ")." << std::endl;
            std::cout << "Could not open config file (" << configFilename << ")." << std::endl;
            throw std::runtime_error("Could not open config file (" + configFilename + ").");
        }

        FWConfiguration config;
        std::string str;

        while (ifs >> str && ifs.good()) {
            if (str == "BASE_DIR=") ifs >> config.baseDirectory_;
            else if (str == "VISCOM_CONFIG=") ifs >> config.viscomConfigName_;
            else if (str == "PROGRAM_PROPERTIES=") ifs >> config.programProperties_;
            else if (str == "SGCT_CONFIG=") ifs >> config.sgctConfig_;
            else if (str == "PROJECTOR_DATA=") ifs >> config.projectorData_;
            else if (str == "LOCAL=") ifs >> config.sgctLocal_;
            else if (str == "--slave") config.sgctSlave_ = true;
            else if (str == "TUIO_PORT=") ifs >> config.tuioPort_;
            else if (str == "VIRTUAL_SCREEN_X=") ifs >> config.virtualScreenSize_.x;
            else if (str == "VIRTUAL_SCREEN_Y=") ifs >> config.virtualScreenSize_.y;
        }
        ifs.close();

        config.resourceSearchPaths_.emplace_back(config.baseDirectory_ + "resources/");
        config.resourceSearchPaths_.emplace_back(config.baseDirectory_ + "extern/fwcore/resources/");

        return config;
    }
}
