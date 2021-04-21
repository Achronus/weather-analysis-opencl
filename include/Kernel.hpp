#pragma once
#include "Helper.hpp"

typedef int mytype;

class Kernel
{
protected:
	cl::Context context;
	cl::CommandQueue queue;
	cl::Program program;

public:
	// Sets a kernel instance with a stored context, queue and program
	Kernel(cl::Context _context, cl::CommandQueue _queue, cl::Program _program)
	{
		context = _context;
		queue = _queue;
		program = _program;
	}

	// Creates a buffer and fills it with zeros
	cl::Buffer createBuffer(size_t& vectorSize)
	{
		cl::Buffer buffer(context, CL_MEM_READ_WRITE, vectorSize);
		queue.enqueueFillBuffer(buffer, CL_TRUE, 0, vectorSize);
		return buffer;
	}

	// Creates a kernel and sets its two arguments, both as buffers
	cl::Kernel setupKernel(string kernelName, cl::Buffer input, cl::Buffer output)
	{
		// Create the kernel
		cl::Kernel kernel = cl::Kernel(program, kernelName.c_str());

		// Set kernel arguments
		kernel.setArg(0, input);
		kernel.setArg(1, output);
		return kernel;
	}

	// Creates a kernel and sets its three arguments, 2 as buffers and 1 as local storage
	cl::Kernel setupKernel(string kernelName, cl::Buffer input, cl::Buffer output, size_t localSize)
	{
		// Create the kernel
		cl::Kernel kernel = cl::Kernel(program, kernelName.c_str());

		// Set kernel arguments
		kernel.setArg(0, input);
		kernel.setArg(1, output);
		kernel.setArg(2, cl::Local(localSize));
		return kernel;
	}

	// Creates a kernel and sets its five arguments, 2 as buffers, 1 as local storage, and 2 given values
	cl::Kernel setupKernel(string kernelName, cl::Buffer input, cl::Buffer output, size_t localSize, int value1, int value2)
	{
		// Create the kernel
		cl::Kernel kernel = cl::Kernel(program, kernelName.c_str());

		// Set kernel arguments
		kernel.setArg(0, input);
		kernel.setArg(1, output);
		kernel.setArg(2, cl::Local(localSize));
		kernel.setArg(3, value1);
		kernel.setArg(4, value2);
		return kernel;
	}

	// Executes the give kernel with the specified parameters
	void executeKernel(string kernelName, cl::Kernel activeKernel, size_t dataSize, size_t localSize, cl::Event& kernelEvent)
	{
		cout << "  " << kernelName << "...";
		if (localSize != NULL)
			queue.enqueueNDRangeKernel(activeKernel, cl::NullRange, cl::NDRange(dataSize), cl::NDRange(localSize), NULL, &kernelEvent);
		else
			queue.enqueueNDRangeKernel(activeKernel, cl::NullRange, cl::NDRange(dataSize), cl::NullRange, NULL, &kernelEvent);
		cout << " Complete." << endl;
	}

	// Reads a kernel buffer and returns the vector output
	vector<mytype> readKernelBuffer(cl::Buffer readBuffer, size_t size, vector<mytype> readVector)
	{
		// Copy the result from device to host
		queue.enqueueReadBuffer(readBuffer, CL_TRUE, 0, size, &readVector[0]);
		queue.finish(); // Wait to finish
		return readVector;
	}
};