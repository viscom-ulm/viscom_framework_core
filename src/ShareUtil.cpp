#include "ShareUtil.h"

namespace pro_cal {
	int START_NODE = 1;
	
	//get information about our window
	sgct::SGCTWindow* getViewportPixelCoords(const int window, std::vector<int> &coords) {
		sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
		sgct::SGCTWindow * winPtr = thisNode->getWindowPtr(window);
		winPtr->getCurrentViewportPixelCoords(coords[0], coords[1], coords[2], coords[3]);
		return winPtr;
	}

	/**
	* convert string with with three integer numbers seperated by whitespaces to a Shared_Msg struct
	*
	* @param msgString string with three integer numbers seperated by whitespaces
	* @return Shared_Msg struct
	*/
	Shared_Msg stringToSharedMsg(const std::string &msgString) {
		Shared_Msg msg;
		if (msgString.size() > 0) {
			std::vector<std::string> strs;
			splitString(msgString, strs, ' ');
			if (strs.size() == 3) {
				msg.slaveID = atoi(strs[0].c_str());
				msg.window = atoi(strs[1].c_str());
				msg.msg = (Message)atoi(strs[2].c_str());
			}
		}
		return msg;
	}

	/**
	* convert Shared_Msg struct to a string with three numbers seperated by whitespaces
	*
	* @param msg Shared_Msg which will be converted to a string
	* @return string with three integer numbers seperated by whitespaces
	*/
	std::string sharedMsgToString(const Shared_Msg msg) {
		std::stringstream ss;
		ss << msg.slaveID << " " << msg.window << " " << msg.msg;
		return ss.str();
	}

	/**
	* convert Message enum to a string which represents the enum description
	*
	* @param msg Message which will be converted to a string
	* @return string which represents the Message enum description
	*/
	std::string MessageToString(const Message msg) {
		switch (msg) {
		case SHOW_FINAL:
			return "SHOW_FINAL";
		case SHUTDOWN:
			return "SHUTDOWN";
		case NONE:
			return "NONE";
		}
		return "";
	}

	/**
	* convert Message enum to a string which represents the enum description
	*
	* @param str string to split by the char ch parameter
	* @param strs vector of strings where the seperated substrings will be assigned
	* @param ch char to split the str string
	*/
	void splitString(const std::string &str, std::vector<std::string> &strs, char ch) {
		std::istringstream iss(str);
		std::string s;
		while (getline(iss, s, ch)) {
			strs.push_back(s);
		}
	}

	/**
	* reads the transforamtion matrix for a node from an opencv xml file
	*
	* @param filename path to the xml file
	* @param nodeID id from the node which matrix will be loaded
	* @param mat cv::Mat to which the loaded matrix will be assigned to
	* @return int 1 if the matrix was loaded successfully and -1 if not
	*/
	void readNodeMatrixFromXML(const std::string filename, const std::string nodeID, cv::Mat& mat) {
		cv::FileStorage fs(filename, cv::FileStorage::READ);
		int frameCount = (int)fs["opencv_storage"];
		fs[nodeID] >> mat;
		fs.release();
	}

	/**
	* writes the transforamtion matrix for a node to an opencv xml file
	*
	* @param filename path to the xml file
	* @param nodeID id from the node which matrix will be saved
	* @param mat cv::Mat with the matrix which will be saved
	* @return int 1 if the matrix was saved successfully and -1 if not
	*/
	void writeNodeMatrixToXML(const std::string filename, const std::string nodeID, const cv::Mat mat) {
		cv::FileStorage fs;
		if (!isFileExist(filename))
		{
			cv::FileStorage fsW(filename, cv::FileStorage::WRITE);
			fs = fsW;
		}
		else {
			cv::FileStorage fsA(filename, cv::FileStorage::APPEND);
			fs = fsA;
		}
		fs << nodeID << mat;
		fs.release();
	}

	void writeVectorOfPoint3f(const std::string filename, const std::string nodeID, const std::vector<cv::Point3f> points) {
		cv::Mat mat = cv::Mat(points);
		writeNodeMatrixToXML(filename, nodeID, mat);
	}

