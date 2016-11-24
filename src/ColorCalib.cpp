#include "ColorCalib.h"

namespace pro_cal {	
    int CELL_COUNT = 10; //config in properties.xml => 1, 2, 4, 8, 10, 20, 40, 60, 120

    const std::vector<glm::vec3> ColorValues = {
        glm::vec3(0, 0, 0), glm::vec3(255, 0, 0), glm::vec3(0, 255, 0), glm::vec3(0, 0, 255)
    };

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

    /**
    * transfer function from all projector cells
    * calculate the corresponding value from x-value
    */
    GLfloat transferFunction(GLfloat x, GLfloat L, GLfloat H) {
        //Farbraum=0-255 | 0.0-1.0 | ...
        //F(x) = ((H-L)/255)*x + L
        return ((H - L) / 255.f) * x + L;
    }


    /**
    * inverse transfer function from a single projector cell
    * calculate the corresponding value from the Fx-value
    */
    GLfloat inverseProjectorTF(GLfloat Fx, GLfloat cellMedianL, GLfloat cellMedianH) {
        //inv_f(x) = R_min + (x - P_min)/(P_max-P_min ) * (R_max - R_min )
        //oder inv_f(x) = (x - L) * 255 / (H-L)
        //inverse color transfer function of one projector
        //TODO: more precise 0, 85,170,255..
        return ((Fx - cellMedianL) * 255.f) / (cellMedianH - cellMedianL);
    }

    //texture3d R = > x = CELL_COUNT; y = CELL_COUNT; z = VALUE_COUNT
    //	texture3d G
    //	texture3d B
    //convert color value range from 0-255 to 0-1
    void calcColorValue(std::vector<GLfloat>& colorLookUpTable, int maxFromMin, int minFromMax, int cellMedianL, int cellMedianH) {
        for (int v = 0; v < VALUE_COUNT; v++) {
            auto Fx = transferFunction(static_cast<float>(v), static_cast<float>(maxFromMin), static_cast<float>(minFromMax));
            //GLfloat colorValue = inverseProjectorTF(Fx, gccd.maxValues[0][x][y], gccd.maxValues[c][x][y]);
            GLfloat colorValue = inverseProjectorTF(Fx, static_cast<float>(cellMedianL), static_cast<float>(cellMedianH));
            colorLookUpTable[v] = colorValue;
        }
    }

    /**
    * calculate the LookUpTableData from the ColorCalibData
    */
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

