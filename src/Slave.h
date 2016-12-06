#ifndef  __SLAVE_H__
#define  __SLAVE_H__

#include "main.h"
#include "ShareUtil.h"
#include "ColorCalib.h"
#include "ProjectorOverlapFramebuffer.h"

namespace pro_cal {
    class Slave {

        struct QuadGrid
        {
            glm::vec3 position;
            glm::vec4 color;

            QuadGrid(const glm::vec3& pos, const glm::vec4& col) : position(pos), color(col){}
        };

    public:
        static Slave* getInstance() { instance = instance == nullptr ? new Slave() : instance; return instance; }		
        void init(const viscom::FWConfiguration& config, sgct::Engine* engine);
        void destroy() { this->getInstance()->~Slave(); }
        void processCommand(sgct::Engine* gEngine, float currentTime);
        void checkMsgFromMaster(const Shared_Msg cmd);
        int getSlaveID(){ return this->slaveID; }
    protected:
        Slave() {}
        ~Slave();
    private:
        viscom::FWConfiguration config;
        sgct::Engine* engine;

        void LoadCalibrationData();

        bool initDataDone;
        bool *initFinalDone;
        bool *frustumupdate;
        static Slave *instance;
        int slaveID;
        int windowsCount;
        Shared_Msg respondedMsg;
        Shared_Msg renderCmd, oldrenderCmd;
        SOCKET ClientSocket;
        std::vector<cv::Point2f> *quad_corners;
        std::vector<cv::Point3f> *TexCoordinates;
        // Contains the ViewPlane-Coordinates of all slaves
        // in the order of 4 Points per slave
        std::vector<cv::Point2f> ProjectorCorners_VP;
        std::vector<cv::Point2f> ProjectorCorners_Wall;
        std::vector<cv::Point2f> *LocalLowHighVP;
        // std::vector<LookUpTableData> lookUpTableData;
        LookUpTableData lookUpTableData;
        glm::vec3 showColorValue;
        cv::Mat pic;
        std::string masterSocketPort;
        std::string masterSocketIP;
        void loadProperties();
        void updateFrustum(float VP_x1, float VP_y1, float VP_x2, float VP_y2, int window);
        void init_transform(sgct::Engine * gEngine, const std::vector<int> coords, const int window);
        void init_final(sgct::Engine * gEngine, const std::vector<int> coords, const int window);
        void show_final(sgct::Engine * gEngine, int window, float currentTime);

        void draw_scene(int window, float currentTime);

        void shutdown(int window);
        void sendMsgToMaster(const Shared_Msg msg);
        void calc_transform_coordinates(int window);

        bool *isTmpFBO_Init;

        bool *isVAO_FillingQuad3v3t_Init;
        bool *isVAO_Triangle_Scene_Init;
        bool *isVAO_TransformQuad_Init;
        bool *isVAO_BackgroundGrid_Init;

        bool isProg_Render1Tex_Init;
        bool isProg_Triangle_Scene_Init;
        bool isProg_R2Quad_Init;
        bool isProg_backgroundQuad_Init;

        bool *isTex_Alpha_Init;

        bool createdAruco;

        GLuint *tex_Alpha;
        GLuint *tex_AlphaTrans;
        GLuint *tex_ColorLookup;

        // newCode
        std::vector<glm::vec2> resolutionScale;

        std::vector<glm::vec3> *textureCoordinates;
        std::vector<glm::vec3> *vertexCoordinates;
        void calc_vert_points(int window);
        void calc_tex_points(int window);
        ProjectorOverlapFramebuffer* oFramebuffer;
        std::vector<glm::vec3> *projectorPoints;
        std::vector<int> *overlapingVerticesCounter;
        uint numberOfQuads;

        void initVAO_Triangle_Scene(int window);
        void initVAO_TransformQuad3v3t(int window);
        void initVAO_FillingQuad_3v3t(int window);
        void initVAO_BackgroundGrid_3v4c(int window);
        void initTmpFBO_1T(int window);
        void checkFrustumUpdate(int window);

        //for every window a framebuffer to be sure
        GLuint *fbo_tmpFBO, *depth_tmpFBO, *render_tmpFBO, *tex_tmpFBO;

        //some scenes
        GLuint *Matrix_Loc, *vao_Triangle_Scene, *vbo_Triangle_Scene, *sceneVertexColorBuffer;
        GLuint *vao_TransformQuad3v3t, *vbo_filling_quad;
        GLuint *vao_FillingQuad_3v3t, *vbo_fillingQuad_3v3t;
        GLuint *vao_backgroundGrid_3v4c, *vbo_backgroundGrid_3v4c;
        std::vector<QuadGrid> backgroundGrid;
        
        int alphaTransition;
        int blurRepetition;
        int blurRadius;
    };
}
#endif  /*__SLAVE_H__*/
