/**
 * @file   TuioInputWrapper.cpp
 * @author Tobias Badura <tobias.badura@uni-ulm.de>
 * @date   2017.01.31
 *
 * @brief  implementation of the TuioInputWrapper.
 */

#include "TuioInputWrapper.h"

#if VISCOM_USE_TUIO
#include <TuioClient.h>
#include <UdpReceiver.h>
#endif


namespace viscom::tuio {

    TuioInputWrapper::TuioInputWrapper(int port)
    {
#if WITH_TUIO
        receiver_ = std::make_unique<TUIO::UdpReceiver>(port);

        tuioClient_ = std::make_unique<TUIO::TuioClient>(receiver_.get());
        tuioClient_->addTuioListener(this);
        tuioClient_->connect();
        tuioClient_->disconnect();
#endif
    }

    TuioInputWrapper::~TuioInputWrapper()
    {
#if WITH_TUIO
        tuioClient_->disconnect();
#endif
    }


#if WITH_TUIO
    void TuioInputWrapper::addTuioCursor(TUIO::TuioCursor *tcur)
    {
        std::cout << "add    cur  " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;
    }

    void TuioInputWrapper::updateTuioCursor(TUIO::TuioCursor *tcur)
    {
        std::cout << "set    cur  " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ") " << tcur->getX() << " " << tcur->getY()
            << " " << tcur->getMotionSpeed() << " " << tcur->getMotionAccel() << " " << std::endl;
    }

    void TuioInputWrapper::removeTuioCursor(TUIO::TuioCursor *tcur)
    {
        std::cout << "delete cur  " << tcur->getCursorID() << " (" << tcur->getSessionID() << "/" << tcur->getTuioSourceID() << ")" << std::endl;
    }

    void TuioInputWrapper::addTuioObject(TUIO::TuioObject *tobj)
    {
        std::cout << "add    obj  " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ") " << tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() << std::endl;
    }

    void TuioInputWrapper::updateTuioObject(TUIO::TuioObject *tobj)
    {
        std::cout << "set    obj  " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ") " << tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle()
            << " " << tobj->getMotionSpeed() << " " << tobj->getRotationSpeed() << " " << tobj->getMotionAccel() << " " << tobj->getRotationAccel() << std::endl;
    }

    void TuioInputWrapper::removeTuioObject(TUIO::TuioObject *tobj)
    {
        std::cout << "TUIO delete obj  " << tobj->getSymbolID() << " (" << tobj->getSessionID() << "/" << tobj->getTuioSourceID() << ")" << std::endl;
    }

    void TuioInputWrapper::addTuioBlob(TUIO::TuioBlob *tblb)
    {
        std::cout << "TUIO add    blob " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ") " << tblb->getX() << " " << tblb->getY() << " " << tblb->getAngle() << " " << tblb->getWidth() << " " << tblb->getHeight() << " " << tblb->getArea() << std::endl;
    }

    void TuioInputWrapper::updateTuioBlob(TUIO::TuioBlob *tblb)
    {
        std::cout << "TUIO set    blob " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ") " << tblb->getX() << " " << tblb->getY() << " " << tblb->getAngle() << " " << tblb->getWidth() << " " << tblb->getHeight() << " " << tblb->getArea()
            << " " << tblb->getMotionSpeed() << " " << tblb->getRotationSpeed() << " " << tblb->getMotionAccel() << " " << tblb->getRotationAccel() << std::endl;
    }

    void TuioInputWrapper::removeTuioBlob(TUIO::TuioBlob *tblb)
    {
        std::cout << "TUIO delete blob " << tblb->getBlobID() << " (" << tblb->getSessionID() << "/" << tblb->getTuioSourceID() << ")" << std::endl;
    }

    void  TuioInputWrapper::refresh(TUIO::TuioTime frameTime)
    {
        std::cout << "TUIO refresh " << frameTime.getTotalMilliseconds() << std::endl;
    }
#endif
}
