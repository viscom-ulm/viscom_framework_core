/**
* @file   TuioInputWrapper.h
* @author Tobias Badura <tobias.badura@uni-ulm.de>
* @date   2017.01.31
*
* @brief  Declaration of the TuioInputWrapper.
*/

#pragma once
#ifdef WITH_TUIO

#include <memory>
#include <TuioListener.h>

namespace TUIO {
    class TuioClient;
    class OscReceiver;
}

namespace viscom {
    class MasterNode;
}

class TuioInputWrapper : public TUIO::TuioListener {

public:
    TuioInputWrapper(int port, viscom::MasterNode* node);
    ~TuioInputWrapper();

    virtual void addTuioObject(TUIO::TuioObject *tobj) override;
    virtual void updateTuioObject(TUIO::TuioObject *tobj) override;
    virtual void removeTuioObject(TUIO::TuioObject *tobj) override;

    virtual void addTuioCursor(TUIO::TuioCursor *tcur) override;
    virtual void updateTuioCursor(TUIO::TuioCursor *tcur) override;
    virtual void removeTuioCursor(TUIO::TuioCursor *tcur) override;

    virtual void addTuioBlob(TUIO::TuioBlob *tblb) override;
    virtual void updateTuioBlob(TUIO::TuioBlob *tblb) override;
    virtual void removeTuioBlob(TUIO::TuioBlob *tblb) override;

    virtual void refresh(TUIO::TuioTime frameTime) override;

private:
    viscom::MasterNode* node_;
    std::unique_ptr<TUIO::TuioClient> tuioClient_;
    std::unique_ptr<TUIO::OscReceiver> receiver_;
};


#endif