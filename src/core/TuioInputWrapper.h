/**
* @file   TuioInputWrapper.h
* @author Tobias Badura <tobias.badura@uni-ulm.de>
* @date   2017.01.31
*
* @brief  Declaration of the TuioInputWrapper.
*/

#pragma once
#ifdef WITH_TUIO

#include "TuioListener.h"
#include "TuioClient.h"
#include "UdpReceiver.h"

using namespace TUIO;

namespace viscom {
    class MasterNode;
}

class TuioInputWrapper : public TuioListener {

public:
	TuioInputWrapper(int port, viscom::MasterNode* node);

	void addTuioObject(TuioObject *tobj);
	void updateTuioObject(TuioObject *tobj);
	void removeTuioObject(TuioObject *tobj);

	void addTuioCursor(TuioCursor *tcur);
	void updateTuioCursor(TuioCursor *tcur);
	void removeTuioCursor(TuioCursor *tcur);

	void addTuioBlob(TuioBlob *tblb);
	void updateTuioBlob(TuioBlob *tblb);
	void removeTuioBlob(TuioBlob *tblb);

	void refresh(TuioTime frameTime);

private:
    viscom::MasterNode* node_;
	TuioClient* tuioClient_;
	OscReceiver* receiver_;
};


#endif