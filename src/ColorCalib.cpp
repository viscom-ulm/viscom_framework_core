#include "ColorCalib.h"

namespace pro_cal {	
	//L=minVal=max(lowVals) und H=maxVal=min(highVals)
	void calcMinMaxColorValue(const std::vector<ColorCalibData>& gccdList, std::vector<int>& maxFromMinValues, std::vector<int>& minFromMaxValues) {
		//L=minVal=max(lowVals) und H=maxVal=min(highVals)
		for (ColorCalibData gccd : gccdList) {
			for (int c = 0; c < ColorValues.size(); c++) {
				for (int x = 0; x < CELL_COUNT; x++) {
					for (int y = 0; y < CELL_COUNT; y++) {
						int value = gccd.medValues[c][x][y];
						if (value >= 0) {
							maxFromMinValues[c] = (gccd.medValues[c][x][y] > maxFromMinValues[c]) ? gccd.medValues[c][x][y] : maxFromMinValues[c];
							minFromMaxValues[c] = (gccd.medValues[c][x][y] < minFromMaxValues[c]) ? gccd.medValues[c][x][y] : minFromMaxValues[c];
						}
					}
				}
			}
		}
	}

	//Farbraum=0-255 | 0.0-1.0 | ...
	//F(x) = ((H-L)/255)*x + L
	GLfloat transferFunction(GLfloat x, GLfloat L, GLfloat H) {
		return ((H - L) / 255.f) * x + L;
	}

	//inv_f(x) = R_min + (x - P_min)/(P_max-P_min ) * (R_max - R_min )
	//oder inv_f(x) = (x - L) * 255 / (H-L)
	//inverse color transfer function of one projector
	//TODO: more precise 0, 85,170,255..
	GLfloat inverseProjectorTF(GLfloat Fx, GLfloat cellMedianL, GLfloat cellMedianH) {
		return ((Fx - cellMedianL) * 255.f) / (cellMedianH - cellMedianL);
	}

	//texture3d R = > x = CELL_COUNT; y = CELL_COUNT; z = VALUE_COUNT
	//	texture3d G
	//	texture3d B
	//convert color value range from 0-255 to 0-1
	void calcColorValue(std::vector<GLfloat>& colorLookUpTable, int maxFromMin, int minFromMax, int cellMedianL, int cellMedianH) {
		for (int v = 0; v < VALUE_COUNT; v++) {
			GLfloat Fx = transferFunction(v, maxFromMin, minFromMax);
			//GLfloat colorValue = inverseProjectorTF(Fx, gccd.maxValues[0][x][y], gccd.maxValues[c][x][y]);
			GLfloat colorValue = inverseProjectorTF(Fx, cellMedianL, cellMedianH);
			colorLookUpTable[v] = colorValue;
		}
	}
	std::vector<LookUpTableData> calcColorLookUpTableData(const std::vector<ColorCalibData>& gccdList) {
		std::vector<int> maxFromMinValues(ColorValues.size(), 0);
		std::vector<int> minFromMaxValues(ColorValues.size(), 255);
		calcMinMaxColorValue(gccdList, maxFromMinValues, minFromMaxValues);
		std::vector<LookUpTableData> lookUpTableDataList;
		for (ColorCalibData gccd : gccdList) {
			LookUpTableData lutData;
			lutData.LookUpTables.resize(3);
			//std::vector<std::vector<std::vector<std::vector<GLfloat>>>>(3):
			for (int c = 1; c < ColorValues.size(); c++) { //R_G_B ohne 0	
				int lutC = c - 1; 	//c-1 => lut nur R G B	
				lutData.LookUpTables[lutC].resize(CELL_COUNT);
				for (int x = 0; x < CELL_COUNT; x++) {
					lutData.LookUpTables[lutC][x].resize(CELL_COUNT);
					for (int y = 0; y < CELL_COUNT; y++) {	
						lutData.LookUpTables[lutC][x][y].resize(VALUE_COUNT);
						//std::vector<GLfloat> colorLookUpTable(VALUE_COUNT);
						if (gccd.medValues[0][x][y] < 0 || gccd.medValues[c][x][y] < 0) { //TODO: test 08.10.16	
							std::fill(lutData.LookUpTables[lutC][x][y].begin(), lutData.LookUpTables[lutC][x][y].end(), -1.f);
						}
						else {
							calcColorValue(lutData.LookUpTables[lutC][x][y], maxFromMinValues[0], minFromMaxValues[c], gccd.medValues[0][x][y], gccd.medValues[c][x][y]);
						}						
						//lutData.LookUpTables[lutC][x][y] = colorLookUpTable; 
					}
				}
			}
			lookUpTableDataList.push_back(lutData);
		}
		return lookUpTableDataList;
	}

