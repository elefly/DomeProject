#include "skeleton.h"
#include "dome.h"
//#include <opencv2/ocl/ocl.hpp>
//#include <opencv2/ocl/matrix_operations.hpp>
//#include <opencv2/nonfree/ocl.hpp>

//#include <QCoreApplication>
//using namespace cl;

vector<cl::Platform> platforms;
vector<cl::Device> devices;
cl::Context context;
cl::CommandQueue queue;
cl::Program::Sources source;
cl::Program program;
vector<cl::Kernel> kernels;
string parameterName;
cl_long parameterValue;

void drawSkeletonOnPanels(Mat jointInWorld, Skeleton sk, Dome dm, char* pInputImagePath, char* pOutputPath, int frameIndex)
{
    char *pInputImage = new char[256];
    char *pOutputImage = new char[256];
    Mat img;
    vector<Point2f> imagePoints;
    DomeCamera camera;
    for (int panIdx = 1; panIdx <= 20; panIdx ++)
    {
        Mat panelImage(480*4, 640*6, CV_8UC3, Scalar::all(0));
        sprintf(pOutputImage, "");
        sprintf(pOutputImage, "%sve%02d.jpg", pOutputPath, panIdx);
        for (int camIdx = 1; camIdx <= 24; camIdx ++)
        {
            camera = dm(panIdx, camIdx);
            if (!camera.isInitialized())
                continue;
            sprintf(pInputImage, "");
            sprintf(pInputImage, "%s%08d_%02d_%02d.png", pInputImagePath, frameIndex, panIdx, camIdx);
            img = imread(pInputImage);
            if (!imagePoints.empty())
                imagePoints.clear();
            imagePoints = sk.projectToCamera(jointInWorld,dm(panIdx,camIdx));
            sk.drawSkeletonOnImage(imagePoints, img);

            int row = (camIdx-1)/6;
            int col = camIdx%6;
            if(col==0)
                col = 6;
            int x = (col-1)*640;
            int y = row*480;
            Mat roi(panelImage,Rect(x,y,640,480));
            img.copyTo(roi);

//            imshow("", panelImage);
//            waitKey(0);
        }
        imwrite(pOutputImage, panelImage);
    }
}

void createKernel(std::string kernelFileName)
{
    try
    {
        std::ifstream programFile(kernelFileName.c_str());
        std::string programString(std::istreambuf_iterator<char>(programFile),
                                  (std::istreambuf_iterator<char>()));
        source = cl::Program::Sources(1, std::make_pair(programString.c_str(),
                                                          programString.length()+1));

        program = cl::Program(context, source);
        program.build(devices);
        program.createKernels(&kernels);

        if(1)
        {
            for(int _mI=0; _mI<kernels.size(); _mI++)
            {
                parameterName = kernels[_mI].getInfo<CL_KERNEL_FUNCTION_NAME>();
                std::cout << "Kernel: " << parameterName << std::endl;
            }
        }
    }
    catch(cl::Error err)
    {
        cout<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout<<err.what()<<"Error code: "<<std::endl;
    }
}

