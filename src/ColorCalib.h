#ifndef  __COLORCALIB_H__
#define  __COLORCALIB_H__

#include "ShareUtil.h"

namespace pro_cal { 
	const int CELL_COUNT = 4; //mit 1, 2, 4, 8, 10, 20...
	const int VALUE_COUNT = 256;
	const std::vector<glm::vec3> ColorValues = {
		glm::vec3(0, 0, 0), glm::vec3(255, 0, 0), glm::vec3(0, 255, 0), glm::vec3(0, 0, 255)
	};
	
	std::vector<LookUpTableData> calcColorLookUpTableData(const std::vector<ColorCalibData>& gccdList);
	std::vector<glm::vec3> lookUpTableDataToOpenGLTextureDataVec3f(const pro_cal::LookUpTableData& lookUpTableData);
	void saveColorCalibData(const std::vector<ColorCalibData>& colorCalibData);
	void loadColorCalibData(std::vector<ColorCalibData>& colorCalibData);
	void loadColorLutData(std::vector<LookUpTableData>& lookUpTableData);
	void saveColorLutData(const std::vector<LookUpTableData>& lookUpTableData);
	std::vector<int> colorCalibDataToIntegerVector(const std::vector<ColorCalibData>& colorCalibData);
	std::vector<ColorCalibData> integerVectorToColorCalibData(const std::vector<int>& ccdData);

}
#endif  /*__COLORCALIB_H__*/