	GLfloat getNextNeighborValueGraterThanZero(const pro_cal::LookUpTableData& lookUpTableData, const int c, const int x, const int y, const int v) {
		int cc = pro_cal::CELL_COUNT;
		for (int i = 1; i < pro_cal::CELL_COUNT; i++) {
			if ((x + i) < cc && lookUpTableData.LookUpTables[c][x + i][y][v] >= 0.f) { //right
				return lookUpTableData.LookUpTables[c][x + i][y][v];
			}
			if ((x - i) > 0 && lookUpTableData.LookUpTables[c][x - i][y][v] >= 0.f) { //left
				return lookUpTableData.LookUpTables[c][x - i][y][v];
			}
			if ((y + i) < cc && lookUpTableData.LookUpTables[c][x][y + i][v] >= 0.f) { //top 
				return lookUpTableData.LookUpTables[c][x][y + i][v];
			}
			if ((y - i) > 0 && lookUpTableData.LookUpTables[c][x][y - i][v] >= 0.f) { //bot
				return lookUpTableData.LookUpTables[c][x][y - i][v];
			}
			if ((x + i) < cc && (y + i) < cc && lookUpTableData.LookUpTables[c][x + i][y + i][v] >= 0.f) { //top_right
				return lookUpTableData.LookUpTables[c][x + i][y + i][v];
			}
			if ((x + i) < cc && (y - i) > 0 && lookUpTableData.LookUpTables[c][x + i][y - i][v] >= 0.f) { //bot_right
				return lookUpTableData.LookUpTables[c][x + i][y - i][v];
			}
			if ((x - i) > 0 && (y + i) < cc && lookUpTableData.LookUpTables[c][x - i][y + i][v] >= 0.f) { //top_left
				return lookUpTableData.LookUpTables[c][x - i][y + i][v];
			}
			if ((x - i) > 0 && (y - i) > 0 && lookUpTableData.LookUpTables[c][x - i][y - i][v] >= 0.f) { //bot_left
				return lookUpTableData.LookUpTables[c][x - i][y - i][v];
			}
		}
		return 0;
	}
	std::vector<glm::vec3> lookUpTableDataToOpenGLTextureDataVec3f(const pro_cal::LookUpTableData& lookUpTableData) {
		int lutTexSize = pro_cal::CELL_COUNT * pro_cal::CELL_COUNT * pro_cal::VALUE_COUNT; //256 * 256 * 256;
		std::vector<glm::vec3> chBuffers(lutTexSize);
		int i = 0;	
		//for GL_TEXTURE_2D_ARRAY	
		for (int v = 0; v < pro_cal::VALUE_COUNT; v++) { 
			for (int y = pro_cal::CELL_COUNT - 1; y >= 0; y--) {  //TODO: test 05.11.16
				for (int x = 0; x < pro_cal::CELL_COUNT; x++) {				
					glm::vec3 colors;
					for (int c = 0; c < 3; c++) {
						GLfloat value = lookUpTableData.LookUpTables[c][x][y][v];
						if (value < 0) {
							value = getNextNeighborValueGraterThanZero(lookUpTableData, c, x, y, v);
						}
						colors[c] = value / 255.f;
					}
					chBuffers[i++] = colors;
				}
			}
		}
		return chBuffers;
	}
	
	std::vector<int> colorCalibDataToIntegerVector(const std::vector<ColorCalibData>& colorCalibData) {
		int lutTexSize = colorCalibData.size() * ColorValues.size() * pro_cal::CELL_COUNT * pro_cal::CELL_COUNT; //256 * 256 * 256;
		std::vector<int> colorCalibDataVector(lutTexSize);
		int i = 0;
		for (ColorCalibData ccd : colorCalibData) {			
			for (int c = 0; c < ColorValues.size(); c++) {
				for (int x = 0; x < CELL_COUNT; x++) {
					for (int y = 0; y < CELL_COUNT; y++) {
						colorCalibDataVector[i++] = ccd.medValues[c][x][y];
					}
				}
			}
		}
		return colorCalibDataVector;
	}

	std::vector<ColorCalibData> integerVectorToColorCalibData(const std::vector<int>& ccdData) {
		int projectorCount = ((ccdData.size() / ColorValues.size()) / pro_cal::CELL_COUNT) / pro_cal::CELL_COUNT;
		std::vector<ColorCalibData> colorCalibData(projectorCount);
		int i = 0;
		for (int p = 0; p < projectorCount; p++) {
			colorCalibData[p].medValues.resize(ColorValues.size());
			for (int c = 0; c < ColorValues.size(); c++) {
				colorCalibData[p].medValues[c].resize(CELL_COUNT);
				for (int x = 0; x < CELL_COUNT; x++) {
					colorCalibData[p].medValues[c][x].resize(CELL_COUNT);
					for (int y = 0; y < CELL_COUNT; y++) {
						colorCalibData[p].medValues[c][x][y] = ccdData[i++];
					}
				}
			}
		}
		return colorCalibData;
	}