	void readVectorOfPoint3f(const std::string filename, const std::string nodeID, std::vector<cv::Point3f> &points)
	{
		cv::Mat mat;
		readNodeMatrixFromXML(filename, nodeID, mat);
		mat.copyTo(points);
	}

	void writeVectorOfPoint2f(const std::string filename, const std::string nodeID, const std::vector<cv::Point2f> points)
	{
		cv::Mat mat = cv::Mat(points);
		writeNodeMatrixToXML(filename, nodeID, mat);
	}

	void readVectorOfPoint2f(const std::string filename, const std::string nodeID, std::vector<cv::Point2f> &points)
	{
		cv::Mat mat;
		readNodeMatrixFromXML(filename, nodeID, mat);
		mat.copyTo(points);
	}

	void writeVectorOfUchar(const std::string filename, const std::string nodeID, const std::vector<uchar> points)
	{
		cv::Mat mat = cv::Mat(points);
		writeNodeMatrixToXML(filename, nodeID, mat);
	}

	void readVectorOfUchar(const std::string filename, const std::string nodeID, std::vector<uchar> &points)
	{
		cv::Mat mat;
		readNodeMatrixFromXML(filename, nodeID, mat);
		mat.copyTo(points);
	}


	void writeVectorOfGLfloat(const std::string filename, const std::string nodeID, const std::vector<GLfloat> points)
	{
		cv::Mat mat = cv::Mat(points);
		writeNodeMatrixToXML(filename, nodeID, mat);
	}

	void readVectorOfGLfloat(const std::string filename, const std::string nodeID, std::vector<GLfloat> &points)
	{
		cv::Mat mat;
		readNodeMatrixFromXML(filename, nodeID, mat);
		mat.copyTo(points);
	}

	void writeVectorOfVec3f(const std::string filename, const std::string nodeID, const std::vector<cv::Vec3f> points)
	{
		//cv::Mat mat = cv::Mat(points);
		cv::Mat_<cv::Vec3f> mat = cv::Mat_<cv::Vec3f>(points);
		writeNodeMatrixToXML(filename, nodeID, mat);
	}

	void readVectorOfVec3f(const std::string filename, const std::string nodeID, std::vector<cv::Vec3f> &points)
	{
		cv::Mat_<cv::Vec3f> mat;
		readNodeMatrixFromXML(filename, nodeID, mat);
		mat.copyTo(points);
	}

	/**
	* check if the a file exists
	*
	* @param filename path to the file
	* @return bool true if the file exists, false if not
	*/
	bool isFileExist(const std::string filename) {
		std::ifstream infile(filename);
		bool exist = infile.good();
		infile.close();
		return exist;
	}


	/**
	*  print a message for the user in the sgct terminal
	*
	* @param msg std::string message to print
	* @param x int generic integer parameter in the message
	*/
	void showMsgToUser(std::string msg, int x) {
		msg += "\n";
		sgct::MessageHandler::instance()->print(msg.c_str(), x);
	}

	/**
	*  convert a opencv Mat object to a std::vector<double>
	*
	* @param mat cv::Mat which data will be assigned to a vector
	* @param std::vector<double> result with the data from the opencv Mat object
	*/
	std::vector<double> opencvMatToDoubleVector(cv::Mat mat) {
		std::vector<double> matArray;
		matArray.assign((double*)mat.datastart, (double*)mat.dataend);
		return matArray;
	}


	/**
	*  converts opencv Points to a glm::vec3
	*
	*/
	std::vector<glm::vec3> opencvPoints2fToGlmVec3(std::vector<cv::Point2f> points) {
		std::vector<glm::vec3> vecs;
		for (cv::Point2f point : points) {
			vecs.push_back(glm::vec3(point.x, point.y, 0.f));
		}
		//std::reverse(vecs.begin(), vecs.end());
		return vecs;
	}

	std::vector<cv::Point2f> points3fToPoints2fVector(std::vector<cv::Point3f> points3f) {
		std::vector<cv::Point2f> point2f;
		for (cv::Point3f point3f : points3f) {
			point2f.push_back(cv::Point2f(point3f.x, point3f.y));
		}
		return point2f;
	}

