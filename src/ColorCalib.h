#ifndef  __COLORCALIB_H__
#define  __COLORCALIB_H__

#include "ShareUtil.h"
#include "main.h"

namespace pro_cal { 
    extern int CELL_COUNT; //config in properties.xml => 1, 2, 4, 8, 10, 20, 40, 60, 120
    constexpr int VALUE_COUNT = 256;

    std::vector<LookUpTableData> calcColorLookUpTableData(const std::vector<ColorCalibData>& gccdList);
    LookUpTableData calcColorLookUpTableDataSingle(const viscom::FWConfiguration& config, unsigned int projectorId);

    std::vector<glm::vec3> lookUpTableDataToOpenGLTextureDataVec3f(const pro_cal::LookUpTableData& lookUpTableData);
    void loadColorCalibData(const viscom::FWConfiguration& config, std::vector<ColorCalibData>& colorCalibData);
    void loadColorCalibDataSingle(const viscom::FWConfiguration& config, unsigned int projectorId, ColorCalibData& colorCalibData);

    void loadColorLutData(const viscom::FWConfiguration& config, std::vector<LookUpTableData>& lookUpTableData);
    std::vector<int> colorCalibDataToIntegerVector(const std::vector<ColorCalibData>& colorCalibData);
    std::vector<ColorCalibData> integerVectorToColorCalibData(const std::vector<int>& ccdData);

}
#endif  /*__COLORCALIB_H__*/