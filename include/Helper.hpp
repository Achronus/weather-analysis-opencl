#pragma once
#include <iomanip>
#include <algorithm>
#include <string>
#include <limits>

#include "Utils.h"

using namespace std;

class Helper
{
private:
	bool startProgram = false;

	// Used to read in the users input
	void readInput(string& str)
	{
		cout << "> ";
		cin >> str;
	};

	// Handles the logic for changing device context (platform and device)
	void changeContext()
	{
		while (true)
		{
			readInput(consoleInput);
			std::transform(consoleInput.begin(), consoleInput.end(), consoleInput.begin(), ::toupper);

			if (consoleInput == "Y" || consoleInput == "YES")
			{
				vector<string> messages = { "Input new platform number: ", "Input new device number: " };
				vector<int> items(messages.size()); // Store platform and device numbers
				string strItem;

				// Iterate over messages
				for (int i = 0; i < messages.size(); i++)
				{
					cerr << messages[i] << endl;

					while (true)
					{
						readInput(consoleInput);

						try {
							if (consoleInput.length() == 1 && stoi(consoleInput) >= 0) {
								items[i] = stoi(consoleInput);
								// Display selected items
								switch (i)
								{
								case(0):
									strItem = GetPlatformName(items[0]);
									cerr << "Selected platform: " << strItem << endl;
									break;
								case(1):
									strItem = GetDeviceName(items[0], items[1]);
									cerr << "Selected device: " << strItem << endl;
									break;
								}
								// Break loop if correct input provided
								if (strItem.find("doesn't exist") == string::npos)
									break;
							}
							else {
								cerr << "Invalid entry! Input is too long. Must be a single digit." << endl;
							}
						}
						catch (std::exception& err) {
							cerr << "Input a valid ID number." << endl;
						}
					}
				}
				// Update ids
				platform_id = items[0];
				device_id = items[1];

				// Return to help screen
				system("CLS");
				printHelp();
				break;
			}
			else if (consoleInput == "N" || consoleInput == "NO") {
				system("CLS");
				printHelp();
				break;
			}
			else
				cerr << "Invalid command. Input 'Y' or 'N'." << endl;
		}
	};

public:
	string consoleInput;
	int platform_id = 0;
	int device_id = 0;

	// Displays the help menu
	void printHelp() {
		displayCurrentContext("Running on:");
		cout << "-----------------------------------------------------------------" << endl;
		cout << "Choose a command:" << endl;
		cout << "-----------------------------------------------------------------" << endl;
		cout << "  1 : list and select platforms and devices" << endl;
		cout << "  2 : calculate statistics (uses selected platform and device)" << endl;
		cout << "  3 : exit program" << endl;
		cout << "-----------------------------------------------------------------" << endl;
	};

	// Displays the platform and device that is active
	void displayCurrentContext(string str)
	{
		cout << str << endl;
		cout << "  Platform " << platform_id << ": " << GetPlatformName(platform_id) << endl;
		cout << "  Device   " << device_id << ": " << GetDeviceName(platform_id, device_id) << endl;
		cout << endl;
	};

	// Displays the file options menu
	void displayFileOptions()
	{
		displayCurrentContext("Running on:");
		// Ask user to select file
		cout << "Select file:" << endl;
		cout << "  1 : Small Lincolnshire Dataset" << endl;
		cout << "  2 : Large Lincolnshire Dataset" << endl;
	};

	// Displays the sort menu
	void displaySortMenu()
	{
		displayCurrentContext("Running on:");
		// Ask user to select what statistics to calculate
		cout << "Select statistic calculation type:" << endl;
		cout << "  1 : Min, max, mean, standard deviation (no sorting)" << endl;
		cout << "  2 : All statistics" << endl;
	}