int main(int argc, char *argv[])
{
    char* pDataPath = "E:\\calibration kinect\\140517_calib2\\";
//    char* pImagePath = "E:\\3DMRSD\\Dome_3D_data\\ideals_mask\\";
    char* pImagePath = "E:\\140722_ultimatum_mask\\";
    Dome dm;
    Skeleton sk;

    dm.initDome();
    dm.removeCamera(14,18);
    dm.readCameraParameter(pDataPath);
//    dm.readImage(pImagePath, "png", 960);
    dm.readImage(pImagePath, "png", 2723);
    dm.m_fScaleFactor = 0.3182f;

    uchar* imageData = new uchar[640 * 480 * dm.getVGACameraNumber()];
    float* rMat = new float[9 * dm.getVGACameraNumber()];
    float* tVec = new float[3 * dm.getVGACameraNumber()];
    float* cameraMat = new float[9 * dm.getVGACameraNumber()];

    DomeCamera camera;
    int index = 0;
    for (int panIdx = 1; panIdx <=20; panIdx ++)
    {
        for (int camIdx =1; camIdx <=24; camIdx ++)
        {
            camera = dm(panIdx, camIdx);
            if (camera.isInitialized())
            {
                memcpy(imageData + index * 640 * 480, camera.image.data, 640 * 480);
                memcpy(rMat + index * 9, (float*)(camera.rMat.data), 9 * sizeof(float));
                memcpy(tVec + index * 3, (float*)(camera.tVec.data), 3 * sizeof(float));
                memcpy(cameraMat + index * 9, (float*)(camera.cameraMatrix.data), 9 * sizeof(float));
                index ++;
            }
        }
    }

    //Mat image(480, 640, CV_8UC1, imageData + (24 * (13 - 1) + 14 - 1) * 640 * 480);
    //imshow("", image);

    float centerX = -0.825672513015673;
    float centerY = -0.0678004113177330 + 0.3;
    float centerZ = -0.106122228354529;
    index = 0;
    float step = dm.m_fScaleFactor * 0.01;
    int size = 150;
    float* points = new float[3 * (size*2+1) * (size*2+1) * (size*2+1)];
    int* pcount = new int[(size*2+1) * (size*2+1) * (size*2+1)]();
    for (float x = centerX - step * size; x <= centerX + step * size; x += step)
    {
        for (float y = centerY - step * size; y <= centerY + step * size; y += step)
        {
            for (float z = centerZ - step * size; z <= centerZ + step * size; z += step)
            {
                memcpy(points + index * 3, &x, sizeof(float));
                memcpy(points + index * 3 + 1, &y, sizeof(float));
                memcpy(points + index * 3 + 2, &z, sizeof(float));
                index ++;
            }
        }
    }


    cl::Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    context = cl::Context(devices);
    queue = cl::CommandQueue(context, devices[0]);
    createKernel("../../DomeProject/checkVoxel.cl");

    cl::Buffer image_data = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * 640 * 480 * dm.getVGACameraNumber(), imageData);
    cl::Buffer Points = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 3 * index, points);
    cl::Buffer RotationMatrix = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 9 * dm.getVGACameraNumber(), rMat);
    cl::Buffer tVector = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 3 * dm.getVGACameraNumber(), tVec);
    cl::Buffer cameraMatrix = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 9 * dm.getVGACameraNumber(), cameraMat);
    cl::Buffer projectionCount = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * index, pcount);

    kernels[0].setArg(0, image_data);
    kernels[0].setArg(1, Points);
    kernels[0].setArg(2, RotationMatrix);
    kernels[0].setArg(3, tVector);
    kernels[0].setArg(4, cameraMatrix);
    kernels[0].setArg(5, projectionCount);

    cl::NDRange offset(cl::NullRange);
    cl::NDRange globalSize(dm.getVGACameraNumber(), index);
    cl::NDRange localSize(1, 1);
    queue.enqueueNDRangeKernel(kernels[0], offset, globalSize, localSize);

    int* result = new int [index]();
    queue.enqueueReadBuffer(projectionCount, CL_TRUE, 0, index*sizeof(int), result);

    fstream file("E:/voxel.xyz", ios_base::out);
    for (int i=0; i<index; i++)
    {
        if(result[i]>300)
            file<<points[i*3]<<" "<<points[i*3+1]<<" "<<points[i*3+2]<<endl;
    }
    file.close();

    delete []imageData;
    delete []rMat;
    delete []tVec;
    delete []points;
    delete []cameraMat;
    delete []result;

//    clReleaseKernel(kernels);
//    clReleaseCommandQueue(queue);
//    clReleaseMemObject(image_data);
//    clReleaseMemObject(Points);
//    clReleaseMemObject(RotationMatrix);
//    clReleaseMemObject(tVector);
//    clReleaseMemObject(cameraMatrix);
//    clReleaseMemObject(projectionCount);
    //cout<<dm(1,1).rMat<<endl;
    /*
    Mat KinectCameraMatrix = (Mat_<float>(3,3)<<1028.29577637000,0,960,0,1028.29577637000,540,0,0,1);
    Mat KinectRvec = (Mat_<float>(3,1)<<-0.0171982491690450,0.139786483818306,0.0359426604140810);
    Mat KinectTvec = (Mat_<float>(3,1)<<0.774936133361373, -0.156834099332820, 0.812955618145678);

    cout<<"skeleton number is "<<sk.ReadSkeletonFromTxt("E:\\00000300\\523_body.txt")<<endl;

    Mat empty;
    Mat jointInKinect = sk.jointToKinectColorCameraSpace(KinectCameraMatrix, empty);
    Mat jointInWorld = sk.jointToWorldSpace(jointInKinect,KinectRvec, KinectTvec, dm.m_fScaleFactor);
    //vector<Point2f> imagePoints = sk.projectToCamera(jointInWorld,dm(1,1));
    //Mat img =  imread("E:/00000300/00000300_01_01.png");
    //sk.drawSkeletonOnImage(imagePoints, img);
    //imshow("", img);
    //waitKey(0);

    Mat R, x;
    Mat X = jointInWorld.col(0);
    //cout<<X.type()<<endl;
    DomeCamera camera;
    int64 start, end;
    Point2f xd;
    start = getTickCount();
    for (int i=0; i<1; i++)
    {
        cout<<i<<endl;
    for (int panIdx = 1; panIdx <= 1; panIdx ++)
    {
        for (int camIdx = 1; camIdx <= 1; camIdx ++)
        {
            camera = dm(panIdx, camIdx);
            if (camera.isInitialized())
            {
                //Rodrigues(camera.rVec, R);
                x = camera.rMat * X + camera.tVec;
                cout<<x<<endl;
                xd = sk.distortPoint(Point2f(x.at<float>(0,0)/x.at<float>(2,0), x.at<float>(1,0)/x.at<float>(2,0)),camera.distCoeffs.at<float>(0,0));
                //cout<<panIdx<<" "<<camIdx<<endl;
            }
        }
    }
    }
    end = getTickCount();
    cout<<"CPU exectution time = "<<((double)end-(double)start)/getTickFrequency()<<endl;
    start = getTickCount();
    end = getTickCount();
    cout<<"GPU exectution time = "<<((double)end-(double)start)/getTickFrequency()<<endl;
    //drawSkeletonOnPanels(jointInWorld, sk, dm, "E:/00000300/", "E:/00000300/", 300);
    */
    return 0;
}
