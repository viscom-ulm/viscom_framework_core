#pragma once

#include "ProjectorOverlapFramebuffer.h"
namespace pro_cal {
    /**
    * @brief: initializes all important variables and sets defaults values.
    *
    * @param engine sgct:Engine * main engine
    */
    ProjectorOverlapFramebuffer::ProjectorOverlapFramebuffer(const viscom::FWConfiguration& config, sgct::Engine* engine)
    {
        gEngine = engine;

        vao_projector_points = 0;
        vbo_projector_points = 0;

        vao_filling_quad = 0;
        vbo_filling_quad = 0;

        fbo = 0;
        fbo2 = 0;

        renderedTexture = 0;
        renderedTexture2 = 0;

        lookupTexture = 0;

        screenHeight = 0;
        screenWidth = 0;

        renderBuffer = 0;
        depthStencilBuffer = 0;

        value_uniform_loc = 0;
        numberOfProjectorPoints = 0;
        numberOfProjectors = 0;

        blurRadius = 1.0f;
        blurRepetitions = 1;

        viewplaneLeft = -1.0f;
        viewplaneRight = 1.0f;
        viewplaneBottom = -1.0f;
        viewplaneTop = 1.0f;

        //shader program for rendering into a texture
        //CreateProgram("Overlap", SHADER_PATH + "colorCalibShader.vert", SHADER_PATH + "colorCalibShader.frag");
        //CreateProgram("blur_tex", SHADER_PATH + "blur_tex.vert", SHADER_PATH + "blur_tex.frag");
        CreateProgram(config, "Overlap", "colorCalibShader.vert", "colorCalibShader.frag");
        CreateProgram(config, "blur_tex", "blur_tex.vert", "blur_tex.frag");
    }

    //=================================================================================================
    /**
    * @brief deconstructor calls destroy()
    */
    ProjectorOverlapFramebuffer::~ProjectorOverlapFramebuffer()
    {
        Destroy();
    }

    //=================================================================================================
    /**
    * @brief delete created objects on gpu
    */
    bool ProjectorOverlapFramebuffer::Destroy()
    {
        //delete fbos
        glDeleteFramebuffers(1, &fbo);
        glDeleteFramebuffers(1, &fbo2);
        glDeleteFramebuffers(1, &fbo_blend_texture1);
        glDeleteFramebuffers(1, &fbo_blend_texture2);

        //delete all buffers
        glDeleteRenderbuffers(1, &renderBuffer);
        glDeleteRenderbuffers(1, &depthStencilBuffer);
        glDeleteVertexArrays(1, &vao_filling_quad);
        glDeleteVertexArrays(1, &vao_projector_points);

        //delete all unwanted textures
        glDeleteTextures(1, &renderedTexture);
        glDeleteTextures(1, &renderedTexture2);
        glDeleteTextures(1, &tex_blend_texture1);
        glDeleteTextures(1, &tex_blend_texture2);

        glDeleteTextures(1, &lookupTexture);

        projectorPoints.clear();
        QuadPoints.clear();
        polygonVerticesCounter.clear();
        chBuffers.clear();

        return true;
    }

    //=================================================================================================
    /**
    * @brief renders the screenspace, with all projectorspaces, then creates a texture with every overlap
    *
    * renders the whole overlapping texture once and blurres it once with given settings
    * possible to get unblurred texture aswell, see methods for getUnblurredTex(), getBlurredTex()
    *
    * 1. takes every 4 points (projectorPoints), draws them via triangle_fan onto the texture
    * 2. increments everytime a pixel is drawn the stencilBuffer
    * 3. draw final colorCorrection map with the use of stencilBuffer
    * 4. Blur it once, or multiple times (see blurRepetition)
    */
    GLuint ProjectorOverlapFramebuffer::RenderOverlapGamma()
    {
        sgct::SGCTWindow * winPtr = gEngine->getCurrentWindowPtr();//old sgct version: getActiveWindowPtr();
        winPtr->getFBOPtr()->unBind();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        //reset all buffers
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearStencil(0);
        glClearDepth(1.0);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);

