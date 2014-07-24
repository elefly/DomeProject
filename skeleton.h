#ifndef SKELETON_H
#define SKELETON_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include "domecamera.h"

using namespace std;
using namespace cv;

struct Joint
{
    Point3f CameraSpacePosition;
    Point2f ColorSpacePosition;
    Point2f DepthSpacePosition;
    int TrackingState;
};

struct Body
{
    Joint joints[25];
    int leftHandState;
    int rightHandState;
};

const int connectionTable[24][2]={{4, 3},{3, 21},{21,2},{2,1},{21, 9},{21, 5},{1, 17},{1, 13},{9, 10},{10, 11},{11, 12},{12, 24},
                             {11, 25},{5, 6},{6, 7},{7, 8},{8, 22},{7, 23},{17, 18},{18, 19},{19, 20},{13, 14},{14, 15},{15, 16}};

class Skeleton
{
public:
    Skeleton();
    ~Skeleton();
    int ReadSkeletonFromTxt(char *txtFileName);
    //void projectToAllCamera(Mat joint_world, Dome dome);
    //void projectToPanel(Mat joint_world, Dome dome, int panIdx);
    Mat jointToKinectColorCameraSpace(Mat cameraMatrix, Mat distCoeffs);//3D points from depth camera to color camera
    Mat jointToWorldSpace(Mat joint_3d, Mat rVec, Mat tVec, float scaleFactor);//3D points from kinect color camera to world space
    //Mat jointToCameraSpace(vector<Point3f> joint_3d, vector<Point2f> color_2d, Mat cameraMatrix);//3D points from world space to camera space
    vector<Point2f> projectToCamera(Mat joint_world, DomeCamera camera);//project 3D points on image plane
    Point2f distortPoint(Point2f pt, double k1);//distort image points
    void drawSkeletonOnImage(vector<Point2f> skeleton, Mat image);//draw skeleon on image;
    vector< vector<Point2f> > projectToPanel(Mat joint_world, int panelIndex);
    //Mat drawSkeletonOnPanel(vector<Point2f> skeleton, Mat image);


private:
    vector<Body> m_vBodyData;
    int m_nBodyCount;
};

#endif // SKELETON_H
