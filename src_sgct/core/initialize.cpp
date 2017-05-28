/**
 * @file   initialize.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Implementation of functions needed to initialize the application.
 */

#include "core/initialize.h"
#include "core/main.h"
#include "core/ApplicationNodeInternal.h"
#include <sgct.h>

void SGCTLog(const char* msg)
{
    std::string msgStripped = msg;
    LOG(INFO) << msgStripped.substr(0, msgStripped.size() - 1);
}

namespace viscom {

    std::unique_ptr<ApplicationNodeInternal> Application_Init(const FWConfiguration& config) {
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
                    argVec.emplace_back(); argVec.back() = { '-', '-', 's', 'l', 'a', 'v', 'e', '\0' };
                    args.push_back(argVec.back().data());
                }
            }
        }
        auto argc = static_cast<int>(args.size());
        auto argv = args.data();
        auto engine = std::make_unique<sgct::Engine>(argc, argv);

        sgct::MessageHandler::instance()->setLogCallback(&SGCTLog);
        sgct::MessageHandler::instance()->setLogToCallback(true);

        auto internalConfig = config;
        auto node = std::make_unique<viscom::ApplicationNodeInternal>(std::move(internalConfig), std::move(engine));
        node->InitNode();

        return node;
    }
}
