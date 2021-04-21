#include <locale>
#include <cmath>
#include "Parser.hpp"
#include "Kernel.hpp"

/*
The application performs like a console app, where commands are input based on pre-set options. Both the small and large 'temp_lincolnshire' datasets are used within the application. The application allows switching between computing devices (platform and device), if required, before calculating the temperature data's statistics. The data is loaded traditionally using a standard C++ approach before being passed through multiple reduce kernels to calculate the statistics. Additionally, Selection Sort is used to sort the data into ascending order, providing the ability to calculate more advanced statistics, such as median, 1st quartile, and 3rd quartile. This sorting algorithm is based on an implementation written by Bainville (2011).

The kernels used within the implementation are inspired by the 'reduce_add_3' kernel presented in Tutorial 3 (Millard, 2020). The approach used loads the temperature data (floating-point numbers) from the selected file into an integer vector. These values are multiplied by 100 to ensure that the floating-point values are retained when passed into the integer vector. This method provides the ability to use atomic operations, which only accept integer values as input. Additionally, barrier functions are used throughout each kernel. Both barrier functions and atomic operations assist with work-item synchronisation, preventing data conflicts and ensuring that every work item has reached the same point in its processing, which is crucial for calculating the statistics correctly. Once the kernels calculations have completed, the output values are divided by 100 to convert the values to the correct format.

References:
	- Bainville, E. (2011) OpenCL Sorting. Parallel Selection Sort. Bealto. Available from: http://www.bealto.com/gpu-sorting_parallel-selection.html [accessed 13 April 2021].
	- Millard, A. (2020) OpenCL Tutorials. GitHub. Available from: https://github.com/alanmillard/OpenCL-Tutorials [accessed 28 March 2021].
	- Scarpino, M. (2012) OpenCL in Action. New York: Manning. Available from: https://www.manning.com/books/opencl-in-action [accessed 28 March 2021].
*/

