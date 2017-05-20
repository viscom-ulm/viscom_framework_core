/**
 * @file   main.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.24
 *
 * @brief  Program entry point.
 */

#include "main.h"
#include <fstream>
#include <iostream>
#include <g3log/logworker.hpp>
#include <g3log/loglevels.hpp>
#include "core/g3log/filesink.h"
#include "sgct.h"
#include "core/ApplicationNodeInternal.h"

namespace viscom {
    FWConfiguration LoadConfiguration(const std::string& configFilename);
    std::unique_ptr<ApplicationNodeInternal> SGCT_Init(FWConfiguration);
}


void SGCTLog(const char* msg)
{
    std::string msgStripped = msg;
    LOG(INFO) << msgStripped.substr(0, msgStripped.size() - 1);
}


int main(int argc, char** argv)
{
    const std::string directory = "./";
    const std::string name = "viscomlabfw.log";
    auto worker = g3::LogWorker::createLogWorker();
    auto handle = worker->addSink(std::make_unique<vku::FileSink>(name, directory, false), &vku::FileSink::fileWrite);

    g3::only_change_at_initialization::setLogLevel(WARNING, false);

#ifndef NDEBUG
    g3::only_change_at_initialization::setLogLevel(WARNING, true);
#endif

    initializeLogging(worker.get());

    LOG(INFO) << "Log created.";

    viscom::FWConfiguration config;
    if (argc > 1) config = viscom::LoadConfiguration(argv[1]);
    else config = viscom::LoadConfiguration("framework.cfg");

    auto appNode = SGCT_Init(config);

    // Main loop
    LOG(INFO) << "Started Rendering.";
    appNode->Render();

    return 0;
}

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
        }
        ifs.close();

        return config;
    }


    std::unique_ptr<ApplicationNodeInternal> SGCT_Init(FWConfiguration config) {
        std::vector<std::vector<char>> argVec;
        std::vector<char*> args;

        {
            argVec.emplace_back();
            std::string configArg = "-config";
            copy(configArg.begin(), configArg.end(), back_inserter(argVec.back()));
            argVec.back().push_back('\0');
            args.push_back(argVec.back().data());

            argVec.emplace_back();
            auto configFileArg = config.sgctConfig_;
            copy(configFileArg.begin(), configFileArg.end(), back_inserter(argVec.back()));
            argVec.back().push_back('\0');
            args.push_back(argVec.back().data());

            if (config.sgctLocal_ != "-1") {
                argVec.emplace_back();
                std::string localArg = "-local";
                copy(localArg.begin(), localArg.end(), back_inserter(argVec.back()));
                argVec.back().push_back('\0');
                args.push_back(argVec.back().data());

                argVec.emplace_back();
                auto localNumberArg = config.sgctLocal_;
                copy(localNumberArg.begin(), localNumberArg.end(), back_inserter(argVec.back()));
                argVec.back().push_back('\0');
                args.push_back(argVec.back().data());

                if (config.sgctSlave_) {
                    argVec.emplace_back(); argVec.back() = {'-', '-', 's', 'l', 'a', 'v', 'e', '\0'};
                    args.push_back(argVec.back().data());
                }
            }
        }
        auto argc = static_cast<int>(args.size());
        auto argv = args.data();
        auto engine = std::make_unique<sgct::Engine>(argc, argv);

        sgct::MessageHandler::instance()->setLogCallback(&SGCTLog);
        sgct::MessageHandler::instance()->setLogToCallback(true);

        auto node = std::make_unique<viscom::ApplicationNodeInternal>(std::move(config), std::move(engine));
        node->InitNode();

        return node;
    }
}
