#ifndef  __MASTER_H__
#define  __MASTER_H__

#include "ShareUtil.h"
#include "ColorCalib.h"
#include "main.h"

#include <iostream>
#include <fstream>
#include <string>

namespace pro_cal {
    class Master {
        public:
            static Master* getInstance() { instance = instance == nullptr ? new Master() : instance; return instance; }
            void init(const FWConfiguration& config);
            void destroy() { this->getInstance()->~Master(); }
            void processState();
            sgct::SharedObject<Shared_Msg>* getMsgToSlave(){ return this->msgToSlave; }
            const int getCurrentState(){ return this->current_state; }
            int isNextStep(){ return this->nextStep; }
            void setNextStep(int r){ this->nextStep = r; }
            
        protected: 
            Master() {}
            ~Master();
        private:
            FWConfiguration config;

            //variables
            static Master *instance;
            std::string masterSocketPort;
            int slave_count;
            int projector_count;
            int current_state;
            int current_slave;
            int current_window;
            int current_color;
            int nextStep;
            
            SOCKET ServerSocket;
            SOCKET ClientSocket;	
            
            sgct::SharedObject<Shared_Msg>* msgToSlave;
            std::vector<Shared_Msg> msgFromSlaves;
            
            void loadProperties();
            void processMsgFromSlave(Message currCmd);
            void finish();
            void checkUserInput();
            bool checkIfSlavesRdy(int slaveID, int window, Message currCmd);
            void sendMsgToSlave(int slave, int window, Message cmd);
    };
}
#endif  /*__MASTER_H__*/