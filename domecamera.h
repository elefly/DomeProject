#ifndef DOMECAMERA_H
#define DOMECAMERA_H

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

enum CameraType
{
    CameraType_HD = 0,
    CameraType_VGA = 1,
    CameraType_UNKOWN = -1
};

class DomeCamera
{
public:
    DomeCamera();
    DomeCamera(int panelIndex, int cameraIndex);
    ~DomeCamera();
    void getIndex(int* pPanelIndex, int* pCameraIndex);
    bool setIndex(int panelIndex, int cameraIndex);
    CameraType getCameraType();
    bool setCameraIndex(int camIdx);
//    void setCameraType(CameraType type);
    bool isInitialized();
    bool isValidIndex(int panelIndex, int cameraIndex);

public:
    Mat cameraMatrix;
    Mat rVec;
    Mat tVec;
    Mat rMat;
    Mat distCoeffs;
    Mat image;

private:


private:
    int panIdx;
    int camIdx;
    CameraType cameraType;

};

#endif // DOMECAMERA_H
