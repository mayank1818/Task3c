#include <iostream>
#include <vector>
#include <algorithm>
#include <CL/cl.hpp>

using namespace std;

// Define the OpenCL kernel source code for sorting
const char* kernelSource =
"void merge(global int* data, int start, int mid, int end) {\n"
"int* temp = new int[end - start + 1];\n"
"int i = start, j = mid+1, k = 0;\n"
"while (i <= mid && j <= end) {\n"
"if (data[i] <= data[j]) {\n"
"temp[k] = data[i];\n"
"i++;\n"
"} else {\n"
"temp[k] = data[j];\n"
"j++;\n"
"}\n"
"k++;\n"
"}\n"
"while (i <= mid) {\n"
"temp[k] = data[i];\n"
"i++;\n"
"k++;\n"
"}\n"
"while (j <= end) {\n"
"temp[k] = data[j];\n"
"j++;\n"
"k++;\n"
"}\n"
"for (int i = 0; i < k; i++) {\n"
"data[start+i] = temp[i];\n"
"}\n"
"delete[] temp;\n"
"}\n"
"_kernel void quicksort(_global int* data, int left, int right) {\n"
"if (left >= right) return;\n"
"int pivot = data[(left + right) / 2];\n"
"int i = left;\n"
"int j = right;\n"
"while (i <= j) {\n"
"while (data[i] < pivot) i++;\n"
"while (data[j] > pivot) j--;\n"
"if (i <= j) {\n"
"int temp = data[i];\n"
"data[i] = data[j];\n"
"data[j] = temp;\n"
"i++;\n"
"j--;\n"
"}\n"
"}\n"
"if (left < j) quicksort(data, left, j);\n"
"if (i < right) quicksort(data, i, right);\n"
"}\n";

int main() {
    // Define the input array
    vector<int> inputArray = {5, 2, 9, 1, 5, 6, 3, 8, 7, 4};
    int arraySize = inputArray.size();

    // Set up OpenCL environment
    cl::Platform openCLPlatform;
    cl::Device openCLDevice;
    cl::Context openCLContext;
    cl::Program::Sources programSources;
    cl::Program openCLProgram;
    cl::Kernel openCLKernel;
    cl::CommandQueue openCLQueue;

    // Initialize OpenCL platform, device, and context
    cl::Platform::get(&openCLPlatform);
    platform.getDevices(CL_DEVICE_TYPE_GPU, &openCLDevice);
    openCLContext = cl::Context(openCLDevice);
    programSources.push_back({kernelSource, strlen(kernelSource)});
    openCLProgram = cl::Program(openCLContext, programSources);

    // Build the OpenCL program
    try {
        openCLProgram.build({openCLDevice});
    } catch (cl::Error& error) {
        cout << "Build status: " << openCLProgram.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(openCLDevice) << endl;
        cout << "Build log:\t " << openCLProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(openCLDevice) << endl;
        throw error;
    }

    // Create OpenCL kernel
    openCLKernel = cl::Kernel(openCLProgram, "quicksort");

    // Create OpenCL buffer for input data
    cl::Buffer inputBuffer(openCLContext, CL_MEM_READ_WRITE, sizeof(int) * arraySize);

    // Copy input data from host to device
    openCLQueue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, sizeof(int) * arraySize, inputArray.data());

    // Set kernel arguments
    openCLKernel.setArg(0, inputBuffer);
    openCLKernel.setArg(1, 0);
    openCLKernel.setArg(2, arraySize - 1);

    // Execute the kernel
    openCLQueue.enqueueNDRangeKernel(openCLKernel, cl::NullRange, cl::NDRange(arraySize), cl::NullRange);
    openCLQueue.finish();

    // Read the sorted data back to host
    openCLQueue.enqueueReadBuffer(inputBuffer, CL_TRUE, 0, sizeof(int) * arraySize, inputArray.data());

    // Output sorted array
    cout << "Sorted array: ";
    for (int i = 0; i < arraySize; i++) {
        cout << inputArray[i] << " ";
    }
    cout << endl;

    return 0;
}
