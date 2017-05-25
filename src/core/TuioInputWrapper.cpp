/**
* @file   TuioInputWrapper.cpp
* @author Tobias Badura <tobias.badura@uni-ulm.de>
* @date   2017.01.31
*
* @brief  implementation of the TuioInputWrapper.
*/

#if WITH_TUIO

#include "TuioInputWrapper.h"

TuioInputWrapper::TuioInputWrapper(int port, viscom::MasterNode* node) :
    node_ { node }
{
	receiver_ = new UdpReceiver(port);

	tuioClient_ = new TuioClient(receiver_);
	tuioClient_->addTuioListener(this);
	tuioClient_->connect();
}

void TuioInputWrapper::addTuioCursor(TuioCursor *tcur) {
	std::cout << "add cur " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;
}

void TuioInputWrapper::updateTuioCursor(TuioCursor *tcur) {
	std::cout << "set cur " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ") " << tcur->getX() << " " << tcur->getY()
		<< " " << tcur->getMotionSpeed() << " " << tcur->getMotionAccel() << " " << std::endl;
}

void TuioInputWrapper::removeTuioCursor(TuioCursor *tcur) {
	std::cout << "del cur " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ")" << std::endl;
}








/**
	The following functions are for other input hardware

*/



void TuioInputWrapper::addTuioObject(TuioObject *tobj) {
	std::cout << "add obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ") " << tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() << std::endl;
}

void TuioInputWrapper::updateTuioObject(TuioObject *tobj) {
	std::cout << "set obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ") " << tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle()
		<< " " << tobj->getMotionSpeed() << " " << tobj->getRotationSpeed() << " " << tobj->getMotionAccel() << " " << tobj->getRotationAccel() << std::endl;
}

void TuioInputWrapper::removeTuioObject(TuioObject *tobj) {
	std::cout << "del obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ")" << std::endl;
}

void TuioInputWrapper::addTuioBlob(TuioBlob *tblb) {
	std::cout << "add blb " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ") " << tblb->getX() << " " << tblb->getY() << " " << tblb->getAngle() << " " << tblb->getWidth() << " " << tblb->getHeight() << " " << tblb->getArea() << std::endl;
}

void TuioInputWrapper::updateTuioBlob(TuioBlob *tblb) {
	std::cout << "set blb " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ") " << tblb->getX() << " " << tblb->getY() << " " << tblb->getAngle() << " " << tblb->getWidth() << " " << tblb->getHeight() << " " << tblb->getArea()
		<< " " << tblb->getMotionSpeed() << " " << tblb->getRotationSpeed() << " " << tblb->getMotionAccel() << " " << tblb->getRotationAccel() << std::endl;
}

void TuioInputWrapper::removeTuioBlob(TuioBlob *tblb) {
	std::cout << "del blb " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ")" << std::endl;
}

void  TuioInputWrapper::refresh(TuioTime frameTime) {
	//std::cout << "refresh " << frameTime.getTotalMilliseconds() << std::endl;
}


#endif
