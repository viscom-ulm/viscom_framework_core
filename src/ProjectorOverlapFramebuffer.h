#pragma once

#include <glm\glm.hpp>
#include "sgct.h"
#include "ShareUtil.h"
#include "ColorCalib.h"

namespace pro_cal {
	class ProjectorOverlapFramebuffer {
	public:
		//---------------------------------------------------------------------
		ProjectorOverlapFramebuffer(const FWConfiguration& config, sgct::Engine* engine);
		~ProjectorOverlapFramebuffer();

		//---------------------------------------------------------------------
		bool CreateContext(const int textureWidth, const int textureHeight, const std::vector<glm::vec3>& projectorPoints,
			const std::vector<int> &overlapingVerticesCounter, const pro_cal::LookUpTableData lookUpTableData);

		GLuint RenderOverlapGamma();
		GLuint RenderOverlapTransistion();
		GLuint getUnblurredTexture(){ return this->renderedTexture; };
		GLuint getRenderTexture2(){ return this->renderedTexture2; };
		GLuint getBlurredTexture(){ return this->tex_blend_texture2; };

		GLuint getLookupTexture(){ return this->lookupTexture; };

		void setBlurRadius(GLfloat value){ this->blurRadius = value; }
		void setBlurRepetition(int reps){this->blurRepetitions = reps;} //std::max(1, reps); 
		void setViewplanePosition(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
		void setWithAlphaTrans(bool with) { this->withAlphaTrans = with; };

	private:
		std::vector<glm::vec3> projectorPoints;
		std::vector <glm::vec3> QuadPoints;
		GLuint vao_projector_points;
		GLuint vao_filling_quad;

		GLuint fbo; // initial framebuffer for the first render into
		GLuint fbo2;

		sgct::Engine* gEngine;

		GLuint value_uniform_loc;

		GLuint getVAOFillingQuad(){ return this->vao_filling_quad; };
	    static bool CreateProgram(const FWConfiguration& config, const std::string& programName, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void UseProgram(const std::string& programName);
		bool Destroy();
		void UnBindProgram();
		void printFramebufferInfo(GLenum target, GLuint fbo);
		bool checkFramebufferStatus();
		void initOGL3BlurTexture();

		void blurTex(GLint blur_texture1, GLint blur_texture2);

		GLuint fbo_blend_texture1, tex_blend_texture1, vao_blend_texture, vbo_blend_texture;
		GLuint fbo_blend_texture2, tex_blend_texture2;

		GLuint vbo_filling_quad;
		GLuint vbo_projector_points;

		GLuint renderBuffer;
		GLuint depthStencilBuffer;
		GLuint renderedTexture;
		GLuint renderedTexture2;
		GLuint lookupTexture;

		GLfloat blurRadius;
		int blurRepetitions;
		bool withAlphaTrans;
		GLfloat viewplaneLeft;
		GLfloat viewplaneRight;
		GLfloat viewplaneBottom;
		GLfloat viewplaneTop;

		int screenWidth;
		int screenHeight;
		int numberOfProjectorPoints;
		int numberOfProjectors;
		std::vector<int> polygonVerticesCounter;
		//not functional!!!! beware of bugs
		//bool SaveFramebufferToFile();

		std::vector<std::vector<GLfloat>> chBuffers;
	};
}