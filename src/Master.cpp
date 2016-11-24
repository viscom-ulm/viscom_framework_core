#include "Master.h"
#include "SocketHandler.h"

//if not defined => old Data will be deleted and new Data will be saved
#define LOAD_DATA 
#define LOAD_COLOR_DATA

namespace pro_cal {
    Master* Master::instance = nullptr;
    
    Master::~Master() // destructor
    {
        delete msgToSlave;
    }

    /**
    * loads the properties from config\properties.xml file
    * masterSocketPort: master socket server port
    */
    void Master::loadProperties()
    {
        cv::FileStorage fs(config.programProperties_, cv::FileStorage::READ);
        if(fs.isOpened()){
            this->masterSocketPort = fs["masterSocketPort"];
            std::string sqrtCellCount = fs["sqrtCellCount"];
            CELL_COUNT = sqrtCellCount.empty() ? 10 : atoi(sqrtCellCount.c_str());
            std::string startnode = fs["startNode"];
            START_NODE = startnode.empty() ? 1 : atoi(startnode.c_str());
        }
        else {
            showMsgToUser("config\\properties.xml not found on master", 0);
        }
        fs.release();
    }
    
    /**
    * calls loadProperties() methode, initialize all private member variables
    * and start socket server to receive messages from slaves
    */
    void Master::init(const FWConfiguration& conf) {
        config = conf;
        loadProperties();

        current_state = 0; 
        current_slave = START_NODE;
        current_window = 0;
        nextStep = 0;
        current_color = 0;
        slave_count = static_cast<int>(sgct_core::ClusterManager::instance()->getNumberOfNodes());
        //calc projector count
        projector_count = -current_slave;
        for (int i = 0; i < slave_count; i++) {
            sgct_core::SGCTNode * currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
            for (int j = 0; j < currNode->getNumberOfWindows(); j++) {
                projector_count += 1;
            }
        }
    
        
        msgToSlave = new sgct::SharedObject<Shared_Msg>();
        /*msgFromSlaves = std::vector<Shared_Msg>(slave_count);
        for (int i = 0; i < slave_count; i++) {
            msgFromSlaves[i].slaveID = i;
            msgFromSlaves[i].window = 255;
            msgFromSlaves[i].msg = NONE;
        }*/
        
        if (sgct_core::ClusterManager::instance()->getThisNodeId() == 0) {
            ServerSocket = INVALID_SOCKET;
            startSocketServer(ServerSocket, sgct_core::ClusterManager::instance()->getMasterAddress()->c_str(), masterSocketPort.c_str());
        }
    }

    /**
    * receives socket message from a slave and put them in msgFromSlaves-vector
    * if the message is an answer to the current command
    *
    * @param currCmd current command which was send to the slave
    */
    /*void Master::processMsgFromSlave(Message currCmd) {
        if (checkAndStartServerSocket(ServerSocket, sgct_core::ClusterManager::instance()->getMasterAddress()->c_str(), masterSocketPort.c_str())) {
            std::string recMsg;
            int result = receiveSocketMsg(ServerSocket, recMsg);
            if (result > 0) {
                Shared_Msg recmsg = stringToSharedMsg(recMsg);
                if (recmsg.slaveID >= 0 && recmsg.slaveID < slave_count && recmsg.msg == currCmd) {
                    msgFromSlaves[recmsg.slaveID].msg = recmsg.msg;
                    msgFromSlaves[recmsg.slaveID].window = recmsg.window;
                    showMsgToUser("Message from node %d received: " + MessageToString(recmsg.msg), recmsg.slaveID);
                }
            }				
        }		
    }*/

    void Master::checkUserInput(){
        if (nextStep > 0) {
            current_state += 1;
        }
        if (nextStep < 0) {
            current_state -= 1;
        }
        nextStep = 0;
    }

    /**
    * controlls the calibration state switching and calls the corresponding methodes
    * state 0: send SHOW_FINAL command to all slaves
    * state 1: call finish() methode
    */
    void Master::processState() {
        switch (current_state) {
        case 0:
            sendMsgToSlave(ALL, ALL, SHOW_FINAL);
            current_state += 1;
            break;
        case 1:
            checkUserInput();
            break;
        case 2:
            finish();
            break;
        default: break;
        }
    }


    /**
    * check if all slaves answered to the SHUTDOWN command for all projector
    * if slaves are ready then this methode handles the user input to change current state
    * if the user input is KEY_UP, then the server and client socket will be closed
    * if the user input is KEY_DOWN, then the state goes back one step
    */
    void Master::finish(){
        /*bool slavesRdy = checkIfSlavesRdy(ALL, ALL, SHUTDOWN);
        if (!slavesRdy) {
            processMsgFromSlave(SHUTDOWN);
        }*/
        if (nextStep > 0) {
            sendMsgToSlave(ALL, ALL, NONE);
            closeSocket(ServerSocket);
            closeSocket(ClientSocket);
            current_state += 1;
        }
        if (nextStep < 0) {
            current_state -= 1;
        }
        nextStep = 0;
    }

    /**
    * Check if one or all slaves responded for the current render command
    * all slave who responded the current render command are ready
    * 
    * @param slaveID which slave is checked for the response message
    * @param current_window which target projector from the slave is checked
    * @param currCmd the command message what the slave have to show next
    * @return boolean true if one or all slaves are ready, otherwise return false
    */
    /*bool Master::checkIfSlavesRdy(int slaveID, int current_window, Message currCmd) {
        if (slaveID == ALL) {
            for (Shared_Msg resp : msgFromSlaves) {
                if (resp.msg != currCmd || (current_window != ALL && resp.window != current_window)) {
                    return false;
                }
            }
            return true;
        }
        else {
            return msgFromSlaves[slaveID].msg == currCmd && msgFromSlaves[slaveID].window == current_window;
        }		
    }*/

    /**
    * Share a message/command with one or all slaves for one or all windows
    *
    * @param slave determine to which slave the message is send to
    * @param window determine the target projector from the slave
    * @param cmd the command message what the slave have to show next
    */
    void Master::sendMsgToSlave(int slave, int window, Message cmd) {
        Shared_Msg msg;
        msg.msg = cmd;
        msg.slaveID = slave;
        msg.window = window;
        this->msgToSlave->setVal(msg);
        showMsgToUser("Message to node %d sent: " + MessageToString(cmd), slave);
    }


}
