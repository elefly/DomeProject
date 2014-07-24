#include "visualhull.h"

VisualHull::VisualHull()
{
    
}

void VisualHull::setParameters(float centerX, float centerY, float centerZ, float voxelSize)
{

}

void VisualHull::createKernel(string kernelFileName)
{
    try
    {
        ifstream programFile(kernelFileName.c_str());
        string programString(istreambuf_iterator<char>(programFile),
                                  (istreambuf_iterator<char>()));
        m_source = cl::Program::Sources(1, std::make_pair(programString.c_str(),
                                                          programString.length()+1));

        m_program = cl::Program(m_context, m_source);
        m_program.build(m_devices);
        m_program.createKernels(&m_kernels);

        if(1)
        {
            for(int _mI=0; _mI<m_kernels.size(); _mI++)
            {
                m_parameterName = m_kernels[_mI].getInfo<CL_KERNEL_FUNCTION_NAME>();
                cout << "Kernel: " << m_parameterName << std::endl;
            }
        }
    }
    catch(cl::Error err)
    {
        cout<<m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_devices[0]);
        cout<<err.what()<<"Error code: "<<endl;
    }
}