    LookUpTableData calcColorLookUpTableDataSingle(const FWConfiguration& config, unsigned int projectorId)
    {
        std::vector<ColorCalibData> colorCalibData;
        loadColorCalibData(config, colorCalibData);

        std::vector<int> maxFromMinValues(ColorValues.size(), 0);
        std::vector<int> minFromMaxValues(ColorValues.size(), 255);
        calcMinMaxColorValue(colorCalibData, maxFromMinValues, minFromMaxValues);
        const auto& gccd = colorCalibData[projectorId];
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
                    } else {
                        calcColorValue(lutData.LookUpTables[lutC][x][y], maxFromMinValues[0], minFromMaxValues[c], gccd.medValues[0][x][y], gccd.medValues[c][x][y]);
                    }
                    //lutData.LookUpTables[lutC][x][y] = colorLookUpTable; 
                }
            }
        }
        return lutData;
    }

    GLfloat calcAvgValue(const std::vector<GLfloat>& values) {
        float avgVal = 0;
        if (!values.empty()) {
            for (GLfloat val : values) {
                avgVal += val;
            }
            return avgVal / (float)values.size();
        }
        return 0;
    }

    /**
    * find the next neighbor from a cell in the LookUpTableData which value is greater or equal zero
    */
    GLfloat getNextNeighborValueGraterThanZero(const pro_cal::LookUpTableData& lookUpTableData, const int c, const int x, const int y, const int v) {
        std::vector<GLfloat> neighbors;
        int cc = pro_cal::CELL_COUNT;
        for (int i = 1; i < pro_cal::CELL_COUNT; i++) {
            if ((x + i) < cc && lookUpTableData.LookUpTables[c][x + i][y][v] >= 0.f) { //right
                neighbors.push_back(lookUpTableData.LookUpTables[c][x + i][y][v]);
            }
            if ((x - i) > 0 && lookUpTableData.LookUpTables[c][x - i][y][v] >= 0.f) { //left
                neighbors.push_back(lookUpTableData.LookUpTables[c][x - i][y][v]);
            }
            if ((y + i) < cc && lookUpTableData.LookUpTables[c][x][y + i][v] >= 0.f) { //top 
                neighbors.push_back(lookUpTableData.LookUpTables[c][x][y + i][v]);
            }
            if ((y - i) > 0 && lookUpTableData.LookUpTables[c][x][y - i][v] >= 0.f) { //bot
                neighbors.push_back(lookUpTableData.LookUpTables[c][x][y - i][v]);
            }
            if ((x + i) < cc && (y + i) < cc && lookUpTableData.LookUpTables[c][x + i][y + i][v] >= 0.f) { //top_right
                neighbors.push_back(lookUpTableData.LookUpTables[c][x + i][y + i][v]);
            }
            if ((x + i) < cc && (y - i) > 0 && lookUpTableData.LookUpTables[c][x + i][y - i][v] >= 0.f) { //bot_right
                neighbors.push_back(lookUpTableData.LookUpTables[c][x + i][y - i][v]);
            }
            if ((x - i) > 0 && (y + i) < cc && lookUpTableData.LookUpTables[c][x - i][y + i][v] >= 0.f) { //top_left
                neighbors.push_back(lookUpTableData.LookUpTables[c][x - i][y + i][v]);
            }
            if ((x - i) > 0 && (y - i) > 0 && lookUpTableData.LookUpTables[c][x - i][y - i][v] >= 0.f) { //bot_left
                neighbors.push_back(lookUpTableData.LookUpTables[c][x - i][y - i][v]);
            }
            if (!neighbors.empty()) {
                return calcAvgValue(neighbors);
            }
        }
        return 0;
    }

    /**
    * calculate the median from the values 
    */
    int calcMedian(std::vector<byte> values) {
        if (!values.empty()) { //TODO: test 10.11.16
            if (values.size() > 2) {
                std::sort(values.begin(), values.end());
                int midIdx = static_cast<int>(values.size() / 2);
                return ((values.size() % 2) == 0) ? (values[midIdx] + values[midIdx + 1]) / 2 : values[(values.size() + 1) / 2];
            }
            else if (values.size() == 2) {
                return (values[0] + values[1]) / 2;
            }
            else {
                return values[0];
            }
        }
        return -1;
    }

    /**
    * calculate the median from the LookUpTableData of each color value
    */
    std::vector<glm::vec3> calcMediansFromFilteredLookUpTable(const pro_cal::LookUpTableData& lookUpTableData) {
        std::vector<glm::vec3> medians(VALUE_COUNT);
        for (int v = 0; v < VALUE_COUNT; v++) {
            std::vector<byte> chBuffers0;
            std::vector<byte> chBuffers1;
            std::vector<byte> chBuffers2;
            for (int y = CELL_COUNT - 1; y >= 0; y--) {  //TODO: test 05.11.16
                for (int x = 0; x < CELL_COUNT; x++) {
                    glm::vec3 colors;
                    for (int c = 0; c < 3; c++) {
                        colors[c] = lookUpTableData.LookUpTables[c][x][y][v];
                    }
                    if (colors[0] > 0 && colors[1] > 0 && colors[2] > 0) {
                        chBuffers0.push_back(static_cast<unsigned char>(colors[0]));
                        chBuffers1.push_back(static_cast<unsigned char>(colors[1]));
                        chBuffers2.push_back(static_cast<unsigned char>(colors[2]));
                    }
                }
            }
            medians[v] = glm::vec3(calcMedian(chBuffers0), calcMedian(chBuffers1), calcMedian(chBuffers2));
        }
        return medians;
    }

    /**
    * converts LookUpTableData-Object to std::vector<glm::vec3> data structure
    */
    std::vector<glm::vec3> lookUpTableDataToOpenGLTextureDataVec3f(const pro_cal::LookUpTableData& lookUpTableData) {
        int lutTexSize = pro_cal::CELL_COUNT * pro_cal::CELL_COUNT * pro_cal::VALUE_COUNT; //256 * 256 * 256;
        std::vector<glm::vec3> chBuffers(lutTexSize);
        std::vector<glm::vec3> medians = calcMediansFromFilteredLookUpTable(lookUpTableData); //TODO: test 10.11.16
        int i = 0;
        //for GL_TEXTURE_2D_ARRAY	
        for (int v = 0; v < pro_cal::VALUE_COUNT; v++) {
            //glm::vec3 avgColors(0);
            for (int y = pro_cal::CELL_COUNT - 1; y >= 0; y--) {  //TODO: test 05.11.16
                for (int x = 0; x < pro_cal::CELL_COUNT; x++) {
                    glm::vec3 colors;
                    for (int c = 0; c < 3; c++) {
                        GLfloat value = lookUpTableData.LookUpTables[c][x][y][v];
                        if (value < 0) { //TODO: test 10.11.16
                            GLfloat neighborValue = getNextNeighborValueGraterThanZero(lookUpTableData, c, x, y, v);
                            value = (neighborValue + medians[v][c]) / 2.f;
                            //value = (neighborValue + (medians[v][c] * 2.f)) / 3.f;
                        }
                        colors[c] = value;
                    }
                    chBuffers[i++] = colors / 255.f;
                }
            }
        }
        return chBuffers;
    }
    
    std::vector<int> colorCalibDataToIntegerVector(const std::vector<ColorCalibData>& colorCalibData) {
        int lutTexSize = static_cast<int>(colorCalibData.size() * ColorValues.size()) * pro_cal::CELL_COUNT * pro_cal::CELL_COUNT; //256 * 256 * 256;
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
        int projectorCount = (static_cast<int>(ccdData.size() / ColorValues.size()) / pro_cal::CELL_COUNT) / pro_cal::CELL_COUNT;
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

    /**
    * converts std::vector<cv::Vec3f data structure to LookUpTableData-Object
    */
    void vec3fVectorToLookUpTables(const int colorIdx, const std::vector<cv::Vec3f> lookUpTableVec3f, LookUpTableData& gridLut) {
        for (cv::Vec3f vec : lookUpTableVec3f) {
            int x = static_cast<int>(vec[0]), y = static_cast<int>(vec[1]);
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
    void loadColorLutData(const FWConfiguration& config, std::vector<LookUpTableData>& lookUpTableData) {
        for (int i = 0; i < lookUpTableData.size(); i++) {
            LookUpTableData gridLutData;
            for (int c = 0; c < 3; c++) {
                std::vector<cv::Vec3f> lookUpTableFloat;
                readVectorOfVec3f(config.colorCalibrationData_, "LookUpTableData_" + std::to_string(i) + "_" + std::to_string(c), lookUpTableFloat);
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
                    lookUpTableVec3f.push_back(cv::Vec3f(static_cast<float>(x), static_cast<float>(y), val));
                }
            }
        }
    }
    void colorCalibDataToVec3fVector(const int colorIdx, const ColorCalibData& ccd, std::vector<cv::Vec3f>& colorCalibDataVec3f) {
        for (int x = 0; x < CELL_COUNT; x++) {
            for (int y = 0; y < CELL_COUNT; y++) {
                float val = static_cast<float>(ccd.medValues[colorIdx][x][y]);
                colorCalibDataVec3f.push_back(cv::Vec3f(static_cast<float>(x), static_cast<float>(y), val));
            }
        }
    }

    /**
    * converts std::vector<cv::Vec3f data structure to ColorCalibData-Object
    */
    void vec3fVectorToColorCalibData(const int colorIdx, const std::vector<cv::Vec3f> lookUpTableVec3f, ColorCalibData& ccd) {
        ccd.medValues.resize(ColorValues.size());
        for (cv::Vec3f vec : lookUpTableVec3f) {
            int x = static_cast<int>(vec[0]), y = static_cast<int>(vec[1]);
            float val = vec[2];
            if (ccd.medValues[colorIdx].empty()) {
                ccd.medValues[colorIdx].resize(CELL_COUNT);
            }
            if (ccd.medValues[colorIdx][x].empty()) {
                ccd.medValues[colorIdx][x].resize(CELL_COUNT);
            }
            ccd.medValues[colorIdx][x][y] = static_cast<int>(val);
        }
    }

    /**
    * load the color calibration data from the data\ColorCalibData.xml file
    */
    void loadColorCalibData(const FWConfiguration& config, std::vector<ColorCalibData>& colorCalibData) {
        auto nodeExists = true;
        unsigned int projectorId = 0;
        while (nodeExists) {
            ColorCalibData ccData;
            for (auto c = 0U; c < ColorValues.size(); c++) {
                std::vector<cv::Vec3f> colorCalibDataVec3f;
                try {
                    readVectorOfVec3f(config.colorCalibrationData_, "ColorCalibData_" + std::to_string(projectorId) + "_" + std::to_string(c), colorCalibDataVec3f);
                } catch (std::out_of_range e) {
                    nodeExists = false;
                    break;
                }
                vec3fVectorToColorCalibData(c, colorCalibDataVec3f, ccData);
            }
            if (nodeExists) {
                colorCalibData.push_back(ccData);
                ++projectorId;
            }
        }

        /*for (int i = 0; i < colorCalibData.size(); i++) {
            LookUpTableData gridLutData;
            for (int c = 0; c < ColorValues.size(); c++) {
                std::vector<cv::Vec3f> colorCalibDataVec3f;
                readVectorOfVec3f(config.colorCalibrationData_, "ColorCalibData_" + std::to_string(i) + "_" + std::to_string(c), colorCalibDataVec3f);
                vec3fVectorToColorCalibData(c, colorCalibDataVec3f, colorCalibData[i]);
            }
        }

        std::vector<cv::Vec3f> colorCalibDataVec3f;
        readVectorOfVec3f(config.colorCalibrationData_, "ColorCalibData_" + std::to_string(5) + "_" + std::to_string(0), colorCalibDataVec3f);*/
    }
}