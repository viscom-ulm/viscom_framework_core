#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <array>
#include "sgct.h"
#include "main.h"
#include "Slave.h"
#include "Master.h"


using namespace pro_cal;

sgct::Engine * gEngine;

void myDrawFun();
void myPreSyncFun();
void myPostSyncPreDrawFun();
void myInitOGLFun();
void myCleanUpFun();
void myEncodeFun();
void myDecodeFun();
void keyCallback(int key, int action);

std::vector<fbData> buffers;
sgct_utils::SGCTBox * myBox = nullptr;

//variables to share across cluster
sgct::SharedDouble curr_time(0.0);

int argc_;
char **argv_;
FWConfiguration config;
// std::string sgctConfigFile;

int SGCT_Init() {
    config = LoadConfiguration();
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
    gEngine = new sgct::Engine(argc, argv);

    gEngine->setInitOGLFunction(myInitOGLFun);
    gEngine->setDrawFunction(myDrawFun);
    gEngine->setPreSyncFunction(myPreSyncFun);
    gEngine->setPostSyncPreDrawFunction(myPostSyncPreDrawFun);
    gEngine->setCleanUpFunction(myCleanUpFun);
    gEngine->setKeyboardCallbackFunction(keyCallback);

    if (!gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile))
    {
        delete gEngine;
        return EXIT_FAILURE;
    }

    sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
    sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);

    //Master and Slave
    Master::getInstance()->init(config);
    Slave::getInstance()->init(config);

    // Main loop
    gEngine->render();

    // Clean up
    delete gEngine;

    return EXIT_SUCCESS;
}

FWConfiguration LoadConfiguration()
{
    std::ifstream ifs("framework.cfg");
    if (!ifs.is_open()) {
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
/**
* assign the config parameter with the path to the sgct config file to the sgctConfigFile variable
*
* @param argc int variable from the main methode, count of the parameters for the program
* @param argv char* parameters for the program, pointer to an array of c strings 
*/
/*void getSgctConfigFileName(int argc, char* argv[]) {
    sgctConfigFile.clear();
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-config") == 0) {
            sgctConfigFile.assign(argv[i + 1]);
        }
    }
}*/

int main(int argc, char* argv[])
{
    // getSgctConfigFileName(argc, argv);
    argc_ = argc;
    argv_ = argv;
    SGCT_Init();

    // Exit program
    exit(EXIT_SUCCESS);
}

/**
* call for every frame on each node the processCommand methode from the slave class
*/
void myDrawFun()
{
    Slave::getInstance()->processCommand(gEngine);
}

/**
* Master only: calls the processState methode, where the states and user input is handled
*/
void myPreSyncFun()
{
    if (gEngine->isMaster())
    {
        curr_time.setVal(sgct::Engine::getTime());
        Master::getInstance()->processState();
    }
}

/**
* Master&Slave: call all check methods from the slave class, if the message or data is destined for this slave
*/
void myPostSyncPreDrawFun()
{
    //=>moved from myPreSyncFunc()
    Slave::getInstance()->checkMsgFromMaster(Master::getInstance()->getMsgToSlave()->getVal());
    Slave::getInstance()->checkQuadCornersFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getQuadCornersToSlave()->getVal());
    Slave::getInstance()->checkTexCoordinatesFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getTexCoordinatesToSlave()->getVal());
    Slave::getInstance()->checkLocalLowHighVPFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getLocalLowHighVPToSlave()->getVal());
    Slave::getInstance()->checkProjectorCorners_VPFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getProjectorCorners_VPToSlave()->getVal());
    Slave::getInstance()->checkProjectorCornersFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getProjectorCornersToSlave()->getVal());
    Slave::getInstance()->checkColorCalibDataFromMaster(Master::getInstance()->getMsgToSlave()->getVal(), Master::getInstance()->getColorCalibDataToSlave()->getVal());
}

void myInitOGLFun()
{
    // init 
}

void myCleanUpFun()
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
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getLocalLowHighVPToSlave());
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getQuadCornersToSlave());
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getTexCoordinatesToSlave());
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getProjectorCorners_VPToSlave());
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getProjectorCornersToSlave());
    sgct::SharedData::instance()->writeVector(Master::getInstance()->getColorCalibDataToSlave());
}

/**
* Nodes: read/decode all shared objectsfrom the master for the appropriated state
*/
void myDecodeFun()
{
    sgct::SharedData::instance()->readDouble(&curr_time);
    //read msgs and data from master to slave
    sgct::SharedData::instance()->readObj(Master::getInstance()->getMsgToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getLocalLowHighVPToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getQuadCornersToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getTexCoordinatesToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getProjectorCorners_VPToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getProjectorCornersToSlave());
    sgct::SharedData::instance()->readVector(Master::getInstance()->getColorCalibDataToSlave());
}

/**
* Master only: callback methode for user input on SGCT window (doesnt work if opencv window is focused!)
* change the next_step variable from the Master, to control if the next or previous step should be processed
*/
void keyCallback(int key, int action)
{
    if (gEngine->isMaster())
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
