/**
 * @file   ApplicationNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "main.h"
#include "sgct.h"
#include <mutex>

namespace viscom {

    class ApplicationNodeImplementation;

    class ApplicationNode
    {
    public:
        ApplicationNode(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine);
        ApplicationNode(const ApplicationNode&) = delete;
        ApplicationNode(ApplicationNode&&) = delete;
        ApplicationNode& operator=(const ApplicationNode&) = delete;
        ApplicationNode& operator=(ApplicationNode&&) = delete;
        virtual ~ApplicationNode();

        void InitNode();
        void Render() const;

        void BasePreWindow();
        void BaseInitOpenGL() const;
        void BasePreSync();
        void PostSyncFunction();
        void BaseClearBuffer() const;
        void BaseDrawFrame() const;
        void BaseDraw2D() const;
        void BasePostDraw() const;
        void BaseCleanUp() const;

        static void BaseEncodeDataStatic();
        static void BaseDecodeDataStatic();
        void BaseEncodeData();
        void BaseDecodeData();

        sgct::Engine* GetEngine() const { return engine_.get(); }
        const FWConfiguration& GetConfig() const { return config_; }
        unsigned int GetGlobalProjectorId(int nodeId, int windowId) const;

    private:
        void loadProperties();

        /** Holds a static pointer to an object to this class making it singleton in a way. */
        // TODO: This is only a workaround and should be fixed in the future. [12/5/2016 Sebastian Maisch]
        static ApplicationNode* instance_;
        /** Holds the mutex for the instance pointer. */
        static std::mutex instanceMutex_;

        /** Holds the applications configuration. */
        FWConfiguration config_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeImplementation> appNodeImpl_;
        /** Holds the SGCT engine. */
        std::unique_ptr<sgct::Engine> engine_;
        /** Holds the start node used for slaves. */
        // TODO: Why is this needed? [11/25/2016 Sebastian Maisch]
        unsigned int startNode_;
        /** Holds the masters port. */
        std::string masterSocketPort_;

        /** Holds the synchronized application time. */
        sgct::SharedDouble currentTimeSynced_;
        /** Holds the current application time. */
        double currentTime_;

        /*int slave_count;

        SOCKET ServerSocket;
        SOCKET ClientSocket;*/

        // sgct::SharedObject<Shared_Msg>* msgToSlave;
        // std::vector<Shared_Msg> msgFromSlaves;
    };
}