int main(int argc, char** argv) {
	//Part 1 - handle command line options such as device selection, verbosity, etc.
	// Instantiate classes
	Helper helper;
	Parser parser;

	helper.handleInput();

	//detect any potential exceptions
	try {
		//Part 2 - host operations
		//2.1 Select computing devices
		cl::Context context = GetContext(helper.platform_id, helper.device_id);

		//display the selected device
		helper.displayCurrentContext("Running on:");

		//create a queue to which we will push commands for the device
		cl::CommandQueue queue(context, CL_QUEUE_PROFILING_ENABLE);

		//2.2 Load & build the device code
		cl::Program::Sources sources;

		AddSources(sources, "kernels/my_kernels.cl");

		cl::Program program(context, sources);

		//build and debug the kernel code
		try {
			program.build();
		}
		catch (const cl::Error& err) {
			cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << endl;
			cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << endl;
			cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>()[0]) << endl;
			throw err;
		}

		// Part 3 - memory allocation
		// Instantiate kernel
		Kernel kernel(context, queue, program);

		// Display console file info
		helper.displayFileOptions();

		// Start data handling
		string file_url;
		file_url = helper.selectFile(file_url); // Select data file

		// Read in data
		vector<mytype> temperatures = parser.readFile(file_url);

		// Set local size variables
		size_t local_size = 256;
		size_t padding_size = temperatures.size() % local_size;
		size_t pad_difference = local_size - padding_size;
		size_t scratch_size = local_size * sizeof(mytype);

		// Pad the data
		int pad_value = 3;
		temperatures = parser.padData(temperatures, local_size, padding_size, pad_value);
		
		// Set size variables
		size_t data_size = temperatures.size(); //number of elements
		size_t vec_size = data_size * sizeof(mytype); // size in bytes
		size_t initial_data_size = data_size - pad_difference;
		size_t wg_size = data_size / local_size;

		size_t n_stats = 7; //number of statistics
		size_t stats_size = n_stats * sizeof(mytype); //size in bytes

		// Host - output
		vector<mytype> out_temps(data_size);
		vector<mytype> core_data(initial_data_size);
		vector<float> statistics(n_stats);

		// Set kernel related vectors
		vector<string> kernelNames = { "minReduce", "maxReduce", "sumReduce", "varianceReduce", "selectionSort" };
		vector<cl::Event> events;

		cout << "  Local size set to: " << local_size << endl;
		cout << "  Number of work-groups: " << wg_size << endl;
		cout << "  Padding increase: " << pad_difference << endl;
		cout << "  Number of records + padding: " << data_size << endl;
		
		cout << "\nCalculating statistics..." << endl;
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
		// Calculate first three stats - min, max, mean
		// Create input buffer and copy to device memory
		cl::Buffer buffer_input(context, CL_MEM_READ_ONLY, vec_size);
		queue.enqueueWriteBuffer(buffer_input, CL_TRUE, 0, vec_size, &temperatures[0]);

		// Iterate over first three kernels
		for (int i = 0; i < 3; ++i)
		{
			// Create output buffer and fill it with zeros
			cl::Buffer buffer_output = kernel.createBuffer(vec_size);

			//4.2 Setup the kernel
			cl::Event kernelEvent;

			cl::Kernel activeKernel = kernel.setupKernel(kernelNames[i], buffer_input, buffer_output, scratch_size);

			// Execute kernel
			kernel.executeKernel(kernelNames[i], activeKernel, data_size, local_size, kernelEvent);

			// Copy the data from device to host
			out_temps = kernel.readKernelBuffer(buffer_output, vec_size, out_temps);

			// Remove padded values
			core_data = parser.removePad(out_temps, initial_data_size);

			// Add kernel event to events vector
			events.push_back(kernelEvent);

			// Set statistic value
			if (i == 2) // mean
				statistics[i] = ((float)core_data[0] / initial_data_size) / 100.f;
			else 
				statistics[i] = core_data[0] / 100.f;
		}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
		// Calculate standard deviation
		// Set kernel variables
		int mean = statistics[2] * 100;
		cl::Event stdEvent;

		// Create output buffer and fill it with zeros
		cl::Buffer buffer_output = kernel.createBuffer(vec_size);

		// Setup kernel
		cl::Kernel calcStd = kernel.setupKernel(kernelNames[3], buffer_input, buffer_output, scratch_size, mean, initial_data_size);

		// Execute kernel
		kernel.executeKernel(kernelNames[3], calcStd, data_size, local_size, stdEvent);

		// Copy the result from device to host
		out_temps = kernel.readKernelBuffer(buffer_output, vec_size, out_temps);

		// Add kernel event to events list
		events.push_back(stdEvent);

		// Set standard deviation
		statistics[3] = sqrt((float)out_temps[0] / initial_data_size);
		//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
		// Calculate remaining statistics - median, Q1, Q3 (requires sorted vector)
		// Set kernel variables
		cl::Event sortEvent;

		// Create output buffer and fill it with zeros
		cl::Buffer buffer_sorted = kernel.createBuffer(vec_size);

		// Setup the kernel
		cl::Kernel sortData = kernel.setupKernel(kernelNames[4], buffer_input, buffer_sorted);

		// Execute kernel
		kernel.executeKernel(kernelNames[4], sortData, data_size, NULL, sortEvent);

		// Copy the result from device to host
		out_temps = kernel.readKernelBuffer(buffer_sorted, vec_size, out_temps);

		// Remove padded values from sorted data
		core_data = parser.removePad(out_temps, pad_value);

		// Add kernel event to events list
		events.push_back(sortEvent);
		
		// Calculate remaining statistics
		statistics[4] = core_data[round(initial_data_size * 0.5)] / 100.f; // median
		statistics[5] = core_data[round(initial_data_size * 0.25)] / 100.f; // Q1
		statistics[6] = core_data[round(initial_data_size * 0.75)] / 100.f; // Q3
		//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

		// Output information to console
		helper.outputInfo(statistics, kernelNames, events);
	}
	catch (cl::Error err) {
		cerr << "\nERROR: " << err.what() << ", " << getErrorString(err.err()) << endl;
	}
	
	return 0;
}