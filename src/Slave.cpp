#include "Slave.h"
#include "SocketHandler.h"
#include <stddef.h>

namespace pro_cal {
    Slave* Slave::instance = nullptr;

    Slave::~Slave() // destructor
    {
        delete[] initFinalDone;
        delete[] frustumupdate;
        delete[] quad_corners;
        delete[] TexCoordinates;	
        delete[] LocalLowHighVP;
        
        delete[] isTmpFBO_Init;

        delete[] isVAO_FillingQuad3v3t_Init;
        delete[] isVAO_Triangle_Scene_Init;
        delete[] isVAO_TransformQuad_Init;
        delete[] isVAO_BackgroundGrid_Init;
        delete[] isTex_Alpha_Init;
        delete[] tex_Alpha;
        delete[] tex_AlphaTrans;
        delete[] tex_ColorLookup;
        delete[] textureCoordinates;
        delete[] vertexCoordinates;		
        delete[] projectorPoints;
        delete[] overlapingVerticesCounter;
        delete[] fbo_tmpFBO;
        delete[] depth_tmpFBO;
        delete[] tex_tmpFBO;
        delete[] Matrix_Loc; 
        delete[] vao_Triangle_Scene; 
        delete[] vbo_Triangle_Scene; 
        delete[] sceneVertexColorBuffer;
        delete[] vao_TransformQuad3v3t; 
        delete[] vbo_filling_quad;
        delete[] vao_FillingQuad_3v3t; 
        delete[] vbo_fillingQuad_3v3t;
        delete[] vao_backgroundGrid_3v4c; 
        delete[] vbo_backgroundGrid_3v4c;

        backgroundGrid.clear();
        masterSocketPort.clear();
        masterSocketIP.clear();
        ProjectorCorners_VP.clear();
        ProjectorCorners_Wall.clear();
    }

    void Slave::LoadCalibrationData()
    {
        for (int window = 0; window < windowsCount; window++) {
            int projectorNo = getProjectorNo(slaveID, window);
            readVectorOfPoint2f(config.projectorData_, "quad_corners" + std::to_string(projectorNo), quad_corners[window]);
            readVectorOfPoint3f(config.projectorData_, "TexCoordinates" + std::to_string(projectorNo), TexCoordinates[window]);
            readVectorOfPoint2f(config.projectorData_, "LocalLowHighVP" + std::to_string(projectorNo), LocalLowHighVP[window]);

            /*std::vector<ColorCalibData> colorCalibData(windowsCount); // (projector_count);
            loadColorCalibDataSingle(config, projectorNo, colorCalibData[window]);
            lookUpTableData = calcColorLookUpTableDataSingle(colorCalibData[window]);*/
            lookUpTableData = calcColorLookUpTableDataSingle(config, projectorNo);
        }

        readVectorOfPoint2f(config.projectorData_, "ViewPlaneCoordinates", ProjectorCorners_VP);
        readVectorOfPoint2f(config.projectorData_, "projectorCorners", ProjectorCorners_Wall);



        // TODO: load color calib data for current projector only. [11/23/2016 Sebastian Maisch]
        /*auto slave_count = static_cast<int>(sgct_core::ClusterManager::instance()->getNumberOfNodes());
        //calc projector count
        int projector_count = -START_NODE;
        for (int i = 0; i < slave_count; i++) {
            sgct_core::SGCTNode * currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
            for (int j = 0; j < currNode->getNumberOfWindows(); j++) {
                projector_count += 1;
            }
        }

        std::vector<ColorCalibData> colorCalibData;
        loadColorCalibData(config, colorCalibData);
        lookUpTableData = calcColorLookUpTableData(colorCalibData);*/
    }

    /**
    * loads the properties from config\properties.xml file
    * masterSocketPort: master socket server port
    */
    void Slave::loadProperties()
    {
        cv::FileStorage fs(config.programProperties_, cv::FileStorage::READ);
        if (fs.isOpened()){
            std::string tmp;
            this->masterSocketPort = fs["masterSocketPort"];
            tmp = fs["alphaTransition"];
            this->alphaTransition = tmp.empty() ? 0 : atoi(tmp.c_str());
            tmp = fs["blurRepetition"];
            this->blurRepetition = tmp.empty() ? 0 : atoi(tmp.c_str());
            tmp = fs["blurRadius"];
            this->blurRadius = tmp.empty() ? 0 : atoi(tmp.c_str());
            std::string startnode = fs["startNode"];
            START_NODE = startnode.empty() ? 1 : atoi(startnode.c_str());
        }
        else {
            showMsgToUser("config\\properties.xml not found on slave %d", slaveID);
        }
        fs.release();
    }

