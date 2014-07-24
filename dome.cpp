#include "dome.h"

Dome::Dome():
    m_nCameraNum(0),
    m_nVGACameraNum(0),
    m_nHDCameraNum(0)
{

}

//! initialize dome cameras, set camera numbers
//! VGA camera number is set to 480 when initialing
//! HD camera number can vary
void Dome::initDome(int hdCameraNum)
{
    if (getCameraNumber() == 0)
    {
        for (int panelIndex = 1; panelIndex <= 20; panelIndex ++)
            for (int cameraIndex = 1; cameraIndex <= 24; cameraIndex ++)
                m_cameraList.insert(pair<INDEX, DomeCamera>(INDEX(panelIndex, cameraIndex),DomeCamera(panelIndex, cameraIndex)));

        for (int cameraIndex = 0; cameraIndex < hdCameraNum; cameraIndex ++)
            m_cameraList.insert(pair<INDEX, DomeCamera>(INDEX(0, cameraIndex),DomeCamera(0, cameraIndex)));

        m_nVGACameraNum = 480;
        m_nHDCameraNum = hdCameraNum;
        m_nCameraNum = m_nVGACameraNum + m_nHDCameraNum;

        cout<<"initialized "<<getVGACameraNumber()<<" VGA and "<<getHDCameraNumber()<<" HD cameras"<<endl;
    }
}

bool Dome::addCamera(int panelIndex, int cameraIndex)
{
    if (isValidIndex(panelIndex, cameraIndex) && !exist(panelIndex, cameraIndex))
    {
        m_cameraList.insert(pair<INDEX, DomeCamera>(INDEX(panelIndex, cameraIndex),DomeCamera(panelIndex, cameraIndex)));
        if (panelIndex)
            m_nVGACameraNum ++;
        else
            m_nHDCameraNum ++;
        m_nCameraNum ++;
        return true;
    }
    else
        return false;
}

int Dome::addCamera(vector<INDEX> cameraList)
{
    int cameraNumber = 0;
    for (int i = 0; i < cameraList.size(); i++)
    {
        if (addCamera(cameraList.at(i).first, cameraList.at(i).second))
            cameraNumber ++;
    }
    return cameraNumber;
}

bool Dome::removeCamera(int panelIndex, int cameraIndex)
{
    if (!isValidIndex(panelIndex, cameraIndex))
        return false;

    CAMERALIST::iterator it;
    it = m_cameraList.find(INDEX(panelIndex, cameraIndex));
    if (it == m_cameraList.end())
        return false;
    m_cameraList.erase(it);
    if (panelIndex)
        m_nVGACameraNum --;
    else
        m_nHDCameraNum --;
    m_nCameraNum --;
    return true;
}

int Dome::removeCamera(vector<INDEX> cameraList)
{
    int cameraNumber = 0;
    for (int i = 0; i < cameraList.size(); i++)
    {
        if (removeCamera(cameraList.at(i).first, cameraList.at(i).second))
            cameraNumber ++;
    }
    return cameraNumber;
}

//! access dome camera by index
DomeCamera Dome::operator() (int panelIndex, int cameraIndex)
{
    CAMERALIST::iterator it;
    it = m_cameraList.find(INDEX(panelIndex, cameraIndex));
    if (it == m_cameraList.end())
    {
        DomeCamera empty;
        return empty;
    }

    return it->second;
}

bool Dome::isValidIndex(int panelIndex, int cameraIndex)
{
    if (panelIndex < 0 || panelIndex >20 || cameraIndex < 0)
        return false;

    if (panelIndex > 0 && cameraIndex > 24)
        return false;

    return true;
}

bool Dome::exist(int panelIndex, int cameraIndex)
{
    if (!isValidIndex(panelIndex, cameraIndex))
        return false;

    CAMERALIST::iterator it;
    it = m_cameraList.find(INDEX(panelIndex, cameraIndex));

    if (it == m_cameraList.end())
        return false;
    return true;
}

int Dome::getCameraNumber()
{
    return m_nCameraNum;
}

int Dome::getVGACameraNumber()
{
    return m_nVGACameraNum;
}

int Dome::getHDCameraNumber()
{
    return m_nHDCameraNum;
}

void Dome::empty()
{
    m_cameraList.clear();
    m_nCameraNum = 0;
    m_nVGACameraNum = 0;
    m_nHDCameraNum = 0;
}

void Dome::readIntrinsic(int panelIndex, int cameraIndex, char* pDataPath)
{
    if (isValidIndex(panelIndex, cameraIndex))
    {
        CAMERALIST::iterator it;
        it = m_cameraList.find(INDEX(panelIndex, cameraIndex));
        if (it != m_cameraList.end())
        {
            char* insFileName;
            insFileName = new char[256];
            sprintf(insFileName, "%s%02d_%02d.txt", pDataPath, panelIndex, cameraIndex);
            ifstream file(insFileName, ios_base::in);
            if (file.is_open())
            {
                string str;
                int count = 0;
                Mat cameraMat(3, 3, CV_32FC1);
                float r;
                while(file>>str)
                {
                    if (count==9)
                        r = static_cast<float>(atof(str.c_str()))/pow(cameraMat.ptr<float>(0)[0],2);
                    else
                        cameraMat.ptr<float>(0)[count] = static_cast<float>(atof(str.c_str()));
                    count ++;
                }
                it->second.cameraMatrix = cameraMat.t();
                it->second.distCoeffs = Mat(1, 1, CV_32FC1, Scalar::all(r));
                file.close();
                delete []insFileName;
            }
        }
    }
}

