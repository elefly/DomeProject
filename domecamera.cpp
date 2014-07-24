#include "domecamera.h"

DomeCamera::DomeCamera() :
    panIdx(-1),
    camIdx(-1),
    cameraType(CameraType_UNKOWN)
{

}

DomeCamera::DomeCamera(int panelIndex, int cameraIndex)
{
    assert(isValidIndex(panelIndex, cameraIndex));
    panIdx = panelIndex;
    camIdx = cameraIndex;
    cameraType = panIdx > 0 ? CameraType_VGA : CameraType_HD;
}

DomeCamera::~DomeCamera()
{

}

void DomeCamera::getIndex(int *pPanelIndex, int *pCameraIndex)
{
    pPanelIndex = &panIdx;
    pCameraIndex = &camIdx;
}

CameraType DomeCamera::getCameraType()
{
    return cameraType;
}

bool DomeCamera::setIndex(int panelIndex, int cameraIndex)
{
    if (isValidIndex(panelIndex, cameraIndex))
    {
        panIdx = panelIndex;
        camIdx = cameraIndex;
        if (panIdx)
            cameraType = CameraType_VGA;
        else
            cameraType = CameraType_HD;
        return true;
    }
    return false;
}

bool DomeCamera::isValidIndex(int panelIndex, int cameraIndex)
{
    if (panelIndex < 0 || panelIndex >20 || cameraIndex < 0)
        return false;

    if (panelIndex > 0 && cameraIndex > 24)
        return false;

    return true;
}

bool DomeCamera::isInitialized()
{
    if (cameraType == CameraType_UNKOWN)
        return false;
    return true;
}
