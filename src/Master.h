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
			void init();	
			void destroy() { this->getInstance()->~Master(); }			
			void processState();
			sgct::SharedObject<Shared_Msg>* getMsgToSlave(){ return this->msgToSlave; }
			sgct::SharedVector<cv::Point2f>* getQuadCornersToSlave(){ return this->quadCornersToSlave; }
			sgct::SharedVector<cv::Point3f>* getTexCoordinatesToSlave(){ return this->texCoordinatesToSlave; }
			sgct::SharedVector<cv::Point2f>* getProjectorCorners_VPToSlave(){ return this->ProjectorCorners_VPToSlave; }
			sgct::SharedVector<cv::Point2f>* getProjectorCornersToSlave(){ return this->ProjectorCornersToSlave; }
			sgct::SharedVector<cv::Point2f>* getLocalLowHighVPToSlave(){ return this->LocalLowHighVPToSlave; }
			sgct::SharedVector<int>* getColorCalibDataToSlave(){ return this->colorCalibDataToSlave; }
			const int getCurrentState(){ return this->current_state; }
			int isNextStep(){ return this->nextStep; }
			void setNextStep(int r){ this->nextStep = r; }
			
		protected: 
			Master() {}
			~Master();
		private:
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
			sgct::SharedVector<cv::Point2f>* quadCornersToSlave;
			sgct::SharedVector<cv::Point3f>* texCoordinatesToSlave;
			sgct::SharedVector<cv::Point2f>* ProjectorCorners_VPToSlave;
			sgct::SharedVector<cv::Point2f>* ProjectorCornersToSlave;
			sgct::SharedVector<cv::Point2f>* LocalLowHighVPToSlave;
			sgct::SharedVector<int>* colorCalibDataToSlave;
			std::vector<Shared_Msg> msgFromSlaves;	
			
			void loadProperties();
			void loadAndSendDataToSlaves();
			void processMsgFromSlave(Message currCmd);
			void finish();
			void checkUserInput();
			bool checkIfSlavesRdy(int slaveID, int window, Message currCmd);
			void sendMsgToSlave(int slave, int window, Message cmd);
	};
}
#endif  /*__MASTER_H__*/