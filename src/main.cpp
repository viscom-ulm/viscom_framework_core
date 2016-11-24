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

#include "Slave.h"
#include "Master.h"


FWConfiguration LoadConfiguration();
std::unique_ptr<sgct::Engine> SGCT_Init(FWConfiguration);
int SGCT_Cleanup(sgct::Engine*);
void myDrawFun(sgct::Engine*);
void myPreSyncFun(sgct::Engine*);
void myPostSyncPreDrawFun(sgct::Engine*);
void myInitOGLFun(sgct::Engine*);
void myCleanUpFun(sgct::Engine*);
void myEncodeFun();
void myDecodeFun();
void keyCallback(sgct::Engine*, int, int);


void SGCTLog(const char* msg)
{
    std::string msgStripped = msg;
    LOG(INFO) << msgStripped.substr(0, msgStripped.size() - 1);
}


int main(int, char**)
{
    const std::string directory = "./";
    const std::string name = "viscomlabfw.log";
    auto worker = g3::LogWorker::createLogWorker();
    auto handle = worker->addSink(std2::make_unique<vku::FileSink>(name, directory, false), &vku::FileSink::fileWrite);

    g3::only_change_at_initialization::setLogLevel(WARNING, false);

#ifndef NDEBUG
    g3::only_change_at_initialization::setLogLevel(WARNING, true);
#endif

    g3::initializeLogging(worker.get());

    LOG(INFO) << "Log created.";

    auto config = LoadConfiguration();

    auto engine = SGCT_Init(config);


    //Master and Slave
    if (engine->isMaster()) pro_cal::Master::getInstance()->init(config);
    pro_cal::Slave::getInstance()->init(config, engine.get());

    // Main loop
    engine->render();

    return SGCT_Cleanup(engine.get());
}


FWConfiguration LoadConfiguration()
{
    LOG(INFO) << "Loading configuration.";
    std::ifstream ifs("framework.cfg");
    if (!ifs.is_open()) {
        LOG(INFO) << "Could not open config file (framework.cfg)." << std::endl;
        std::cout << "Could not open config file (framework.cfg)." << std::endl;
        throw std::runtime_error("Could not open config file (framework.cfg).");
    }

    FWConfiguration config;
    std::string str;

    while (ifs >> str && ifs.good()) {
        if (str == "BASE_DIR=") ifs >> config.baseDirectory_;
        else if (str == "PROGRAM_PROPERTIES=") ifs >> config.programProperties_;
        else if (str == "SGCT_CONFIG=") ifs >> config.sgctConfig_;
        else if (str == "PROJECTOR_DATA=") ifs >> config.projectorData_;
        else if (str == "COLOR_CALIBRATION_DATA=") ifs >> config.colorCalibrationData_;
        else if (str == "LOCAL=") ifs >> config.sgctLocal_;
    }
    ifs.close();

    return config;
}


std::unique_ptr<sgct::Engine> SGCT_Init(FWConfiguration config) {
    std::array<std::vector<char>, 2> argVec;
    std::array<char*, 2> args;

    {
        auto configArg = R"(-config ")" + config.sgctConfig_ + R"(")";
        auto localArg = "-local " + config.sgctLocal_;
        std::copy(configArg.begin(), configArg.end(), std::back_inserter(argVec[0]));
        std::copy(configArg.begin(), configArg.end(), std::back_inserter(argVec[1]));
        args[0] = argVec[0].data();
        args[1] = argVec[1].data();
    }
    auto argc = static_cast<int>(args.size());
    auto argv = args.data();
    auto engine = std::make_unique<sgct::Engine>(argc, argv);

    sgct::MessageHandler::instance()->setLogCallback(&SGCTLog);
    sgct::MessageHandler::instance()->setLogToCallback(true);


    engine->setInitOGLFunction([engine = engine.get()](){ myInitOGLFun(engine); });
    engine->setDrawFunction([engine = engine.get()](){ myDrawFun(engine); });
    engine->setPreSyncFunction([engine = engine.get()](){ myPreSyncFun(engine); });
    engine->setPostSyncPreDrawFunction([engine = engine.get()](){ myPostSyncPreDrawFun(engine); });
    engine->setCleanUpFunction([engine = engine.get()](){ myCleanUpFun(engine); });
    engine->setKeyboardCallbackFunction([engine = engine.get()](int key, int action){ keyCallback(engine, key, action); });

    if (!engine->init(sgct::Engine::OpenGL_3_3_Core_Profile))
    {
        LOG(FATAL) << "Failed to create SGCT engine.";
        throw std::runtime_error("Failed to create SGCT engine.");
    }

    sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
    sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);

    return engine;
}


int SGCT_Cleanup(sgct::Engine*)
{
    return 0;
}



using namespace pro_cal;


//variables to share across cluster
sgct::SharedDouble curr_time(0.0);

/**
* call for every frame on each node the processCommand method from the slave class
*/
void myDrawFun(sgct::Engine* engine)
{
    Slave::getInstance()->processCommand(engine, static_cast<float>(curr_time.getVal()));
}

/**
* Master only: calls the processState method, where the states and user input is handled
*/
void myPreSyncFun(sgct::Engine* engine)
{
    if (engine->isMaster())
    {
        curr_time.setVal(sgct::Engine::getTime());
        Master::getInstance()->processState();
    }
}

/**
* Master&Slave: call all check methods from the slave class, if the message or data is destined for this slave
*/
void myPostSyncPreDrawFun(sgct::Engine* engine)
{
    //=>moved from myPreSyncFunc()
    Slave::getInstance()->checkMsgFromMaster(Master::getInstance()->getMsgToSlave()->getVal());
}

void myInitOGLFun(sgct::Engine* engine)
{
    // init 
}

void myCleanUpFun(sgct::Engine* engine)
{
    Slave::getInstance()->destroy();
    Master::getInstance()->destroy();
}


/**
* Master only: write/encode all shared objects to send it to the slaves for the appropriated state
*/
void myEncodeFun()
{
    sgct::SharedData::instance()->writeDouble(&curr_time);
    //write msgs and data from master to slave
    sgct::SharedData::instance()->writeObj(Master::getInstance()->getMsgToSlave());
}

/**
* Nodes: read/decode all shared objects from the master for the appropriated state
*/
void myDecodeFun()
{
    sgct::SharedData::instance()->readDouble(&curr_time);
    //read msgs and data from master to slave
    sgct::SharedData::instance()->readObj(Master::getInstance()->getMsgToSlave());
}

/**
* Master only: callback method for user input on SGCT window (does not work if OpenCV window is focused!)
* change the next_step variable from the Master, to control if the next or previous step should be processed
*/
void keyCallback(sgct::Engine* engine, int key, int action)
{
    if (engine->isMaster())
    {
        switch (key)
        {
        case SGCT_KEY_UP:
            if (action == SGCT_PRESS) {
                Master::getInstance()->setNextStep(1);
            }
            break;
        case SGCT_KEY_DOWN:
            if (action == SGCT_PRESS) {
                Master::getInstance()->setNextStep(-1);
            }
            break;
        default: break;
        }
    }
}
