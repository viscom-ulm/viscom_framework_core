/**
* @file   TuioInputWrapper.h
* @author Tobias Badura <tobias.badura@uni-ulm.de>
* @date   2017.01.31
*
* @brief  Declaration of the TuioInputWrapper.
*/

#pragma once

#include "core/main.h"

#ifdef VISCOM_USE_TUIO
#include <memory>
#include <TuioListener.h>

#else

namespace TUIO {
    using TuioCursor = void;
    using TuioObject = void;
    using TuioBlob = void;
    using TuioTime = int;
}

#endif

namespace TUIO {
    class TuioClient;
    class OscReceiver;
}

namespace viscom::tuio {

#ifdef VISCOM_USE_TUIO

    class TuioInputWrapper : public TUIO::TuioListener
    {
    public:
        TuioInputWrapper(int port);
        virtual ~TuioInputWrapper() override;

        /**
         *  Sets the callback function called when adding a cursor.
         *  @param fn the add cursor callback function to be used.
         */
        void SetAddCursorCallback(std::function<void(TUIO::TuioCursor*)> fn);
        /**
         *  Sets the callback function called when updating a cursor.
         *  @param fn the update cursor callback function to be used.
         */
        void SetUpdateCursorCallback(std::function<void(TUIO::TuioCursor*)> fn);
        /**
         *  Sets the callback function called when removing a cursor.
         *  @param fn the remove cursor callback function to be used.
         */
        void SetRemoveCursorCallback(std::function<void(TUIO::TuioCursor*)> fn);

        /**
         *  Adds a cursor to the touch screen.
         *  @param tcur cursor to be added.
         */
        void addTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Updates a cursor.
         *  @param tcur cursor to be updated.
         */
        void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Removes a cursor from the touch screen.
         *  @param tcur cursor to be removed.
         */
        void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        //////////////////////////////////////////////////////////////////////////
        // These methods are currently not used, so do not override them.
        virtual void addTuioObject(TUIO::TuioObject *tobj) override;
        virtual void updateTuioObject(TUIO::TuioObject *tobj) override;
        virtual void removeTuioObject(TUIO::TuioObject *tobj) override;

        virtual void addTuioBlob(TUIO::TuioBlob *tblb) override;
        virtual void updateTuioBlob(TUIO::TuioBlob *tblb) override;
        virtual void removeTuioBlob(TUIO::TuioBlob *tblb) override;

        virtual void refresh(TUIO::TuioTime frameTime) override;
        //////////////////////////////////////////////////////////////////////////

    private:
        std::unique_ptr<TUIO::TuioClient> tuioClient_;
        std::unique_ptr<TUIO::OscReceiver> receiver_;
        std::function<void(TUIO::TuioCursor*)> addCursorFn_;
        std::function<void(TUIO::TuioCursor*)> updateCursorFn_;
        std::function<void(TUIO::TuioCursor*)> removeCursorFn_;
    };

#else

    class TuioInputWrapper
    {
    public:
        TuioInputWrapper(int port);
        virtual ~TuioInputWrapper();

        void SetAddCursorCallback(std::function<void(TUIO::TuioCursor*)> fn) {}
        void SetUpdateCursorCallback(std::function<void(TUIO::TuioCursor*)> fn) {}
        void SetRemoveCursorCallback(std::function<void(TUIO::TuioCursor*)> fn) {}

        virtual void addTuioCursor(TUIO::TuioCursor *tcur) {}
        virtual void updateTuioCursor(TUIO::TuioCursor *tcur) {}
        virtual void removeTuioCursor(TUIO::TuioCursor *tcur) {}

        virtual void addTuioObject(TUIO::TuioObject *tobj) {}
        virtual void updateTuioObject(TUIO::TuioObject *tobj) {}
        virtual void removeTuioObject(TUIO::TuioObject *tobj) {}

        virtual void addTuioBlob(TUIO::TuioBlob *tblb) {}
        virtual void updateTuioBlob(TUIO::TuioBlob *tblb) {}
        virtual void removeTuioBlob(TUIO::TuioBlob *tblb) {}

        virtual void refresh(TUIO::TuioTime frameTime) {}
    };

#endif
}
