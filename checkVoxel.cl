#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

__kernel
void checkVoxel(__global const unsigned char* image_data,
                __global const float* Points,
                __global const float* RotationMatrix,
                __global const float* tVector,
                __global const float* cameraMatrix,
                __global int* projectionCount
                                )
{
    //Work-item gets its index within index space
    const int camIdx = get_global_id(0);
    const int pointIdx = get_global_id(1);

    //get point(x, y, z)
    float X = Points[3 * pointIdx];
    float Y = Points[3 * pointIdx + 1];
    float Z = Points[3 * pointIdx + 2];

    //point to camera space
    float Xc = RotationMatrix[9 * camIdx] * X + RotationMatrix[9 * camIdx + 1] * Y + RotationMatrix[9 * camIdx + 2] * Z + tVector[3 * camIdx];
    float Yc = RotationMatrix[9 * camIdx + 3] * X + RotationMatrix[9 * camIdx + 4] * Y + RotationMatrix[9 * camIdx + 5] * Z + tVector[3 * camIdx + 1];
    float Zc = RotationMatrix[9 * camIdx + 6] * X + RotationMatrix[9 * camIdx + 7] * Y + RotationMatrix[9 * camIdx + 8] * Z + tVector[3 * camIdx + 2];

    //project to image
    float z = cameraMatrix[9 * camIdx + 6] * Xc + cameraMatrix[9 * camIdx + 7] * Yc + cameraMatrix[9 * camIdx + 8] * Zc;
    int x = (int) ( (cameraMatrix[9 * camIdx] * Xc + cameraMatrix[9 * camIdx + 1] * Yc + cameraMatrix[9 * camIdx + 2] * Zc) / z + 0.5f);
    int y = (int) ( (cameraMatrix[9 * camIdx + 3] * Xc + cameraMatrix[9 * camIdx + 4] * Yc + cameraMatrix[9 * camIdx + 5] * Zc) / z + 0.5f);

    if ( camIdx < 479 && pointIdx < 201*201*200)
    {
            if( x >= 0 && x <640 && y >= 0 && y < 480)
            {
                    if (image_data[640 * 480 * camIdx + 640 * y + x] > 0)
                    {
                            atomic_add(&(projectionCount[pointIdx]), 1);
                    }
            }
    }
}
