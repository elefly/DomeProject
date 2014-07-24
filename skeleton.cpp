#include "skeleton.h"

Skeleton::Skeleton():
    m_nBodyCount(0)
{

}

Skeleton::~Skeleton()
{

}

//! read skeleton from txt file
int Skeleton::ReadSkeletonFromTxt(char* txtFileName)
{
    if(m_vBodyData.empty())
        m_vBodyData.clear();
    ifstream file(txtFileName, ios_base::in);
    string str;
    int count = 0;
    float *data;
    data = new float[205*6];
    while(file>>str)
    {
        data[count]=::atof(str.c_str());
        count++;
    }
    file.close();

    m_nBodyCount = count/202;
    Body body;
    for (int cnt=0; cnt<m_nBodyCount; cnt++)
    {
        for (int i=0; i<25; i++)
        {
            body.joints[i].CameraSpacePosition.x = data[cnt*202+i*8];
            body.joints[i].CameraSpacePosition.y = data[cnt*202+i*8+1];
            body.joints[i].CameraSpacePosition.z = data[cnt*202+i*8+2];
            body.joints[i].ColorSpacePosition.x = data[cnt*202+i*8+3];
            body.joints[i].ColorSpacePosition.y = data[cnt*202+i*8+4];
            body.joints[i].DepthSpacePosition.x = data[cnt*202+i*8+5];
            body.joints[i].DepthSpacePosition.y = data[cnt*202+i*8+6];
            body.joints[i].TrackingState = static_cast<int>(data[cnt*202+i*8+7]);
        }
        body.leftHandState = static_cast<int>(data[cnt*202+200]);
        body.rightHandState = static_cast<int>(data[cnt*202+201]);
        m_vBodyData.push_back(body);
    }
    delete []data;
    return m_nBodyCount;
}

Mat Skeleton::jointToKinectColorCameraSpace(Mat cameraMatrix, Mat distCoeffs)//consider to pass in DomeCamera object
{
    vector<Point3f> skeleton_3d;
    vector<Point2f> skeleton_2d;
    for (int cnt=0; cnt<m_nBodyCount; cnt++)
    {
        for (int i=0; i<25; i++)
        {
            skeleton_3d.push_back(Point3f(-m_vBodyData.at(cnt).joints[i].CameraSpacePosition.x, -m_vBodyData.at(cnt).joints[i].CameraSpacePosition.y,
                                          m_vBodyData.at(cnt).joints[i].CameraSpacePosition.z));
            skeleton_2d.push_back(Point2f(1920 - m_vBodyData.at(cnt).joints[i].ColorSpacePosition.x, m_vBodyData.at(cnt).joints[i].ColorSpacePosition.y));
        }
    }

    Mat rvec, tvec, rmat;
    solvePnP(skeleton_3d, skeleton_2d, cameraMatrix, distCoeffs, rvec, tvec, false, CV_EPNP);
    Rodrigues(rvec,rmat);
    Mat joint3d = Mat(skeleton_3d).reshape(1);
    rmat.convertTo(rmat, CV_32FC1); //CV_64FC1 to CV_32FC1, because joint3d is CV_32FC1
    tvec.convertTo(tvec, CV_32FC1);
    tvec=repeat(tvec,1,m_nBodyCount*25);
    return rmat * joint3d.t() + tvec;
}

Mat Skeleton::jointToWorldSpace(Mat joint_3d, Mat rVec, Mat tVec, float scaleFactor)
{
    Mat rmat;
    Rodrigues(rVec,rmat);
    rmat.convertTo(rmat, CV_32FC1);
    tVec = repeat(tVec,1,m_nBodyCount*25);
    Mat jointInWorld = rmat.t() * (scaleFactor * joint_3d - tVec);
    return jointInWorld;
}


vector<Point2f> Skeleton::projectToCamera(Mat joint_world, DomeCamera camera)
{
    Mat imagePoint, distCoeffs;
    vector<Point2f> dist_points, imagePointVec;
    if (camera.distCoeffs.total()>1)
        distCoeffs = camera.distCoeffs;
    projectPoints(joint_world.t(), camera.rVec, camera.tVec, camera.cameraMatrix, distCoeffs, imagePoint);
    //cout<<imagePoint<<endl;
    imagePoint.copyTo(imagePointVec);
    if (camera.distCoeffs.total() == 1)
    {
        Point2f cameraCenter(camera.cameraMatrix.ptr<float>(0)[2], camera.cameraMatrix.ptr<float>(1)[2]);
        for (int i=0; i<imagePoint.rows; i++)
        {
            dist_points.push_back(distortPoint(imagePointVec.at(i)-cameraCenter, ((float*)camera.distCoeffs.data)[0])+cameraCenter);
        }
        return dist_points;
    }
    return imagePointVec;
}

//vector< vector<Point2f> > Skeleton::projectToPanel(Mat joint_world, int panelIndex)
//{

//}

Point2f Skeleton::distortPoint(Point2f pt, double k1)
{
    if (k1 == 0)
        return pt;
    const double t2 = pt.y*pt.y;
    const double t3 = t2*t2*t2;
    const double t4 = pt.x*pt.x;
    const double t7 = k1*(t2+t4);
    if (k1 > 0)
    {
        const double t8 = 1.0/t7;
        const double t10 = t3/(t7*t7);
        const double t14 = sqrt(t10*(0.25+t8/27.0));
        const double t15 = t2*t8*pt.y*0.5;
        const double t17 = pow(t14+t15,1.0/3.0);
        const double t18 = t17-t2*t8/(t17*3.0);
        return Point2f(t18*pt.x/pt.y, t18);
    }
    else
    {
        const double t9 = t3/(t7*t7*4.0);
        const double t11 = t3/(t7*t7*t7*27.0);
        const std::complex<double> t12 = t9+t11;
        const std::complex<double> t13 = sqrt(t12);
        const double t14 = t2/t7;
        const double t15 = t14*pt.y*0.5;
        const std::complex<double> t16 = t13+t15;
        const std::complex<double> t17 = pow(t16,1.0/3.0);
        const std::complex<double> t18 = (t17+t14/
        (t17*3.0))*std::complex<double>(0.0,sqrt(3.0));
        const std::complex<double> t19 = -0.5*(t17+t18)+t14/(t17*6.0);
        return Point2f(t19.real()*pt.x/pt.y, t19.real());
    }
}

void Skeleton::drawSkeletonOnImage(vector<Point2f> skeleton, Mat image)
{
    for (int cnt=0; cnt<m_nBodyCount; cnt++)
    {
        for(int i=0; i<24; i++)
        {
            line(image,skeleton.at(connectionTable[i][0]-1),skeleton.at(connectionTable[i][1]-1), Scalar(0,0,255), 2);
        }
    }
}


