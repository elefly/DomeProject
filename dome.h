#ifndef DOME_H
#define DOME_H

#include "domecamera.h"
#include <fstream>

using namespace cv;
using namespace std;

typedef pair<int, int> INDEX;
typedef map<INDEX, DomeCamera> CAMERALIST;

class Dome
{
public:
    Dome();
    void initDome(int hdCameraNum = 30);
    bool removeCamera(int panelIndex, int cameraIndex);
    int removeCamera(vector<INDEX> cameraList);
    bool addCamera(int panelIndex, int cameraIndex);
    int addCamera(vector<INDEX> cameraList);
    DomeCamera operator() (int pandelIndex, int cameraIndex);
    bool isValidIndex(int panelIndex, int cameraIndex);
    bool exist(int panelIndex, int cameraIndex);
    bool isEmpty();
    void empty();
    int getCameraNumber();
    int getVGACameraNumber();
    int getHDCameraNumber();
    void readIntrinsic(int panelIndex, int cameraIndex, char *pDataPath);
    void readIntrinsic(INDEX index, DomeCamera *camera, char *pDataPath);
    void readExtrinsic(int panelIndex, int cameraIndex, char *pDataPath);
    void readExtrinsic(INDEX index, DomeCamera *camera, char *pDataPath);
    void readCameraParameter(int panelIndex, int cameraIndex, char *pDataPath);
    void readCameraParameter(INDEX index, DomeCamera *camera, char *pDataPath);
    void readCameraParameter(char *pDataPath);
    void readImage(int panelIndex, int cameraIndex, char *pDataPath, char *pFormat, int frameIndex);
    void readImage(INDEX index, DomeCamera *camera, char *pDataPath, char *pFormat, int frameIndex);
    void readImage(char *pDataPath, char *pFormat, int frameIndex);

public:
    float m_fScaleFactor;//metre to world

private:
    int m_nCameraNum;
    int m_nVGACameraNum;
    int m_nHDCameraNum;
    CAMERALIST m_cameraList;
//    CAMERALIST m_VGACameraList;
//    CAMERALIST m_HDCameraList;
};

#endif // DOME_H
