// CheapChatContext.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "ProjectScanner.h"
#include "FileFilter.h"

using namespace std;

const std::vector<std::string> FLAGS = {"-i", "-e"};

int main(int argc, char *argv[])
{
	// Initialize variables for args
	std::vector<std::string> includeArgs = {};
	std::vector<std::string> excludeArgs = {};

	// Parse args
	std::string currentFlag = "";
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (std::ranges::find(FLAGS, arg) != FLAGS.end())
		{
			currentFlag = arg;
			continue;
		}

		if (currentFlag == "-i")
		{
			includeArgs.push_back(arg);
		}
		else if (currentFlag == "-e")
		{
			excludeArgs.push_back(arg);
		}
	}

	// Print args (for development only)
	for (const auto &arg : includeArgs)
	{
		cout << "include arg: " << arg << endl;
	}
	for (const auto &arg : excludeArgs)
	{
		cout << "exclude arg: " << arg << endl;
	}

	// Get the relative paths to the files in the project directory (handling .gitignore flies)
	auto files = ScanProject(std::filesystem::current_path());

	cout << "Files:" << endl
		 << endl;
	for (const auto &file : files)
	{
		cout << "file: " << file << endl;
	};

	// 2nd arg: include patterns, 3rd arg: exclude patterns
	auto filteredFiles = filterFiles(files, {"**/*.cpp"}, {"tests/"});

	cout << "Filtered Files:" << endl
		 << endl;
	for (const auto &file : filteredFiles)
	{
		cout << "file: " << file << endl;
	};

	return 0;
}