        sgct::ShaderManager::instance()->bindShaderProgram("Overlap");
        GLint polygonColor = sgct::ShaderManager::instance()->getShaderProgram("Overlap").getUniformLocation("polygonColor");
        GLuint transformLoc = sgct::ShaderManager::instance()->getShaderProgram("Overlap").getUniformLocation("transform");

        //---------------begin overlapping---------------
        glBindVertexArray(vao_projector_points);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF); //increment everytime a pixel is drawn by one
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);


        glUniform4f(polygonColor, 0.0f, 0.0f, 0.0f, 0.0f);

        glm::mat4 transform = glm::ortho(viewplaneLeft, viewplaneRight, viewplaneBottom, viewplaneTop);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        int positionPtr = 0;
        //draw every projectorPoint onto the normalized screen
        for (int i = 0; i < numberOfProjectors; i++) //TODO: test => for (int i = 1; 
        {
            glDrawArrays(GL_TRIANGLE_FAN, positionPtr, polygonVerticesCounter[i]); //GL_TRIANGLE_STRIP
            positionPtr += polygonVerticesCounter[i];
        }

        glBindVertexArray(0);// disable projectorpoints
        //-- finish overlapping regions --

        //-- begin drawing colorCorrection Alpha Texture --
        glClear(GL_COLOR_BUFFER_BIT);
        glStencilMask(0x00); //disable writing into the stencil buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // keep all stencil bits.. just in case
        glBindVertexArray(vao_filling_quad); // bind screen filling quad

        //------------ stencil check begin ---------

        //ive implemented a 12 values check... because we got possible 12 overlapping projectors
        GLfloat val = 0.0f;
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        //glUniform4f(polygonColor, val, val, val, val);
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //TODO_ test

        //TODO: test
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Gamma Correction for Blending
        // The light curve of the beamer (Optoma HD36) shows that
        // 0.5 of the intensity that is given in as a color comes out as 0.22
        // so we have to correct the gamma values and have to make the value higher to get half of the light intensity
        // 0.5 ^ x = 0.22
        //(1 / n) ^ (1/2.2) // in our example above n=2
        // 2.2 is the standard gamma value

        // 12 values, because in our scenario we have only max 12 overlappings
        // for more projectors needs to be adjusted