    /**
    * calls loadProperties() methode, initialize all private member variables
    * and start socket client to send messages to the master.
    *
    * Loads all Shaders and binds them.
    * Generates for every variable an array with size of windows.
    *
    * Creates all needed elements for rendering
    */
    void Slave::init(const viscom::FWConfiguration& conf, sgct::Engine* eng) {
        this->slaveID = sgct_core::ClusterManager::instance()->getThisNodeId();
        config = conf;
        engine = eng;

        masterSocketIP.assign(sgct_core::ClusterManager::instance()->getMasterAddress()->c_str());
        loadProperties();

        ClientSocket = INVALID_SOCKET;
        startSocketClient(ClientSocket, masterSocketIP.c_str(), masterSocketPort.c_str(), IPPROTO_UDP);

        const auto SHADER_PATH = config.baseDirectory_ + "/shader/";
        if (!sgct::ShaderManager::instance()->shaderProgramExists("yform"))
        {
            sgct::ShaderManager::instance()->addShaderProgram("yform", SHADER_PATH + "SimpleVertexShader.vertexshader", SHADER_PATH + "ultrasimpleFragmentShader.fragmentshader");
            sgct::ShaderManager::instance()->bindShaderProgram("yform");
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }

        if (!sgct::ShaderManager::instance()->shaderProgramExists("OpenGL3R2T")){
            sgct::ShaderManager::instance()->addShaderProgram("OpenGL3R2T", SHADER_PATH + "simpleOgl3.vert", SHADER_PATH + "simpleOgl3.frag");
            sgct::ShaderManager::instance()->bindShaderProgram("OpenGL3R2T");
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }

        if (!sgct::ShaderManager::instance()->shaderProgramExists("Render1Tex")){
            sgct::ShaderManager::instance()->addShaderProgram("Render1Tex", SHADER_PATH + "displayTex.vert", SHADER_PATH + "displayTex.frag");
            sgct::ShaderManager::instance()->bindShaderProgram("Render1Tex");
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }

        if (!sgct::ShaderManager::instance()->shaderProgramExists("backgroundGrid")){
            sgct::ShaderManager::instance()->addShaderProgram("backgroundGrid", SHADER_PATH + "backgroundGrid.vert", SHADER_PATH + "backgroundGrid.frag");
            sgct::ShaderManager::instance()->bindShaderProgram("backgroundGrid");
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }

        if (!sgct::ShaderManager::instance()->shaderProgramExists("InvertColor")){
            sgct::ShaderManager::instance()->addShaderProgram("InvertColor", SHADER_PATH + "simple.vert", SHADER_PATH + "simple.frag");
            sgct::ShaderManager::instance()->bindShaderProgram("InvertColor");
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }


        createdAruco = false;
        initDataDone = false;
        

        sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
        windowsCount = static_cast<int>(thisNode->getNumberOfWindows());


        textureCoordinates = new std::vector<glm::vec3>[windowsCount];
        vertexCoordinates = new std::vector<glm::vec3>[windowsCount];

        // NewCode
        resolutionScale.resize(windowsCount);

        Matrix_Loc = new GLuint[windowsCount];
        vao_Triangle_Scene = new GLuint[windowsCount];
        vbo_Triangle_Scene = new GLuint[windowsCount];
        sceneVertexColorBuffer = new GLuint[windowsCount];
        vao_TransformQuad3v3t = new GLuint[windowsCount];
        vbo_filling_quad = new GLuint[windowsCount];
        fbo_tmpFBO = new GLuint[windowsCount];

        depth_tmpFBO = new GLuint[windowsCount];
        tex_tmpFBO = new GLuint[windowsCount];
        tex_Alpha = new GLuint[windowsCount];
        tex_AlphaTrans = new GLuint[windowsCount];

        tex_ColorLookup = new GLuint[windowsCount];

        projectorPoints = new std::vector<glm::vec3>[windowsCount];
        vao_FillingQuad_3v3t = new GLuint[windowsCount];
        vbo_fillingQuad_3v3t = new GLuint[windowsCount];
        vao_backgroundGrid_3v4c = new GLuint[windowsCount];
        vbo_backgroundGrid_3v4c = new GLuint[windowsCount];

        overlapingVerticesCounter = new std::vector<int>[windowsCount];
        quad_corners = new std::vector<cv::Point2f>[windowsCount];
        TexCoordinates = new std::vector<cv::Point3f>[windowsCount];
        LocalLowHighVP = new std::vector<cv::Point2f>[windowsCount];

        frustumupdate = new bool[windowsCount];
        isVAO_FillingQuad3v3t_Init = new bool[windowsCount];
        isVAO_Triangle_Scene_Init = new bool[windowsCount];
        isVAO_TransformQuad_Init = new bool[windowsCount];
        isVAO_BackgroundGrid_Init = new bool[windowsCount];
        isTex_Alpha_Init = new bool[windowsCount];
        isTmpFBO_Init = new bool[windowsCount];
        initFinalDone = new bool[windowsCount];

        for (int i = 0; i < windowsCount; i++) {
            frustumupdate[i] = false;
            isVAO_FillingQuad3v3t_Init[i] = false;
            isVAO_Triangle_Scene_Init[i] = false;
            isVAO_TransformQuad_Init[i] = false;
            isVAO_BackgroundGrid_Init[i] = false;
            isTex_Alpha_Init[i] = false;
            isTmpFBO_Init[i] = false;
            initFinalDone[i] = false;

            //TODO: change to configurable
            if (this->slaveID == 0 && START_NODE > 0) {
                LocalLowHighVP[i].emplace_back(cv::Point2f(-ViewPlaneX, -ViewPlaneY));
                LocalLowHighVP[i].emplace_back(cv::Point2f(ViewPlaneX, ViewPlaneY));
            }

        }
        LoadCalibrationData();
    }