void Dome::readIntrinsic(INDEX index, DomeCamera *camera, char *pDataPath)
{
    char* insFileName = new char[256];
    sprintf(insFileName, "%s%02d_%02d.txt", pDataPath, index.first, index.second);
    ifstream file(insFileName, ios_base::in);
    if (file.is_open())
    {
        string str;
        int count = 0;
        Mat cameraMat(3, 3, CV_32FC1);
        float r;
        while(file>>str)
        {
            if (count==9)
                r = static_cast<float>(atof(str.c_str()))/pow(cameraMat.ptr<float>(0)[0],2);
            else
                cameraMat.ptr<float>(0)[count] = static_cast<float>(atof(str.c_str()));
            count ++;
        }
        camera->cameraMatrix = cameraMat.t();
        camera->distCoeffs = Mat(1, 1, CV_32FC1, Scalar::all(r));
        file.close();
        delete []insFileName;
    }
}

void Dome::readExtrinsic(int panelIndex, int cameraIndex, char *pDataPath)
{
    if (isValidIndex(panelIndex, cameraIndex))
    {
        CAMERALIST::iterator it;
        it = m_cameraList.find(INDEX(panelIndex, cameraIndex));
        if (it != m_cameraList.end())
        {
            char* extFileName;
            extFileName = new char[256];
            sprintf(extFileName,"%s%02d_%02d_ext.txt", pDataPath, panelIndex, cameraIndex);
            ifstream file(extFileName, ios_base::in);
            if (file.is_open())
            {
                Mat rVec = Mat::zeros(3, 1, CV_32FC1);
                Mat tVec = Mat::zeros(3, 1, CV_32FC1);
                string str;
                int count = 0;
                while (file>>str)
                {
                    if (count<3)
                        rVec.ptr<float>(count)[0]=static_cast<float>(atof(str.c_str()));
                    else
                        tVec.ptr<float>(count-3)[0]=static_cast<float>(atof(str.c_str()));
                    count++;
                }
                it->second.rVec = rVec;
                it->second.tVec = tVec;
                Rodrigues(rVec, it->second.rMat);
                it->second.rMat.convertTo(it->second.rMat, CV_32FC1);
                file.close();
                delete []extFileName;
            }
        }
    }
}

void Dome::readExtrinsic(INDEX index, DomeCamera *camera, char *pDataPath)
{
    char* extFileName;
    extFileName = new char[256];
    sprintf(extFileName,"%s%02d_%02d_ext.txt", pDataPath, index.first, index.second);
    ifstream file(extFileName, ios_base::in);
    if (file.is_open())
    {
        Mat rVec = Mat::zeros(3, 1, CV_32FC1);
        Mat tVec = Mat::zeros(3, 1, CV_32FC1);
        string str;
        int count = 0;
        while (file>>str)
        {
            if (count<3)
                rVec.ptr<float>(count)[0]=static_cast<float>(atof(str.c_str()));
            else
                tVec.ptr<float>(count-3)[0]=static_cast<float>(atof(str.c_str()));
            count++;
        }
        camera->rVec = rVec;
        camera->tVec = tVec;
        Rodrigues(rVec, camera->rMat);
        camera->rMat.convertTo(camera->rMat, CV_32FC1);
        file.close();
        delete []extFileName;
    }
}

void Dome::readCameraParameter(int panelIndex, int cameraIndex, char *pDataPath)
{
    readIntrinsic(panelIndex, cameraIndex, pDataPath);
    readExtrinsic(panelIndex, cameraIndex, pDataPath);
}

void Dome::readCameraParameter(INDEX index, DomeCamera *camera, char *pDataPath)
{
    readIntrinsic(index, camera, pDataPath);
    readExtrinsic(index, camera, pDataPath);
}

void Dome::readCameraParameter(char *pDataPath)
{
    CAMERALIST::iterator it;
    for (it = m_cameraList.begin(); it != m_cameraList.end(); it ++)
    {
        readCameraParameter(it->first, &(it->second), pDataPath);
    }
}

void Dome::readImage(int panelIndex, int cameraIndex, char *pDataPath, char *pFormat, int frameIndex)
{
    if (isValidIndex(panelIndex, cameraIndex))
    {
        CAMERALIST::iterator it;
        it = m_cameraList.find(INDEX(panelIndex, cameraIndex));
        if (it != m_cameraList.end())
        {
            char* imageName = new char[256];
            sprintf(imageName, "%s%08d_%02d_%02d.%s", pDataPath, frameIndex, panelIndex, cameraIndex, pFormat);
            it->second.image = imread(imageName, CV_LOAD_IMAGE_GRAYSCALE);
            delete []imageName;
        }
    }

}

void Dome::readImage(INDEX index, DomeCamera *camera, char *pDataPath, char *pFormat, int frameIndex)
{
    char* imageName = new char[256];
    sprintf(imageName, "%s%08d_%02d_%02d.%s", pDataPath, frameIndex, index.first, index.second, pFormat);
    camera->image = imread(imageName, CV_LOAD_IMAGE_GRAYSCALE);
    delete []imageName;
}

void Dome::readImage(char *pDataPath, char *pFormat, int frameIndex)
{
    CAMERALIST::iterator it;
    for (it = m_cameraList.begin(); it != m_cameraList.end(); it ++)
    {
        readImage(it->first, &(it->second), pDataPath, pFormat, frameIndex);
    }
}