	// Handles the main menu functionality
	void handleInput()
	{
		printHelp();
		while (startProgram == false) {
			readInput(consoleInput);

			// Check for valid input
			try {
				if (stoi(consoleInput)) {
					int command = stoi(consoleInput);

					// Handle input
					switch (command) {
					case(1):
						system("CLS");
						cout << ListPlatformsDevices() << endl;
						displayCurrentContext("Currently selected:");
						cout << "Select new platform and device? (Y/N)" << endl;
						changeContext();
						break;
					case(2):
						startProgram = true; // begin program
						system("CLS");
						break;
					case(3):
						exit(0);
						break;
					default:
						cerr << "Command number doesn't exist! Input a number between '1' and '3'." << endl;
						break;
					}
				}
			}
			catch (std::exception& err) {
				cerr << "Invalid entry. Input a number between '1' and '3'." << endl;
			}
		}
	};

	// Assigns the correct file based on the users input
	string selectFile(string file_url) 
	{
		while (true) {
			readInput(consoleInput);
			if (stoi(consoleInput) == 1) {
				file_url = "datasets/temp_lincolnshire_short.txt";
				break;
			}
			else if (stoi(consoleInput) == 2) {
				file_url = "datasets/temp_lincolnshire.txt";
				break;
			}
			else
				cerr << "Invalid file number selected. Choose '1' or '2'." << endl;			
		}
		system("CLS");
		return file_url;
	}

	// Run the program with sorting or without sorting
	bool enableSorting() 
	{
		displaySortMenu();
		while (true) {
			readInput(consoleInput);
			if (stoi(consoleInput) == 1)
				return false;
			else if (stoi(consoleInput) == 2)
				return true;
			else
				cerr << "Invalid file number selected. Choose '1' or '2'." << endl;
		}
	}

	// Create table divider dynamically, used for top and bottom of table
	void tableFormatting(int strLen)
	{
		// Table header format
		cout << "|";
		for (int i = 0; i < strLen - 1; i++)
			cout << "-";
		cout << "|" << endl;
	}

	// Outputs the statistics and kernel information at the end of the program
	void outputInfo(vector<float>& statistics, vector<string>& kernel_names, vector<cl::Event>& kernel_events, bool sortFlag)
	{
		vector<string> stat_names = { "Min Value", "Max Value", "Mean ", "Standard Deviation" };
		float total_seconds = 0.;

		// Add additional names if sorting enabled
		if (sortFlag) {
			stat_names.push_back("Median");
			stat_names.push_back("1st Quartile");
			stat_names.push_back("3rd Quartile");
		}

		// Calculate length of string
		int strLen = 0;
		for (int i = 0; i < stat_names.size(); i++)
		{
			strLen += stat_names[i].length();
			strLen += 3;
		}

		// Table header format
		cout << "\nStatistic Results:" << endl;
		tableFormatting(strLen);

		// Statistic names
		for (int i = 0; i < stat_names.size(); i++)
		{
			cout << "| " << stat_names[i] << " ";
		}
		cout << "|" << endl;

		// Statistic results
		for (int i = 0; i < stat_names.size(); i++)
		{
			int length = stat_names[i].length();
			cout << std::left << "| " << setfill(' ') << setw(static_cast<streamsize>(length) + 1) << fixed << setprecision(3) << statistics[i];
		}
		cout << "|" << endl;

		// Table footer format
		tableFormatting(strLen);

		// Kernel run times
		cout << "\nKernel execution times:" << endl;
		for (int i = 0; i < kernel_names.size(); i++)
		{
			float nanoseconds = kernel_events[i].getProfilingInfo<CL_PROFILING_COMMAND_END>() - kernel_events[i].getProfilingInfo<CL_PROFILING_COMMAND_START>();
			float seconds = nanoseconds / 1e+9;
			cout << "  " << i + 1 << ". " << kernel_names[i] << ": " << fixed << setprecision(9) << seconds << " [secs]" << endl;
			cout << "     " << GetFullProfilingInfo(kernel_events[i], ProfilingResolution::PROF_US) << endl;
			cout << endl;
			total_seconds += seconds;
		}
		cout << "Total run time: " << fixed << setprecision(9) << total_seconds << " [secs]" << endl;
		cout << endl;
	};
};