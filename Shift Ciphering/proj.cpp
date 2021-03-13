#define CL_USE_DEPRECATED_OPENCL_2_0_APIS	// using OpenCL 1.2, some functions deprecated in OpenCL 2.0
#define __CL_ENABLE_EXCEPTIONS				// enable OpenCL exemptions

// C++ standard library and STL headers
#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
#include "common.h"

#define GLOBAL_SIZE 512
#define LOCAL_SIZE 1
#define NUM_WORK_GROUPS GLOBAL_SIZE/LOCAL_SIZE

void upper(std::string * str);

int main(void) 
{
	cl::Platform platform;			// device's platform
	cl::Device device;				// device used
	cl::Context context;			// context for the device
	cl::Program program;			// OpenCL program object
	cl::Kernel kernel;				// a single kernel object
	cl::CommandQueue queue;			// commandqueue for a context and device

	cl::Buffer inputBuffer1, inputBuffer2, inputBuffer3, outputBuffer1;

	std::string filename = "plaintext.txt";

	std::vector <int> input;
	std::vector<int> output;

	cl_int n;

	try {
		// select an OpenCL device
		if (!select_one_device(&platform, &device))
		{
			// if no device selected
			quit_program("Device not selected.");
		}

		// open file
		std::ifstream inputFile(filename);

		// check whether file was opened
		if (!inputFile.is_open())
		{
			std::cout << "File not found." << std::endl;
			return 0;
		}

		// load file contents into string
		std::string inputString(std::istreambuf_iterator<char>(inputFile), (std::istreambuf_iterator<char>()));

		// convert string to uppercase
		upper(&inputString);

		// load string into vector
		input.resize(inputString.length());
		std::copy(inputString.begin(), inputString.end(), input.begin());

		output.resize(input.size());

		cl_int chunksPerWorkUnit = input.size() / (16 * GLOBAL_SIZE) + 1;
		// get n value from user
		std::cout << "Please enter an integer: ";

		std::cin >> n;

		// create a context from device
		context = cl::Context(device);

		// build the program
		if(!build_program(&program, &context, "kernel.cl")) 
		{
			// if OpenCL program build error
			quit_program("OpenCL program build error.");
		}

		// create a kernel
		kernel = cl::Kernel(program, "task");

		// create command queue
		queue = cl::CommandQueue(context, device);

		// create buffers
		inputBuffer1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int) * input.size(), &input[0]);
		inputBuffer2 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &n);
		inputBuffer3 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &chunksPerWorkUnit);
		outputBuffer1 = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * output.size());

		kernel.setArg(0, inputBuffer1);
		kernel.setArg(1, inputBuffer2);
		kernel.setArg(2, inputBuffer3);
		kernel.setArg(3, outputBuffer1);

		// enqueue kernel for execution
		cl::NDRange offset(0);
		cl::NDRange globalSize(GLOBAL_SIZE);
		cl::NDRange localSize(LOCAL_SIZE);

        queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);

		// enqueue command to read from device to host memory
		queue.enqueueReadBuffer(outputBuffer1, CL_TRUE, 0, sizeof(cl_int) * output.size(), &output[0]);

		// output results
		std::ofstream outputFile;
		outputFile.open("ciphertext.txt");

		for (int &c : output)
		{
			outputFile << (cl_char)c;
		}
		
		//outputFile << programString;
		outputFile.close();

		// decrypt
		input = output;
		n = -n;

		// create buffers
		inputBuffer1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int) * input.size(), &input[0]);
		inputBuffer2 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &n);
		inputBuffer3 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &chunksPerWorkUnit);
		outputBuffer1 = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * output.size());

		kernel.setArg(0, inputBuffer1);
		kernel.setArg(1, inputBuffer2);
		kernel.setArg(2, inputBuffer3);
		kernel.setArg(3, outputBuffer1);

		queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);

		// enqueue command to read from device to host memory
		queue.enqueueReadBuffer(outputBuffer1, CL_TRUE, 0, sizeof(cl_int) * output.size(), &output[0]);

		outputFile.open("decrypted.txt");

		for (int &c : output)
		{
			outputFile << (cl_char)c;
		}

		//outputFile << programString;
		outputFile.close();
	}
	// catch any OpenCL function errors
	catch (cl::Error e) {
		// call function to handle errors
		handle_error(e);
	}

#ifdef _WIN32
	// wait for a keypress on Windows OS before exiting
	std::cout << "\npress a key to quit...";
	std::cin.ignore();
#endif
	
	return 0;
}

void upper(std::string * str)
{
	for (char & c : *str)
	{
		// if the character is a lowercase character (ie not uppercase or special character)
		if (c >= 97 && c <= 122)
		{
			c = c - 32;
		}
	}
}