	std::vector<cv::Point2f> vec3ToPoints2fVector(std::vector<glm::vec3> v3) {
		std::vector<cv::Point2f> point2f;
		for (glm::vec3 v : v3) {
			point2f.push_back(cv::Point2f(v.x, v.y));
		}
		return point2f;
	}

	std::vector<glm::vec2> vec3ToVec2(std::vector<glm::vec3> v3) {
		std::vector<glm::vec2> vecs;
		for (glm::vec3 v : v3) {
			vecs.push_back(glm::vec2(v));
		}
		return vecs;
	}
	std::vector<glm::vec3> vec2ToVec3(std::vector<glm::vec2> v2) {
		std::vector<glm::vec3> vecs;
		for (glm::vec2 v : v2) {
			vecs.push_back(glm::vec3(v, 0.f));
		}
		return vecs;
	}

	std::vector<GLbyte> ucharToGLbyteVector(std::vector<uchar> ucharVec) {
		std::vector<GLbyte> bytesVec;
		for (uchar uc : ucharVec) {
			bytesVec.push_back((uchar)uc);
		}
		return bytesVec;
	}

	/**
	* Calculates the current projector number from the current window from the current slave
	*
	* @return int the current projector number
	*/
	int getProjectorNo(int nodeID, int window) {			
		if (nodeID >= START_NODE) {
			int current_projector = 0;
			for (int i = START_NODE; i < sgct_core::ClusterManager::instance()->getNumberOfNodes(); i++) {
				sgct_core::SGCTNode * currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
				for (int j = 0; j < currNode->getNumberOfWindows(); j++) {
					if (i == nodeID && j == window) {
						return current_projector;
					}
					current_projector += 1;
				}
			}
		}
		return 0;
	}


	/**
	* Finds the intersection of two lines, or returns false.
	* The lines are defined by (o1, p1) and (o2, p2).
	* @param o1
	* @param p1
	* @param o2
	* @param p2
	* @param r
	*/
	bool intersection(glm::vec2 o1, glm::vec2 p1, glm::vec2 o2, glm::vec2 p2, glm::vec2 &r)
	{
		glm::vec2 x = o2 - o1;
		glm::vec2 d1 = p1 - o1;
		glm::vec2 d2 = p2 - o2;

		float cross = d1.x*d2.y - d1.y*d2.x;
		if (abs(cross) < /*EPS*/1e-8)
			return false;

		float t1 = (x.x * d2.y - x.y * d2.x) / cross;
		r.x = o1.x + d1.x * t1;
		r.y = o1.y + d1.y * t1;
		return true;
	}


	// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
	// intersect the intersection point may be stored in the floats i_x and i_y.
	bool getLineIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 &inter)
	{
		float s1_x, s1_y, s2_x, s2_y;
		s1_x = p1.x - p0.x;     s1_y = p1.y - p0.y;
		s2_x = p3.x - p2.x;     s2_y = p3.y - p2.y;

		float s, t;
		s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = (s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			// Collision detected
			inter.x = p0.x + (t * s1_x);
			inter.y = p0.y + (t * s1_y);
			return true;
		}

		return false; // No collision
	}
	//http://totologic.blogspot.de/2014/01/accurate-point-in-triangle-test.html
	bool pointInTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 p) {
		//using barycentric coords
		float denominator = ((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));
		float a = ((v2.y - v3.y)*(p.x - v3.x) + (v3.x - v2.x)*(p.y - v3.y)) / denominator;
		float b = ((v3.y - v1.y)*(p.x - v3.x) + (v1.x - v3.x)*(p.y - v3.y)) / denominator;
		float c = 1 - a - b;
		return 0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1;
	}
	bool pointInPolygon(glm::vec3 test, std::vector<glm::vec3> polygon)
	{
		for (int i = 0; i < polygon.size(); i++) {
			bool inTriangle = pointInTriangle(polygon[i], polygon[(i + 1) % polygon.size()], polygon[(i + 2) % polygon.size()], test);
			if (inTriangle) {
				return true;
			}
		}
		return false;
	}
	float angleBetween(glm::vec2 a, glm::vec2 b)
	{
		float sin = a.x * b.y - b.x * a.y;
		float cos = a.x * b.x + a.y * b.y;
		//float l1 = glm::length(a);
		//float l2 = glm::length(b);
		//return glm::acos((glm::dot(a, b)) / (l1 * l2));
		return std::atan2(sin, cos) * (180.f / atan(1) * 4);
	}
	//http://stackoverflow.com/questions/6989100/sort-points-in-clockwise-order
	bool less(glm::vec2 a, glm::vec2 b, glm::vec2 center)
	{
		if (a.x - center.x >= 0 && b.x - center.x < 0)
			return true;
		if (a.x - center.x < 0 && b.x - center.x >= 0)
			return false;
		if (a.x - center.x == 0 && b.x - center.x == 0) {
			if (a.y - center.y >= 0 || b.y - center.y >= 0)
				return a.y > b.y;
			return b.y > a.y;
		}

		// compute the cross product of vectors (center -> a) x (center -> b)
		float det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
		if (det < 0)
			return true;
		if (det > 0)
			return false;

		// points a and b are on the same line from the center
		// check which point is closer to the center
		float d1 = (a.x - center.x) * (a.x - center.x) + (a.y - center.y) * (a.y - center.y);
		float d2 = (b.x - center.x) * (b.x - center.x) + (b.y - center.y) * (b.y - center.y);
		return d1 > d2;
	}

	struct {
		glm::vec3 center;
		bool operator()(glm::vec3 a, glm::vec3 b) {
			//float angle1 = angleBetween(glm::vec2(0.f, 1.f), glm::vec2(a.x, a.y));
			//float angle2 = angleBetween(glm::vec2(0.f, 1.f), glm::vec2(b.x, b.y));
			//return angle1 > angle2;
			return less(glm::vec2(a), glm::vec2(b), glm::vec2(center));
		}
	} angleComperator;

	void sortCorners(std::vector<glm::vec3>& corners) {
		if (corners.size() > 2) {						
			//callc average vector => center
			for (glm::vec3 v : corners) {
				angleComperator.center += v;
			}
			angleComperator.center /= corners.size();
			std::sort(corners.begin(), corners.end(), angleComperator);
		}
	}

	int oberlapingPolygone(std::vector<glm::vec3> quad1, std::vector<glm::vec3> quad2, std::vector<glm::vec3> &overlap)
	{
		//TODO: test 06.07.2016 => gammaVal & simpleObgl3.alphaTransValue vergleichen mit simpleObgl3.alphaTransWithGamma ohne gammaVal
		//overlapingPolygone => entweder case1 [intersections & Quad1InsideQuad2] oder case2 [intersections & Quad2insideQuad1]
		//case1 => intersections_alpha = 0.5;  Quad1InsideQuad2_alpha = 0
		//case2 => intersections_alpha = 0.5;  Quad2insideQuad1_alpha = 1
		//float gamma = 1.f / 2.2f;
		std::vector<glm::vec3> intersections;
		for (int i = 0; i < 4; i++) {			
			for (int j = 0; j < 4; j++) {
				glm::vec2 inter;
				bool intersected = getLineIntersection(glm::vec2(quad1[i]), glm::vec2(quad1[(i + 1) % 4]), glm::vec2(quad2[j]), glm::vec2(quad2[(j + 1) % 4]), inter);
				if (intersected) {
					//float gammaVal = pow((0.5f), gamma);
					intersections.push_back(glm::vec3(inter, 0.5f));
					//intersections.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
				}
			}
		}
		if (!intersections.empty()) {
			std::vector<glm::vec3> insideQuad1;
			for (glm::vec3 v2 : quad2) {
				bool isInQuad1 = pointInPolygon(v2, quad1);
				if (isInQuad1) {
					//float gammaVal = pow((0.999f), gamma);
					insideQuad1.push_back(glm::vec3(v2.x, v2.y, 1.f));
					//insideQuad1.push_back(glm::vec3(1.f, 1.f, 1.f));
				}
			}

			std::vector<glm::vec3> insideQuad2;
			for (glm::vec3 v1 : quad1) {
				bool isInQuad2 = pointInPolygon(v1, quad2);
				if (isInQuad2) {
					//float gammaVal = pow((0.001f), gamma);
					insideQuad2.push_back(glm::vec3(v1.x, v1.y, 0.f));
					//insideQuad2.push_back(glm::vec3(0.f, 0.f, 0.f));
				}
			}

			std::vector<glm::vec3> corners;
			if (!insideQuad1.empty()) {
				corners.insert(corners.end(), insideQuad1.begin(), insideQuad1.end());
			}
			if (!insideQuad2.empty()) {
				corners.insert(corners.end(), insideQuad2.begin(), insideQuad2.end());
			}
			corners.insert(corners.end(), intersections.begin(), intersections.end()); //TODO: test 15.06.		
			sortCorners(corners);
			for (glm::vec3 co : corners) {
				overlap.push_back(glm::vec3(co.x, co.y, 0.f));
				overlap.push_back(glm::vec3(co.z, co.z, co.z));
			}
		}
		return overlap.size() / 2;
	}

	int countOverlapsFromVertex(glm::vec3 vertex, const std::vector<glm::vec3> &projectorPoints, const std::vector<int> &polygonVerticesCounter)
	{
		int positionPtr = 0, overlapCounter = 0;
		for (int i = 0; i < polygonVerticesCounter.size(); i++) { //TODO: test => for (int i = 1; 	
			//polygon without color data
			std::vector<glm::vec3> polygon;
			for (int j = 0; j < polygonVerticesCounter[i]; j++) {
				polygon.push_back(projectorPoints[positionPtr+(j*2)]);
			}
			//	(projectorPoints.begin() + positionPtr, projectorPoints.begin() + positionPtr + overlapingVerticesCounter[i]);
			bool isInPoly = pointInPolygon(vertex, polygon);
			if (isInPoly) {
				overlapCounter += 1;
			}
			positionPtr += polygonVerticesCounter[i] * 2;
		}
		return overlapCounter;
	}
	void setVertexAlphas(std::vector<glm::vec3> &projectorPoints, const std::vector<int> &overlapingVerticesCounter) {
		//TODO: test => 21.07.16
		for (int i = 0; i < projectorPoints.size() - 1; i += 2) {
			glm::vec3 vertex = projectorPoints[i];
			glm::vec3 colorData = projectorPoints[i + 1];
			if (colorData.x == 0.5) { //TODO: test
				int overlapCounter = countOverlapsFromVertex(vertex, projectorPoints, overlapingVerticesCounter);
				if (overlapCounter > 2) {
					float ov = 1.f / overlapCounter; //TODO: test 22.07.16
					projectorPoints[i + 1] = glm::vec3(glm::pow(ov, ov)); //glm::pow(ov, 1.f / (overlapCounter-1)
				}
			}
		}
	}

	//OpenCV stores images from top to bottom, left to right
	//stores color images in BGR order
	//might not store image rows tightly packed
	cv::Mat openglTextureToOpenCV(int width, int height) {
		cv::Mat img(height, width, CV_8UC3);
		//use fast 4-byte alignment (default anyway) if possible
		glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
		//set length of one complete row in destination data (doesn't need to equal img.cols)
		glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());
		glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
		cv::Mat flipped;
		cv::flip(img, flipped, 0);
		return flipped;
	}


	//
	// Polygon Clipping routines inspired by, pseudo code found here: http://www.cs.rit.edu/~icss571/clipTrans/PolyClipBack.html
	//
	// Coverage Map's polygon coordinates are from -1 to 1 in the following mapping to screen space.
	//
	//         (0,0)                   (windowWidth, 0)
	//         -1,1                    1,1
	//           +-----------------------+ 
	//           |           |           |
	//           |           |           |
	//           | -1,0      |           |
	//           |-----------+-----------|
	//           |          0,0          |
	//           |           |           |
	//           |           |           |
	//           |           |           |
	//           +-----------------------+
	//           -1,-1                  1,-1
	// (0,windowHeight)                (windowWidth,windowHeight)
	//

	bool pointInsideBoundary(const glm::vec2& testVertex, const std::vector<glm::vec2>& clipBoundary) {
		// bottom edge
		if (clipBoundary[1].x > clipBoundary[0].x) {
			if (testVertex.y >= clipBoundary[0].y) {
				return true;
			}
		}
		// top edge
		if (clipBoundary[1].x < clipBoundary[0].x) {
			if (testVertex.y <= clipBoundary[0].y) {
				return true;
			}
		}
		// right edge
		if (clipBoundary[1].y > clipBoundary[0].y) {
			if (testVertex.x <= clipBoundary[1].x) {
				return true;
			}
		}
		// left edge
		if (clipBoundary[1].y < clipBoundary[0].y) {
			if (testVertex.x >= clipBoundary[1].x) {
				return true;
			}
		}
		return false;
	}

	void segmentIntersectsBoundary(const glm::vec2& first, const glm::vec2&  second,
		const std::vector<glm::vec2>& clipBoundary, glm::vec2& intersection) {
		// horizontal
		if (clipBoundary[0].y == clipBoundary[1].y) {
			intersection.y = clipBoundary[0].y;
			intersection.x = first.x + (clipBoundary[0].y - first.y) * (second.x - first.x) / (second.y - first.y);
		}
		else { // Vertical
			intersection.x = clipBoundary[0].x;
			intersection.y = first.y + (clipBoundary[0].x - first.x) * (second.y - first.y) / (second.x - first.x);
		}
	}

	/**
	* Sorting cv::Point2f corners in a vector in order: 1.top_left 2.top_right 3.bot_right 4.bot_left
	*
	* @param corners const reference vector for the unsorted corners
	* @param sortedCorners reference vector for the sorted corners output
	*/
	void sortCorners(const std::vector<cv::Point2f>& corners, std::vector<cv::Point2f>& sortedCorners)
	{
		if (corners.size() >= 4) {
			std::vector<cv::Point2f> top, bot, left, right;
			cv::Point2f top_left = corners[0], top_right = corners[1], bot_left = corners[2], bot_right = corners[3];

			for (unsigned int i = 0; i < corners.size(); i++)
			{
				float xy = corners[i].x + corners[i].y;

				float tl_xy = top_left.x + top_left.y;
				if (xy < tl_xy)
					top_left = corners[i];

				float br_xy = bot_right.x + bot_right.y;
				if (xy > br_xy)
					bot_right = corners[i];

				xy = corners[i].x - corners[i].y;
				float tr_xy = top_right.x - top_right.y;
				if (xy > tr_xy)
					top_right = corners[i];

				xy = corners[i].y - corners[i].x;
				float bl_xy = bot_left.y - bot_left.x;
				if (xy > bl_xy)
					bot_left = corners[i];
			}
			sortedCorners.clear();
			sortedCorners.push_back(top_left);
			sortedCorners.push_back(top_right);
			sortedCorners.push_back(bot_right);
			sortedCorners.push_back(bot_left);
		}
	}


	std::vector<cv::Point3f> convertPoint3fVectorOfVectorToVector(const std::vector<std::vector<cv::Point3f>> points2D) {
		std::vector<cv::Point3f> points1D;
		for (size_t i = 0; i < points2D.size(); i++) {
			for (size_t j = 0; j < points2D[i].size(); j++) {
				points1D.push_back(cv::Point3f(points2D[i][j].x, points2D[i][j].y, points2D[i][j].z));
			}
		}
		return points1D;
	}
	std::vector<cv::Point2f> convertPoint2fVectorOfVectorToVector(const std::vector<std::vector<cv::Point2f>> points2D) {
		std::vector<cv::Point2f> points1D;
		for (size_t i = 0; i < points2D.size(); i++) {
			for (size_t j = 0; j < points2D[i].size(); j++) {
				points1D.push_back(cv::Point2f(points2D[i][j].x, points2D[i][j].y));
			}
		}
		return points1D;
	}
}