
#ifndef  __MAIN_H__
#define  __MAIN_H__

extern sgct::Engine * gEngine;

//FBO-stuff
struct fbData
{
    unsigned int texture;
    unsigned int fbo;
    unsigned int renderBuffer;
    unsigned int depthBuffer;
    int width;
    int height;
};

struct FWConfiguration
{
    std::string baseDirectory_;
    std::string programProperties_;
    std::string sgctConfig_;
    std::string projectorData_;
    std::string colorCalibrationData_;
    std::string sgctLocal_;
};

extern std::vector<fbData> buffers;
extern sgct_utils::SGCTBox * myBox;

//variables to share across cluster
extern sgct::SharedDouble curr_time;

// extern void drawScene();
FWConfiguration LoadConfiguration();

int SGCT_Init();

extern bool SGCT_Restart;

#endif  /*__MAIN_H__*/
