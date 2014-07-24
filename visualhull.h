#ifndef VISUALHULL_H
#define VISUALHULL_H

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>

using namespace std;

class VisualHull
{
public:
    VisualHull();
    void setParameters(float centerX, float centerY, float centerZ, float voxelSize);
    void createKernel(string kernelFileName);
    void generatePoints();
    void doVisualHullReconstruction();
    
public:

private:
    vector<cl::Platform> m_platforms;
    vector<cl::Device> m_devices;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program::Sources m_source;
    cl::Program m_program;
    vector<cl::Kernel> m_kernels;
    string m_parameterName;
    cl_long m_parameterValue;


};

#endif // VISUALHULL_H