    /**
     * send a response message to answer a render command from the master
     * check if the response message is already sended
     * if not sended then the message will be sended with the socket client to the master 
     *
     * @param respMsg Shared_Msg wich will be sended to the master
     */
    void Slave::sendMsgToMaster(const Shared_Msg respMsg) {
        if (respondedMsg.msg != respMsg.msg || respondedMsg.window != respMsg.window) {
            if (checkAndStartClientSocket(ClientSocket, masterSocketIP.c_str(), masterSocketPort.c_str(), IPPROTO_UDP)) {
                sendSocketMsg(ClientSocket, sharedMsgToString(respMsg));
                respondedMsg = respMsg;
                showMsgToUser("Message from node %d sent: " + MessageToString(respMsg.msg), this->slaveID);
            }
        }
    }

    /**
    * check if the  render command is destined for this slave
    * then the command will be assigned to the renderCmd member variable 
    *
    * @param cmd Shared_Msg render command from the master
    */
    void Slave::checkMsgFromMaster(const Shared_Msg cmd) {
        if (cmd.slaveID == this->slaveID || cmd.slaveID == ALL) {
            renderCmd.window = cmd.window;
            renderCmd.msg = cmd.msg;
        }
        else {
            renderCmd.window = ALL;
            renderCmd.msg = NONE;
        }
    }


    /*void Slave::checkColorCalibDataFromMaster(const Shared_Msg cmd, const std::vector<int> dat) {
        if (cmd.msg == SHOW_FINAL && (cmd.slaveID == this->slaveID || cmd.slaveID == ALL) && !dat.empty() && lookUpTableData.empty()) {
            std::vector<ColorCalibData> colorCalibData = integerVectorToColorCalibData(dat);
            lookUpTableData = calcColorLookUpTableData(colorCalibData);
        }
    }*/




    /**
    * process the current render command which was received from the master
    * SHOW_FINAL		render the scene
    * SHUTDOWN			show black full screen
    * NONE				close socket client
    * 
    * @param gEngine sgct::Engine * engine pointer to access parameters and methodes from the sgct engine
    */
    void Slave::processCommand(sgct::Engine * gEngine, float currentTime) {
        Shared_Msg response;
        response.slaveID = slaveID;
        response.window = ALL;
        response.msg = NONE;
        int activeWindow = gEngine->getCurrentWindowPtr()->getId();
        sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
        switch (renderCmd.msg) {
        case SHOW_FINAL:
            if (renderCmd.window == ALL) {
                show_final(gEngine, activeWindow, currentTime);
            }
            else if (activeWindow == renderCmd.window){
                show_final(gEngine, renderCmd.window, currentTime);
            }
            response.window = renderCmd.window;
            response.msg = SHOW_FINAL;
            break;
        case SHUTDOWN:
            if (renderCmd.window == ALL) {
                for (unsigned int win = 0; win < thisNode->getNumberOfWindows(); win++) {
                    shutdown(win);
                }
            }
            else {
                shutdown(renderCmd.window);
            }
            response.window = renderCmd.window;
            response.msg = SHUTDOWN;
            break;
        case NONE:
            if (ClientSocket != INVALID_SOCKET) {
                closeSocket(ClientSocket);
                ClientSocket = INVALID_SOCKET;
            }
            response.window = ALL;
            respondedMsg.window = ALL;
            response.msg = NONE;
            respondedMsg.msg = NONE;
            break;
        }	
        
        oldrenderCmd = renderCmd;
        sendMsgToMaster(response);
    }

    void Slave::checkFrustumUpdate(int window) {
        if (!frustumupdate[window]) {
            if (!LocalLowHighVP[window].empty()) {
                updateFrustum(LocalLowHighVP[window][0].x, LocalLowHighVP[window][0].y, LocalLowHighVP[window][1].x, LocalLowHighVP[window][1].y, window);
            }
            frustumupdate[window] = true;
        }
    }