	void vec3fVectorToLookUpTables(const int colorIdx, const std::vector<cv::Vec3f> lookUpTableVec3f, LookUpTableData& gridLut) {
		for (cv::Vec3f vec : lookUpTableVec3f) {
			int x = vec[0], y = vec[1];
			float val = vec[2];
			if (gridLut.LookUpTables[colorIdx].empty()) {
				gridLut.LookUpTables[colorIdx].resize(CELL_COUNT);
			}
			if (gridLut.LookUpTables[colorIdx][x].empty()) {
				gridLut.LookUpTables[colorIdx][x].resize(CELL_COUNT);
			}
			gridLut.LookUpTables[colorIdx][x][y].push_back(val);
		}
	}
	void loadColorLutData(std::vector<LookUpTableData>& lookUpTableData) {
		for (int i = 0; i < lookUpTableData.size(); i++) {
			LookUpTableData gridLutData;
			for (int c = 0; c < 3; c++) {
				std::vector<cv::Vec3f> lookUpTableFloat;
				readVectorOfVec3f(COL_DAT_FILE, "LookUpTableData_" + std::to_string(i) + "_" + std::to_string(c), lookUpTableFloat);
				vec3fVectorToLookUpTables(c, lookUpTableFloat, gridLutData);
			}
			lookUpTableData.push_back(gridLutData);
		}
	}
	void gridLookUpTablesToVec3fVector(const int colorIdx, const LookUpTableData& gridLut, std::vector<cv::Vec3f>& lookUpTableVec3f) {
		for (int x = 0; x < CELL_COUNT; x++) {
			for (int y = 0; y < CELL_COUNT; y++) {
				for (int v = 0; v < VALUE_COUNT; v++) {
					float val = gridLut.LookUpTables[colorIdx][x][y][v];
					lookUpTableVec3f.push_back(cv::Vec3f(x, y, val));
				}
			}
		}
	}
	void saveColorLutData(const std::vector<LookUpTableData>& lookUpTableData) {
		for (int i = 0; i < lookUpTableData.size(); i++) {
			for (int c = 0; c < 3; c++) {
				std::vector<cv::Vec3f> lookUpTableVec3f;
				gridLookUpTablesToVec3fVector(c, lookUpTableData[i], lookUpTableVec3f);
				writeVectorOfVec3f(COL_DAT_FILE, "LookUpTableData_" + std::to_string(i) + "_" + std::to_string(c), lookUpTableVec3f);
			}
		}
	}
	void colorCalibDataToVec3fVector(const int colorIdx, const ColorCalibData& ccd, std::vector<cv::Vec3f>& colorCalibDataVec3f) {
		for (int x = 0; x < CELL_COUNT; x++) {
			for (int y = 0; y < CELL_COUNT; y++) {
				float val = ccd.medValues[colorIdx][x][y];
				colorCalibDataVec3f.push_back(cv::Vec3f(x, y, val));
			}
		}
	}
	void saveColorCalibData(const std::vector<ColorCalibData>& colorCalibData) {
		for (int i = 0; i < colorCalibData.size(); i++) {
			//std::vector<std::vector<std::vector<int>>> medValues;
			for (int c = 0; c < ColorValues.size(); c++) {
				std::vector<cv::Vec3f> colorCalibDataVec3f;

				colorCalibDataToVec3fVector(c, colorCalibData[i], colorCalibDataVec3f);
				writeVectorOfVec3f(COL_DAT_FILE, "ColorCalibData_" + std::to_string(i) + "_" + std::to_string(c), colorCalibDataVec3f);
			}
		}
	}
	void vec3fVectorToColorCalibData(const int colorIdx, const std::vector<cv::Vec3f> lookUpTableVec3f, ColorCalibData& ccd) {
		ccd.medValues.resize(ColorValues.size());
		for (cv::Vec3f vec : lookUpTableVec3f) {
			int x = vec[0], y = vec[1];
			float val = vec[2];
			if (ccd.medValues[colorIdx].empty()) {
				ccd.medValues[colorIdx].resize(CELL_COUNT);
			}
			if (ccd.medValues[colorIdx][x].empty()) {
				ccd.medValues[colorIdx][x].resize(CELL_COUNT);
			}
			ccd.medValues[colorIdx][x][y] = val;
		}
	}
	void loadColorCalibData(std::vector<ColorCalibData>& colorCalibData) {
		for (int i = 0; i < colorCalibData.size(); i++) {
			LookUpTableData gridLutData;
			for (int c = 0; c < ColorValues.size(); c++) {
				std::vector<cv::Vec3f> colorCalibDataVec3f;
				readVectorOfVec3f(COL_DAT_FILE, "ColorCalibData_" + std::to_string(i) + "_" + std::to_string(c), colorCalibDataVec3f);
				vec3fVectorToColorCalibData(c, colorCalibDataVec3f, colorCalibData[i]);
			}
			//colorCalibData.push_back(gridLutData);
		}
	}
}