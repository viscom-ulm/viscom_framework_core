#ifndef  __SHAREUTIL_H__
#define  __SHAREUTIL_H__

#include <string>
#include <iostream>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "sgct.h"

namespace pro_cal {/*
	const std::string CFG_FILE = "config\\properties.xml";
	const std::string DAT_PATH = "data\\";
	const std::string DAT_FILE = DAT_PATH + "ProjectorData.xml";
	const std::string COL_DAT_FILE = DAT_PATH + "ColorCalibData.xml";
	const std::string IMG_PATH = "images\\";
	const std::string SHADER_PATH = "shader\\";*/

	extern int START_NODE;
	const float ViewPlaneX = 1.77777778f;
	const float ViewPlaneY = 1.0f;
	const float resolutionX = 1920.f;
	const float resolutionY = 1080.f;

	typedef unsigned char byte;
	enum Message { 
		NONE, SHOW_FINAL, SHUTDOWN, ERROR_MSG
	};
	const static int ALL = 255;
	struct Shared_Msg {
		int slaveID;
		int window;
		Message msg;
	};

	class LookUpTableData {
	public:
		std::vector<std::vector<std::vector<std::vector<GLfloat>>>> LookUpTables; //[colorIdx][x][y][colorVal]
		LookUpTableData() : LookUpTables(std::vector<std::vector<std::vector<std::vector<GLfloat>>>>(3)) {};
	};

	class ColorCalibData {
	public:
		std::vector<std::vector<std::vector<int>>> medValues; //[colorIdx][x][y]	
		ColorCalibData(){};
		ColorCalibData(int colorCount) :
			medValues(std::vector<std::vector<std::vector<int>>>(colorCount)){};
	};

	Shared_Msg stringToSharedMsg(const std::string &msgString);
	std::string sharedMsgToString(const Shared_Msg msg);

	void splitString(const std::string &str, std::vector<std::string> &strs, char ch);
	void readNodeMatrixFromXML(const std::string filename, const std::string nodeID, cv::Mat& mat);
	void writeNodeMatrixToXML(const std::string filename, const std::string nodeID, const cv::Mat mat);
	bool isFileExist(const std::string filename);
	void showMsgToUser(std::string msg, int x);
	std::string MessageToString(const Message msg);
	std::vector<double> opencvMatToDoubleVector(cv::Mat mat);
	std::vector<glm::vec3> opencvPoints2fToGlmVec3(std::vector<cv::Point2f> points);
	std::vector<cv::Point2f> vec3ToPoints2fVector(std::vector<glm::vec3> v3);
	int getProjectorNo(int nodeID, int window);
	bool pointInPolygon(glm::vec3 test, std::vector<glm::vec3> polygon);
	bool intersection(glm::vec2 o1, glm::vec2 p1, glm::vec2 o2, glm::vec2 p2, glm::vec2 &r);
	bool getLineIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 &inter);
	int oberlapingPolygone(std::vector<glm::vec3> quad1, std::vector<glm::vec3> quad2, std::vector<glm::vec3> &overlap);
	void writeVectorOfPoint3f(const std::string filename, const std::string nodeID, const std::vector<cv::Point3f> points);
	void readVectorOfPoint3f(const std::string filename, const std::string nodeID, std::vector<cv::Point3f> &points);
	void writeVectorOfPoint2f(const std::string filename, const std::string nodeID, const std::vector<cv::Point2f> points);
	void readVectorOfPoint2f(const std::string filename, const std::string nodeID, std::vector<cv::Point2f> &points);
	void writeVectorOfUchar(const std::string filename, const std::string nodeID, const std::vector<uchar> points);
	void readVectorOfUchar(const std::string filename, const std::string nodeID, std::vector<uchar> &points);
	void writeVectorOfGLfloat(const std::string filename, const std::string nodeID, const std::vector<GLfloat> points);
	void readVectorOfGLfloat(const std::string filename, const std::string nodeID, std::vector<GLfloat> &points);
	void writeVectorOfVec3f(const std::string filename, const std::string nodeID, const std::vector<cv::Vec3f> points);
	void readVectorOfVec3f(const std::string filename, const std::string nodeID, std::vector<cv::Vec3f> &points);
	void setVertexAlphas(std::vector<glm::vec3> &projectorPoints, const std::vector<int> &overlapingVerticesCounter);
	cv::Mat openglTextureToOpenCV(int height, int width);
	std::vector<cv::Point2f> points3fToPoints2fVector(std::vector<cv::Point3f> points3f);
	std::vector<GLbyte> ucharToGLbyteVector(std::vector<uchar> ucharVec);
	void sortCorners(const std::vector<cv::Point2f>& corners, std::vector<cv::Point2f>& sortedCorners);
	sgct::SGCTWindow* getViewportPixelCoords(const int window, std::vector<int> &coords);
	std::vector<cv::Point3f> convertPoint3fVectorOfVectorToVector(const std::vector<std::vector<cv::Point3f>> points2D);
	std::vector<cv::Point2f> convertPoint2fVectorOfVectorToVector(const std::vector<std::vector<cv::Point2f>> points2D);
}
#endif  /*__SHAREUTIL_H__*/