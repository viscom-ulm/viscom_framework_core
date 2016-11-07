
#ifndef  __MAIN_H__
#define  __MAIN_H__

#include "rapidxml-1.13\rapidxml.hpp"
#include "rapidxml-1.13\rapidxml_iterators.hpp"
#include "rapidxml-1.13\rapidxml_print.hpp"
#include "rapidxml-1.13\rapidxml_utils.hpp"

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

extern std::vector<fbData> buffers;
extern sgct_utils::SGCTBox * myBox;

//variables to share across cluster
extern sgct::SharedDouble curr_time;

extern void drawScene();

int SGCT_Init();

extern bool SGCT_Restart;

extern std::string sgctConfigFile;
void getSgctConfigFileName(int argc, char* argv[]);

#endif  /*__MAIN_H__*/