    void Slave::init_transform(sgct::Engine * gEngine, const std::vector<int> coords, const int window) {
        checkFrustumUpdate(window);
        if (isVAO_BackgroundGrid_Init[window] == false)
            initVAO_BackgroundGrid_3v4c(window);
        if (isVAO_Triangle_Scene_Init[window] == false)
            initVAO_Triangle_Scene(window);
        if (isVAO_TransformQuad_Init[window] == false)
            initVAO_TransformQuad3v3t(window);
        if (isTmpFBO_Init[window] == false)
            initTmpFBO_1T(window);
    }

void Slave::init_final(sgct::Engine * gEngine, const std::vector<int> coords, const int window) {
    int current_pro = getProjectorNo(slaveID, window);

    init_transform(gEngine, coords, window);

    if (isTex_Alpha_Init[window] == false) 
    {
        oFramebuffer = new ProjectorOverlapFramebuffer(config, gEngine);
        std::vector<glm::vec3> projectorpoints;
        projectorpoints.clear();

        for (auto &itr : ProjectorCorners_VP)
        {
            // divide the ProjectorPoints throughh Viewplane-Values to get normalized coordinates for the blending
            projectorpoints.emplace_back(glm::vec3(itr.x, itr.y, 0));
        }
        
        oFramebuffer->setViewplanePosition(-1.77777778f / ViewPlaneX, 1.77777778f / ViewPlaneX, -1.0f / ViewPlaneY, 1.0f / ViewPlaneY);

        int idx = current_pro * 4;
        std::vector<cv::Point2f> proX_wallcoord;
        proX_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[idx].x, ProjectorCorners_Wall[idx].y));
        proX_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[idx + 1].x, ProjectorCorners_Wall[idx + 1].y));
        proX_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[idx + 2].x, ProjectorCorners_Wall[idx + 2].y));
        proX_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[idx + 3].x, ProjectorCorners_Wall[idx + 3].y));

        std::vector<cv::Point2f> proX_viewplane;

        proX_viewplane.push_back(cv::Point2f(-1.f, 1.f)); //TEST 14.06.16 evtl. noch Master.sendProjectorCornersToSlave  / ViewPlaneX entfernen
        proX_viewplane.push_back(cv::Point2f(1.f, 1.f));
        proX_viewplane.push_back(cv::Point2f(1.f, -1.f));
        proX_viewplane.push_back(cv::Point2f(-1.f, -1.f));


        cv::Mat WallToProXcoord = cv::getPerspectiveTransform(proX_wallcoord, proX_viewplane);
        std::vector<glm::vec3> proX_viewplaneOGL = opencvPoints2fToGlmVec3(proX_viewplane); //TEST mit proX_wallcoord / proX_viewplane	
        projectorPoints[window].push_back(proX_viewplaneOGL.at(0));
        projectorPoints[window].push_back(glm::vec3(1.f, 1.f, 1.f));
        projectorPoints[window].push_back(proX_viewplaneOGL.at(1));
        projectorPoints[window].push_back(glm::vec3(1.f, 1.f, 1.f));
        projectorPoints[window].push_back(proX_viewplaneOGL.at(2));
        projectorPoints[window].push_back(glm::vec3(1.f, 1.f, 1.f));
        projectorPoints[window].push_back(proX_viewplaneOGL.at(3));
        projectorPoints[window].push_back(glm::vec3(1.f, 1.f, 1.f));
        overlapingVerticesCounter[window].push_back(4);


        for (int i = 0; i <= ProjectorCorners_Wall.size() - 4; i += 4) {
            if (i != idx) {
                std::vector<cv::Point2f> proY_wallcoord;
                proY_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[i].x, ProjectorCorners_Wall[i].y));
                proY_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[i + 1].x, ProjectorCorners_Wall[i + 1].y));
                proY_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[i + 2].x, ProjectorCorners_Wall[i + 2].y));
                proY_wallcoord.push_back(cv::Point2f(ProjectorCorners_Wall[i + 3].x, ProjectorCorners_Wall[i + 3].y));

                //cv::Mat WallToProXcoord = cv::getPerspectiveTransform(proY_wallcoord, proX_wallcoord);
                std::vector<cv::Point2f> proY_in_proXcoord;
                cv::perspectiveTransform(proY_wallcoord, proY_in_proXcoord, WallToProXcoord);
                std::vector<glm::vec3> proY_viewplaneOGL = opencvPoints2fToGlmVec3(proY_in_proXcoord);
                //std::vector<glm::vec3> proY_viewplaneOGL = opencvPoints2fToGlmVec3(proY_wallcoord);

                //TEST: calc overlaping polygon mit vertices + colors/alpha
                std::vector<glm::vec3> overlapPoly;
                int polyVertices = oberlapingPolygone(proX_viewplaneOGL, proY_viewplaneOGL, overlapPoly);
                //std::vector<glm::vec3> overlapPoly2;
                //oberlapingPolygoneSutherlandHodgman(proX_viewplaneOGL, proY_viewplaneOGL, overlapPoly2);
                overlapingVerticesCounter[window].push_back(polyVertices);
                projectorPoints[window].insert(projectorPoints[window].end(), overlapPoly.begin(), overlapPoly.end());
                //projectorPoints[window].insert(projectorPoints[window].end(), proY_viewplaneOGL.begin(), proY_viewplaneOGL.end());
            }
        }
        //---------------------------------------------------------------------------
        //texture width, height, projectorcorners
        // TODO: load color calib data for current projector only. [11/23/2016 Sebastian Maisch]
        oFramebuffer->CreateContext(coords[2], coords[3], projectorPoints[window], overlapingVerticesCounter[window], lookUpTableData);
        
        oFramebuffer->setBlurRadius(static_cast<float>(blurRadius));
        oFramebuffer->setBlurRepetition(blurRepetition);
        oFramebuffer->setWithAlphaTrans(alphaTransition == 1);

        tex_Alpha[window] = oFramebuffer->RenderOverlapGamma(); //gamma
        
        if (alphaTransition == 1) {			
            tex_AlphaTrans[window] = oFramebuffer->RenderOverlapTransistion(); //trans
        }
        isTex_Alpha_Init[window] = true;

        tex_ColorLookup[window] = oFramebuffer->getLookupTexture();
    }
}

    /**
    * @brief renders the correct transformed scene with colorCorrection-Mask
    *
    * binds the correct window, checks if everything is loaded,
    * check if alpha mask is already created and check if ProjectorCorners_VP (Viewplane) is filled
    *       if not created create new class for colorCorrection-Texture and give the class important paramters
    * first renderpass: Render to Texture - disables Tests, renders the scene into fbo-texture
    * second renderpass: render texture with colorCorrection onto a quad, transform it
    *
    * @param gEngine sgct::Engine * engine pointer to access parameters and methodes from the sgct engine
    * @param window int indice of which window is currently rendered
    */
    void Slave::show_final(sgct::Engine * gEngine, int window, float currentTime) {
        std::vector<int> coords(4);
        sgct::SGCTWindow* winPtr = getViewportPixelCoords(window, coords);
        if (!initFinalDone[window]) {
            init_final(gEngine, coords, window);
        }
        

        //////////////////////////////////////////////////////////////////////////
        //drawing scene into fbo
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);

        winPtr->getFBOPtr()->unBind();

        //glViewport(coords[0], coords[1], coords[2], coords[3]);  // NewCode => auskommentiert
        glViewport(coords[0], coords[1], static_cast<GLsizei>(coords[2] * resolutionScale[window].x), static_cast<GLsizei>(coords[3] * resolutionScale[window].y));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_tmpFBO[window]);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_tmpFBO[window]);
        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //TODO: edit for different scene
        draw_scene(window, currentTime);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //////////////////////////////////////////////////////////////////////////
        //render a quad in ortho/2D mode with target texture
        winPtr->getFBOPtr()->bind();

        //get viewport coords
        winPtr->getCurrentViewportPixelCoords(coords[0], coords[1], coords[2], coords[3]);
        glViewport(coords[0], coords[1], coords[2], coords[3]);
        //winPtr->getCurrentViewport()->setPos(coords[0], coords[1]);
        //winPtr->getCurrentViewport()->setSize(coords[2], coords[3]);
        //winPtr->update();

        sgct::ShaderManager::instance()->bindShaderProgram("OpenGL3R2T");
        
        //TODO: test
        auto withAlphaTrans_loc = sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("withAlphaTrans");
        glUniform1i(withAlphaTrans_loc, alphaTransition);

        auto resolution_loc = sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("resolution");
        glUniform2f(resolution_loc, static_cast<float>(coords[2]), static_cast<float>(coords[3]));
    
        //bind drawn scene into textureUnit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_tmpFBO[window]);
        glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("tex"), 0);

        if (alphaTransition == 1) {
            //bind colorCorrection Texture into textureUnit 1
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex_AlphaTrans[window]);
            glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("alphaTrans"), 1);
        }

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_Alpha[window]);
        glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("alphaOverlap"), 2);

        //TODO: test color calib		
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex_ColorLookup[window]);
        glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("OpenGL3R2T").getUniformLocation("colorLookup"), 3);


        glBindVertexArray(vao_TransformQuad3v3t[window]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }


    /**
    * deletes everything and frees memory
    * @param window int indice of which window is currently rendered.
    */
    void Slave::shutdown(int window) {
        //TODO: shutdown projector and delete all vbo and vao and textures and everything that exists
    }

    /**
    * @brief fills the vertex points and the texture coordinates
    * @param window int indice of which window is currently rendered
    */
    void Slave::calc_transform_coordinates(int window)
    {
        textureCoordinates[window].clear();
        vertexCoordinates[window].clear();

        calc_vert_points(window);
        calc_tex_points(window);
    }

    /**
    * @brief fills the vertexCoordinates Vector that is used in initVAO_TransformQuad3v3t()
    *
    * fills the vertexCoordinates that are used in the quad on which the transformed scene is rendered.
    * @param window int indice of which window is currently rendered
    */
    void Slave::calc_vert_points(int window)
    {
        // open cv turns clockwise and starts left top
        // drawn 
        if (quad_corners[window].empty()){
            vertexCoordinates[window].push_back(glm::vec3(-1.0f, -1.0f, 0.0f));//lower left
            vertexCoordinates[window].push_back(glm::vec3(1.0f, -1.0f, 0.0f));//lower right
            vertexCoordinates[window].push_back(glm::vec3(1.0f, 1.0f, 0.0f));//upper right
            vertexCoordinates[window].push_back(glm::vec3(-1.0f, 1.0f, 0.0f));//upper left
        }
        else   {
            glm::vec3 normalization(1.0f, 1.0f, 0.0f);
            vertexCoordinates[window].push_back((glm::vec3(quad_corners[window].at(3).x, quad_corners[window].at(3).y, 0.0f)*2.0f) - normalization);
            vertexCoordinates[window].push_back((glm::vec3(quad_corners[window].at(2).x, quad_corners[window].at(2).y, 0.0f)*2.0f) - normalization);
            vertexCoordinates[window].push_back((glm::vec3(quad_corners[window].at(1).x, quad_corners[window].at(1).y, 0.0f)*2.0f) - normalization);
            vertexCoordinates[window].push_back((glm::vec3(quad_corners[window].at(0).x, quad_corners[window].at(0).y, 0.0f)*2.0f) - normalization);
        }

        // newCode
        unsigned int i = window;
        resolutionScale[i].x = 1;
        resolutionScale[i].y = 1;

        float MinX = vertexCoordinates[i].at(0).x;
        float MaxX = vertexCoordinates[i].at(0).x;
        float MinY = vertexCoordinates[i].at(0).y;
        float MaxY = vertexCoordinates[i].at(0).y;
        for (unsigned int j = 1; j < vertexCoordinates[i].size(); j++)
        {
            if (vertexCoordinates[i].at(j).x < MinX) MinX = vertexCoordinates[i].at(j).x;
            if (vertexCoordinates[i].at(j).y < MinY) MinY = vertexCoordinates[i].at(j).y;
            if (vertexCoordinates[i].at(j).x > MaxX) MaxX = vertexCoordinates[i].at(j).x;
            if (vertexCoordinates[i].at(j).y > MaxY) MaxY = vertexCoordinates[i].at(j).y;
        }
        resolutionScale[i].x = (MaxX - MinX) * 1.1f;
        resolutionScale[i].y = (MaxY - MinY) * 1.1f;
    }

    /**
    * @brief fills the textureCoordinates Vector that is used in initVAO_TransformQuad3v3t()
    *
    * fills the vertexCoordinates that are used in the quad on which the transformed scene is rendered.
    * @param window int indice of which window is currently rendered
    */
    void Slave::calc_tex_points(int window)
    {
        if (TexCoordinates[window].empty()){
            textureCoordinates[window].push_back(glm::vec3(0.0f, 0.0f, 1.0f));//lower left
            textureCoordinates[window].push_back(glm::vec3(1.0f, 0.0f, 1.0f));//lower right
            textureCoordinates[window].push_back(glm::vec3(1.0f, 1.0f, 1.0f));//upper right
            textureCoordinates[window].push_back(glm::vec3(0.0f, 1.0f, 1.0f));//upper left
        }
        else {
            textureCoordinates[window].push_back(glm::vec3(glm::vec3(TexCoordinates[window].at(0).x, TexCoordinates[window].at(0).y, TexCoordinates[window].at(0).z)));
            textureCoordinates[window].push_back(glm::vec3(glm::vec3(TexCoordinates[window].at(1).x, TexCoordinates[window].at(1).y, TexCoordinates[window].at(1).z)));
            textureCoordinates[window].push_back(glm::vec3(glm::vec3(TexCoordinates[window].at(2).x, TexCoordinates[window].at(2).y, TexCoordinates[window].at(2).z)));
            textureCoordinates[window].push_back(glm::vec3(glm::vec3(TexCoordinates[window].at(3).x, TexCoordinates[window].at(3).y, TexCoordinates[window].at(3).z)));
        }
    }

    /**
    * @brief draws the opengl scene
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::draw_scene(int window, float currentTime){
        //background
        glm::mat4 MVP = engine->getCurrentModelViewProjectionMatrix(); 

        sgct::ShaderManager::instance()->bindShaderProgram("backgroundGrid");
        Matrix_Loc[window] = sgct::ShaderManager::instance()->getShaderProgram("backgroundGrid").getUniformLocation("MVP");
        glUniformMatrix4fv(Matrix_Loc[window], 1, GL_FALSE, &MVP[0][0]);

        glBindVertexArray(vao_backgroundGrid_3v4c[window]);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        for (auto quad = 0U; quad < Slave::numberOfQuads; quad++)
        {
            //first triangle
            glDrawArrays(GL_TRIANGLES, quad * 6, 3);
            glDrawArrays(GL_TRIANGLES, quad * 6 + 3, 3);

        }
        glBindVertexArray(0);

        //--------------------
        //triangle_Scene
        sgct::ShaderManager::instance()->unBindShaderProgram();

        float speed = 1.0f;
        glm::mat4 scene_mat = glm::rotate(glm::mat4(1.0f), currentTime * speed, glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = engine->getCurrentModelViewProjectionMatrix() * scene_mat;

        sgct::ShaderManager::instance()->bindShaderProgram("yform");
        Matrix_Loc[window] = sgct::ShaderManager::instance()->getShaderProgram("yform").getUniformLocation("MVP");
        glUniformMatrix4fv(Matrix_Loc[window], 1, GL_FALSE, &MVP[0][0]);

        // Draw the triangle !
        glBindVertexArray(vao_Triangle_Scene[window]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        sgct::ShaderManager::instance()->unBindShaderProgram();
    }

    /**
    * @brief inits VAO for using draw_Triangle_Scene()
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::initVAO_Triangle_Scene(int window){
        const GLfloat vertex_position_data[] = {
            -0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };

        const GLfloat vertex_color_data[] = {
            1.0f, 0.0f, 0.0f, //red
            0.0f, 1.0f, 0.0f, //green
            0.0f, 0.0f, 1.0f //blue
        };

        //generate the VAO
        glGenVertexArrays(1, &vao_Triangle_Scene[window]);
        glBindVertexArray(vao_Triangle_Scene[window]);

        //generate VBO for vertex positions
        glGenBuffers(1, &vbo_Triangle_Scene[window]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_Triangle_Scene[window]);
        //upload data to GPU
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_position_data), vertex_position_data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            reinterpret_cast<void*>(0) // array buffer offset
            );

        //generate VBO for vertex colors
        glGenBuffers(1, &sceneVertexColorBuffer[window]);
        glBindBuffer(GL_ARRAY_BUFFER, sceneVertexColorBuffer[window]);
        //upload data to GPU
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color_data), vertex_color_data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            reinterpret_cast<void*>(0) // array buffer offset
            );

        //unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //todo: clear arrays and delete them

        isVAO_Triangle_Scene_Init[window] = true;

    }

    /**
    * @brief inits VAO with the correct vertex and texture coordinates for drawing the transformed scene on the quad.
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::initVAO_TransformQuad3v3t(int window)
    {
        calc_transform_coordinates(window);

        std::vector<glm::vec3> tmp_fillingQuad;
        tmp_fillingQuad.clear();
        for (int i = 0; i < 4; i++)
        {
            tmp_fillingQuad.push_back(vertexCoordinates[window].at(i));
            tmp_fillingQuad.push_back(textureCoordinates[window].at(i));
        }

        glGenVertexArrays(1, &vao_TransformQuad3v3t[window]);
        glBindVertexArray(vao_TransformQuad3v3t[window]);

        glGenBuffers(1, &vbo_filling_quad[window]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_filling_quad[window]);
        glBufferData(GL_ARRAY_BUFFER, tmp_fillingQuad.size()*sizeof(glm::vec3), tmp_fillingQuad.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        glBindVertexArray(0);

        isVAO_TransformQuad_Init[window] = true;
    }

    /**
    * @brief init of everything needed to render the scene
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::initVAO_FillingQuad_3v3t(int window){

        std::vector<glm::vec3> bufferData;
        bufferData.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));//lower left
        bufferData.push_back(glm::vec3(0.0f, 0.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(1.0f, -1.0f, 0.0f));//lower right
        bufferData.push_back(glm::vec3(1.0f, 0.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(1.0f, 1.0f, 0.0f));//upper right
        bufferData.push_back(glm::vec3(1.0f, 1.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));//upper left
        bufferData.push_back(glm::vec3(0.0f, 1.0f, 1.0f));//tex

        glGenVertexArrays(1, &vao_FillingQuad_3v3t[window]);
        glBindVertexArray(vao_FillingQuad_3v3t[window]);

        glGenBuffers(1, &vbo_fillingQuad_3v3t[window]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_fillingQuad_3v3t[window]);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size()*sizeof(glm::vec3), bufferData.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);//position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);//texCoordinates
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glBindVertexArray(0);
        bufferData.clear();
        isVAO_FillingQuad3v3t_Init[window] = true;
    }

    /**
    * @brief init of everything needed to render the scene
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::initVAO_BackgroundGrid_3v4c(int window){

        backgroundGrid.clear();

        auto delta = 0.125f;
        for (auto x = -6.0f - 1.778f; x < 6; x += delta) {
            auto green = (x + 6) / 12;

            for (float y = -3; y < 3; y += delta) {
                auto red = (y + 3) / 6;

                auto dx = 0.004f;
                auto dy = 0.004f;

                backgroundGrid.emplace_back(glm::vec3(x + dx, y + dy, 0), glm::vec4(red, green, 0.0f, 1.0f));//right top
                backgroundGrid.emplace_back(glm::vec3(x - dx + delta, y + dy, 0), glm::vec4(red, green, 0.0f, 1.0f));//left top
                backgroundGrid.emplace_back(glm::vec3(x - dx + delta, y - dy + delta, 0), glm::vec4(red, green, 0.0f, 1.0f));//left bottom

                backgroundGrid.emplace_back(glm::vec3(x - dx + delta, y - dy + delta, 0), glm::vec4(red, green, 0.0f, 1.0f));//left bottom
                backgroundGrid.emplace_back(glm::vec3(x + dx, y - dy + delta, 0), glm::vec4(red, green, 0.0f, 1.0f));//right bottom
                backgroundGrid.emplace_back(glm::vec3(x + dx, y + dy, 0), glm::vec4(red, green, 0.0f, 1.0f));//right top

            }
        }
        
        glGenVertexArrays(1, &vao_backgroundGrid_3v4c[window]);
        glBindVertexArray(vao_backgroundGrid_3v4c[window]);

        glGenBuffers(1, &vbo_backgroundGrid_3v4c[window]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_backgroundGrid_3v4c[window]);
        glBufferData(GL_ARRAY_BUFFER, backgroundGrid.size()*sizeof(Slave::QuadGrid), backgroundGrid.data(), GL_STATIC_DRAW);		

        glEnableVertexAttribArray(0);//position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Slave::QuadGrid), (GLvoid*)offsetof(QuadGrid, position));
        glEnableVertexAttribArray(1);//color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Slave::QuadGrid), (GLvoid*)offsetof(QuadGrid, color));
        glBindVertexArray(0);
        

        unsigned int numberOfVertices = static_cast<unsigned int>(backgroundGrid.size());
        numberOfQuads = numberOfVertices / 6;

        backgroundGrid.clear();
        

        isVAO_BackgroundGrid_Init[window] = true;

    }

    /**
    * @brief inits a temporary framebufferObject for rendering a scene into a texture
    *
    * the fbo in which the scenes are rendered
    * later on the fbo.texture is used for rendering it onto a quad and display
    *
    * @param window int indice of which window is currently rendered
    */
    void Slave::initTmpFBO_1T(int window)
    {
        //TODO: window size anpassen
        int fb_width;
        int fb_height;

        sgct::SGCTWindow * winPtr = engine->getWindowPtr(window);
        winPtr->getFinalFBODimensions(fb_width, fb_height); //old sgct version: getDrawFBODimensions(fb_width, fb_height);
        
        // NewCode
        fb_width = static_cast<int>(static_cast<float>(fb_width)* resolutionScale[window].x);
        fb_height = static_cast<int>(static_cast<float>(fb_height)* resolutionScale[window].y);

        glGenFramebuffers(1, &fbo_tmpFBO[window]);
        glGenRenderbuffers(1, &(depth_tmpFBO[window]));
        
        //setup texture
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_tmpFBO[window]);
        glGenTextures(1, &(tex_tmpFBO[window]));
        glBindTexture(GL_TEXTURE_2D, tex_tmpFBO[window]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fb_width, fb_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_tmpFBO[window], 0);

        //////
        //if you need depth buffer:
        //setup depth buffer
        //glBindFramebuffer(GL_FRAMEBUFFER, depth_tmpFBO[window]);
        //glBindRenderbuffer(GL_RENDERBUFFER, depth_tmpFBO[window]);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, fb_width, fb_height);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_tmpFBO[window]);

        engine->checkForOGLErrors();
        //sgct::MessageHandler::instance()->print("%d target textures created.\n", numberOfTargets);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        isTmpFBO_Init[window] = true;
    }

    /**
    * @brief updates the sgct frustrum, that is used for the specific window, you are rending in
    *
    * @param window int indices of which window is currently rendered
    */
    void Slave::updateFrustum(float VP_x1, float VP_y1, float VP_x2, float VP_y2, int window) {
        sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
        //old sgct version: setViewPlaneCoords(sgct_core::Viewport::LowerLeft, glm::vec3(VP_x1, VP_y1, 0));
        thisNode->getWindowPtr(window)->getCurrentViewport()->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::LowerLeft, glm::vec3(VP_x1, VP_y1, 0)); 
        thisNode->getWindowPtr(window)->getCurrentViewport()->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperLeft, glm::vec3(VP_x1, VP_y2, 0));
        thisNode->getWindowPtr(window)->getCurrentViewport()->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperRight, glm::vec3(VP_x2, VP_y2, 0));
        //thisNode->getWindowPtr(window)->update(); //TODO: test!
        engine->updateFrustums(); //TODO: test!
    }


}