#define GAMMA(x) (pow((1.f/x),(1.f/2.2f)))
        for (int i = 1; i <= numberOfProjectors + 1; i++) {
            val = GAMMA((float)i); //TODO: test 21.07.16 => nur anzahl overlaps benoetigt
            //val = 1.f / (float)i;
            glStencilFunc(GL_EQUAL, i, 0xFF);
            glUniform4f(polygonColor, val, val, val, val);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glBindVertexArray(0);

        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);

        sgct::ShaderManager::instance()->unBindShaderProgram();

        if (blurRepetitions > 0 && !withAlphaTrans) {
            //---------------------- BLUR STEP horizontal --------------------------------------
            // increase the size of the texture and afterwards shrink it down to reduce step effect
            // will be used once in any way, if you want unblurred, use method for it, see headerfile
            blurTex(renderedTexture, tex_blend_texture1);
            //return renderedTexture;

            //extra blurrepititions if you want the tex to be more smooth
            // blurRepetitions is max(1, reps) dont worry
            for (int i = 0; i < blurRepetitions - 1; i++)
            {
                blurTex(tex_blend_texture2, tex_blend_texture1);
            }
            return tex_blend_texture2;
        }
        else {
            return renderedTexture;
        }		
    }

    /**
    * @brief blurres texture with given settings, given by set methods
    *
    * increases the texture x4 and blurres it with a gausian horizontal and vertical blur. See shader for more information about the blur.
    */
    void ProjectorOverlapFramebuffer::blurTex(GLint blur_texture1, GLint blur_texture2){

        sgct::SGCTWindow * winPtr = gEngine->getCurrentWindowPtr();
        winPtr->getFBOPtr()->unBind();

        //---------------------- BLUR STEP horizontal --------------------------------------
        // increase the size of the texture and afterwards shrink it down to reduce step effect

        glViewport(0, 0, screenWidth, screenHeight);// screenWidth * 4, screenHeight * 4);

        UseProgram("blur_tex");

        //setting up uniforms
        GLint resolution_loc = sgct::ShaderManager::instance()->getShaderProgram("blur_tex").getUniformLocation("resolution");
        GLint radius_loc = sgct::ShaderManager::instance()->getShaderProgram("blur_tex").getUniformLocation("radius");
        GLint dir_loc = sgct::ShaderManager::instance()->getShaderProgram("blur_tex").getUniformLocation("dir");


        //set uniforms
        glUniform1f(resolution_loc, static_cast<float>(screenWidth));//screenWidth * 4);
        glUniform1f(radius_loc, blurRadius);
        glUniform2f(dir_loc, 1.0f, 0.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_blend_texture1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blur_texture1);
        glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("blur_tex").getUniformLocation("tex"), 0);

        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers);

        glBindVertexArray(vao_blend_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        //---------------------- BLUR STEP vertical --------------------------------------
        // increase the size of the texture and afterwards shrink it down to reduce step effect

        glViewport(0, 0, screenWidth, screenHeight);// screenWidth * 4, screenHeight * 4);

        UseProgram("blur_tex");

        //setting up uniforms
        glUniform1f(resolution_loc, static_cast<float>(screenHeight));//screenHeight * 4);
        glUniform1f(radius_loc, blurRadius);
        glUniform2f(dir_loc, 0.0f, 1.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_blend_texture2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blur_texture2);
        glUniform1i(sgct::ShaderManager::instance()->getShaderProgram("blur_tex").getUniformLocation("tex"), 0);

        glDrawBuffers(1, DrawBuffers);

        glBindVertexArray(vao_blend_texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

    }


    GLuint ProjectorOverlapFramebuffer::RenderOverlapTransistion()
    {
        sgct::SGCTWindow * winPtr = gEngine->getCurrentWindowPtr();//old sgct version: getActiveWindowPtr();
        winPtr->getFBOPtr()->unBind();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

        //reset all buffers
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearStencil(0);
        glClearDepth(1.0);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);

        sgct::ShaderManager::instance()->bindShaderProgram("Overlap");
        GLint polygonColor = sgct::ShaderManager::instance()->getShaderProgram("Overlap").getUniformLocation("polygonColor");
        GLuint transformLoc = sgct::ShaderManager::instance()->getShaderProgram("Overlap").getUniformLocation("transform");

        //---------------begin overlapping---------------
        glUniform4f(polygonColor, -1.0f, -1.0f, -1.0f, -1.0f);

        glm::mat4 transform = glm::ortho(viewplaneLeft, viewplaneRight, viewplaneBottom, viewplaneTop);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(vao_projector_points);


        //GAMMA(x) (pow((1.f/x),(1.f/2.2f)))
        //draw every projectorPoint onto the normalized screen
        glDrawArrays(GL_TRIANGLE_FAN, 0, polygonVerticesCounter[0]);
        int positionPtr = polygonVerticesCounter[0];

        //TODO: für TEST4 auskommentieren
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);

        //glBlendEquation(GL_FUNC_ADD);
        //glBlendFunc(GL_DST_COLOR, GL_ZERO);
        //glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_DST_ALPHA);
        for (int i = 1; i < numberOfProjectors; i++) //TODO: test => for (int i = 1; 
        {
            glDrawArrays(GL_TRIANGLE_FAN, positionPtr, polygonVerticesCounter[i]); //GL_TRIANGLE_STRIP
            positionPtr += polygonVerticesCounter[i];
        }

        glDisable(GL_BLEND);
        glBindVertexArray(0);

        sgct::ShaderManager::instance()->unBindShaderProgram();


        if (blurRepetitions > 0) {
            //---------------------- BLUR STEP horizontal --------------------------------------
            // increase the size of the texture and afterwards shrink it down to reduce step effect
            // will be used once in any way, if you want unblurred, use method for it, see headerfile
            blurTex(renderedTexture2, tex_blend_texture1);

            //extra blurrepititions if you want the tex to be more smooth
            // blurRepetitions is max(1, reps) dont worry
            for (int i = 0; i < blurRepetitions - 1; i++)
            {
                blurTex(tex_blend_texture2, tex_blend_texture1);
            }
            return tex_blend_texture2;
        }
        else {
            return renderedTexture2;
        }
    }


    /**
    * @brief creates context for rendering colorcorrection mask
    *
    * @param int screenWidth sets the screen width for the used texture that is to be blurred
    * @param int screenHeight sets the screen height for the used texture that is to be blurred
    */
    bool ProjectorOverlapFramebuffer::CreateContext(const int screenWidth, const int screenHeight,
        const std::vector<glm::vec3> &projectorPoints, const std::vector<int> &overlapingVerticesCounter, const pro_cal::LookUpTableData lookUpTableData) {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        numberOfProjectorPoints = static_cast<int>(projectorPoints.size() / 2); //TODO: test  / 2 => wegen color data
        numberOfProjectors = static_cast<int>(overlapingVerticesCounter.size());
        polygonVerticesCounter = overlapingVerticesCounter;

        //---------------- projector points vao----------
        glGenVertexArrays(1, &vao_projector_points);
        glBindVertexArray(vao_projector_points);

        glGenBuffers(1, &vbo_projector_points);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_projector_points);
        glBufferData(GL_ARRAY_BUFFER, projectorPoints.size()*sizeof(glm::vec3), projectorPoints.data(), GL_STATIC_DRAW); //TODO: test 14.06.16 geändert =>  sizeof(glm::vec3)
        glEnableVertexAttribArray(0); //vertPosition
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); //TODO: test 14.06.16 geändert =>  3* sizeof(GLfloat)
        //TODO: test 14.06.16
        glEnableVertexAttribArray(1);//vertAlpha
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //--------------- filling quad vao----------
        // watch out... name of vao is the same as in the slave.cpp
        // but they contain not the same... 
        // legacy minor error/ disturbance

        // i take the uv coords out of the vertex points... they are the same.
        QuadPoints.clear();
        QuadPoints.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));
        QuadPoints.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
        QuadPoints.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
        QuadPoints.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

        glGenVertexArrays(1, &vao_filling_quad);
        glBindVertexArray(vao_filling_quad);

        glGenBuffers(1, &vbo_filling_quad);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_filling_quad);
        glBufferData(GL_ARRAY_BUFFER, QuadPoints.size()*sizeof(glm::vec3), QuadPoints.data(), GL_STATIC_DRAW);
        //Vertex Coordinates
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        ////UV Coordinates taking the same as position coords
        //glEnableVertexAttribArray(1);
        //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        QuadPoints.clear();

        //-----------FBO--------------------------------------------------
        // fbo - Renderbuffer ColorAttachment0
        //     - Renderbuffer StencilBuffer

        //creation of frameBufferObject
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        //glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, screenWidth);
        //glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, screenHeight);
        // samples muesste aber von den attachments default eingestellt werden

        ////creation of a renderbuffer
        //glGenRenderbuffers(1, &renderBuffer);
        //glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, screenWidth, screenHeight);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);


        //creation of texture
        glGenTextures(1, &renderedTexture);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);


        // The stencil/depth buffer
        glGenRenderbuffers(1, &depthStencilBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer);


        //set the colorbuffers that are drawn into
        //set the list of draw buffers
        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "DEBUG::FRAMEBUFFERCREATION::FAILED::STATUSCODE " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            return false;
        }
        else
        {
            std::cout << "DEBUG::FRAMEBUFFERCREATION::SUCCESS::STATUSCODE " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        }

        //creation of frameBufferObject
        glGenFramebuffers(1, &fbo2);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

        glGenTextures(1, &renderedTexture2);
        glBindTexture(GL_TEXTURE_2D, renderedTexture2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Set "renderedTexture2" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture2, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        //TODO: test color calib
        /*
        glGenBuffers(1, &CLUTB);
        glBindBuffer(GL_TEXTURE_BUFFER, CLUTB);
        glBufferData(GL_TEXTURE_BUFFER, colorLookUpTable.size(), colorLookUpTable.data(), GL_DYNAMIC_DRAW);

        glGenTextures(1, &colorLookupTexture);
        glBindTexture(GL_TEXTURE_BUFFER, colorLookupTexture);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, CLUTB);
        glActiveTexture(GL_TEXTURE3);
        */
        //chBuffers = lookUpTableDataToOpenGLTextureData(lookUpTableData);
        std::vector<glm::vec3> lutBuffer = lookUpTableDataToOpenGLTextureDataVec3f(lookUpTableData);
        glGenTextures(1, &lookupTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, lookupTexture);
        //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, pro_cal::CELL_COUNT, pro_cal::CELL_COUNT, pro_cal::VALUE_COUNT);
        //glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, pro_cal::CELL_COUNT, pro_cal::CELL_COUNT, pro_cal::VALUE_COUNT, GL_RED, GL_FLOAT, chBuffers[0].data());
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, pro_cal::CELL_COUNT, pro_cal::CELL_COUNT, pro_cal::VALUE_COUNT, 0, GL_RGB, GL_FLOAT, lutBuffer.data());
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_LINEAR GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexImage1D(GL_TEXTURE_1D, 0, GL_R8UI, LookUpTables[0].size(), 0, GL_RED, GL_UNSIGNED_BYTE, LookUpTables[0].data()); 	
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        //-----------blend_tex---------
        initOGL3BlurTexture(); //TODO: test


        return true;
    }

    /**
    * @brief bind shader program
    *
    * @param std::string& programName name of the program that should be bound
    */
    void ProjectorOverlapFramebuffer::UseProgram(const std::string& programName)
    {
        sgct::ShaderManager::instance()->bindShaderProgram(programName);
    }

    /**
    * @brief unbind shader program currently used
    */
    void ProjectorOverlapFramebuffer::UnBindProgram()
    {
        sgct::ShaderManager::instance()->unBindShaderProgram();
    }

    //=================================================================================================
    /**
    * @brief create shader program with vertex and fragment shader
    *
    * @param const std::string& vertexShaderPath path to the vertex shader
    * @param const std::string& fragmentShaderPath path to the fragment shader
    */
    bool ProjectorOverlapFramebuffer::CreateProgram(const viscom::FWConfiguration& config, const std::string& programName,
        const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    {
        if (!sgct::ShaderManager::instance()->shaderProgramExists(programName)){
            sgct::ShaderManager::instance()->addShaderProgram(programName, config.baseDirectory_ + "/shader/" + vertexShaderPath, config.baseDirectory_ + "/shader/" + fragmentShaderPath);
            sgct::ShaderManager::instance()->bindShaderProgram(programName);
            sgct::ShaderManager::instance()->unBindShaderProgram();
        }

        if (sgct::ShaderManager::instance()->shaderProgramExists(programName))
        {
#ifdef DEBUG
            std::cout << "DEBUG::SHADERCREATION::" << programName << "::CREATIONSUCCESSFULL" << std::endl;
#endif // DEBUG
            return true;
        }
        else
        {
#ifdef DEBUG
            std::cout << "DEBUG::SHADERCREATION::" << programName << "::CREATIONFAILED" << std::endl;
#endif // DEBUG
            return false;
        }
    }

    /**
    * @brief prints framebuffer information in console
    *
    * @param GLenum target target you want to test
    * @param GLuint fbo handle of the fbo you want to test
    */
    void ProjectorOverlapFramebuffer::printFramebufferInfo(GLenum target, GLuint fbo)
    {
        int res, i = 0;
        GLint buffer;

        glBindFramebuffer(target, fbo);

        do
        {
            glGetIntegerv(GL_DRAW_BUFFER0 + i, &buffer);

            if (buffer != GL_NONE)
            {

                printf("Shader Output Location %d - color attachment %d\n", i, buffer - GL_COLOR_ATTACHMENT0);

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
                printf("\tAttachment Type: %s\n", res == GL_TEXTURE ? "Texture" : "Render Buffer");

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
                printf("\tAttachment object name: %d\n", res);

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &res);
                printf("\tAttachment object stencil size: %d\n", res);

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &res);
                printf("\tAttachment object alpha size: %d\n", res);

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &res);
                printf("\tAttachment object depth size: %d\n", res);

                glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &res);
                printf("\tAttachment object depth size: %d\n", res);
            }
            ++i;

        } while (buffer != GL_NONE);
    }

    /**
    * @brief checks the currently bound framebuffer status
    */
    bool ProjectorOverlapFramebuffer::checkFramebufferStatus()
    {
        // check FBO status
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            std::cout << "Framebuffer complete." << std::endl;
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
            return false;
            /*
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
            return false;

            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
            std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
            return false;
            */
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
            return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
            return false;

        default:
            std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
            return false;
        }
    }

    /**
    * @brief initializes blurring related rendering parts. VAO, FBO1, FBO2
    *
    * This method intializes everything needed to blur a texture
    * 1. program for blurring
    * 2. VAO with filling quad to draw onto
    * 3. FBO1, FBO2 for rendering into with texture attached
    */
    void ProjectorOverlapFramebuffer::initOGL3BlurTexture(){
        //--------------- filling quad vao----------
        //generating a vector with VTVTVTVT

        std::vector<glm::vec3> bufferData;
        bufferData.clear();
        bufferData.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));//lower left
        bufferData.push_back(glm::vec3(0.0f, 0.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(1.0f, -1.0f, 0.0f));//lower right
        bufferData.push_back(glm::vec3(1.0f, 0.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(1.0f, 1.0f, 0.0f));//upper right
        bufferData.push_back(glm::vec3(1.0f, 1.0f, 1.0f));//tex
        bufferData.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));//upper left
        bufferData.push_back(glm::vec3(0.0f, 1.0f, 1.0f));//tex

        glGenVertexArrays(1, &vao_blend_texture);
        glBindVertexArray(vao_blend_texture);

        glGenBuffers(1, &vbo_blend_texture);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_blend_texture);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size()*sizeof(glm::vec3), bufferData.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);//position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);//texCoordinates
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        glBindVertexArray(0);

        //---------------------- fbo1 --------------------
        glGenFramebuffers(1, &fbo_blend_texture1);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_blend_texture1);

        glGenTextures(1, &tex_blend_texture1);
        glBindTexture(GL_TEXTURE_2D, tex_blend_texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); //screenWidth * 4, screenHeight * 4
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_blend_texture1, 0); //TODO: test glFramebufferTexture => glFramebufferTexture2D

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //-------------------------fbo2------------------
        glGenFramebuffers(1, &fbo_blend_texture2);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_blend_texture2);

        glGenTextures(1, &tex_blend_texture2);
        glBindTexture(GL_TEXTURE_2D, tex_blend_texture2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); //screenWidth * 4, screenHeight * 4
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_blend_texture2, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
    * @brief setter for viewplane positions
    *
    * @param GLfloat left position of the left border (x)
    * @param GLfloat right position of the right border (x)
    * @param GLfloat bottom position of the bottom border (y)
    * @param GLfloat top position of the top border (y)
    */
    void ProjectorOverlapFramebuffer::setViewplanePosition(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top){
        viewplaneLeft = left;
        viewplaneRight = right;
        viewplaneBottom = bottom;
        viewplaneTop = top;
    }
}