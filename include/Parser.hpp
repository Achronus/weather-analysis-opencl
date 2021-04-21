# pragma once
# include "Helper.hpp"

class Parser
{
	Helper* helper;
	ifstream file;
	string line;

public:
	// Reads the data from a given text file url and returns its last column as a vector of integers (values are multipled by 100)
	vector<int> readFile(string& file_url)
	{
		cout << "\nReading in data from file... " << endl;
		cout << "  Note: this may take a few moments... ";
		vector<int> data;

		file.open(file_url);
		if (file.is_open())
		{
			// Get data from file
			while (getline(file, line))
			{
				// Get last value in line and multiply by 100
				int lastVal = stof(line.substr(line.rfind(" "))) * 100;
				data.push_back(lastVal); // add to vector
			}
			file.close();
			cout << "Complete." << endl;
			cout << "  Total records in file: " << data.size() << endl;
			return data;
		}
		else {
			cerr << "\nUnable to open file! Check that the file exists and is inside the 'datasets' folder.\n";
			exit(0);
		}
	}

	// Adds a given padded value to a given data vector, if local size isn't a factor of the data size
	vector<int> padData(vector<int> data, size_t localSize, size_t paddingSize, int value = 0)
	{
		if (paddingSize)
		{
			vector<int> extendVec(localSize - paddingSize, value);
			data.insert(data.end(), extendVec.begin(), extendVec.end());
		}
		return data;
	}

	// Removes padded values from the given vector starting at a given location
	vector<int> removePad(vector<int> data, size_t startPoint)
	{
		data.erase(data.begin() + startPoint, data.end());
		return data;
	}

	// Removes a given padded value from a given vector
	vector<int> removePad(vector<int> data, int value)
	{
		data.erase(remove(data.begin(), data.end(), value), data.end());
		return data;
	}
};