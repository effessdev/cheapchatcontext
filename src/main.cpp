// CheapChatContext.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "ProjectScanner.h"
#include "FileFilter.h"
#include "FileReader.h"
#include "FileWriter.h"

using namespace std;

const std::vector<std::string> FLAGS = {"-i", "-e"};

int main(int argc, char *argv[])
{
	// Initialize variables for args
	std::vector<std::string> includeArgs = {};
	std::vector<std::string> excludeArgs = {};

	// Parse `config` command
	if (argc > 1 && std::string(argv[1]) == "config")
	{
		createConfig(std::filesystem::current_path());
		cout << "Created config file at "
			 << std::filesystem::current_path() / "ccc-config.toml" << endl
			 << "Edit the file according to your needs." << endl;
		return 0;
	};

	// Parse -i (include) and -e (exclude) args
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

	// Get files
	// `files` are the relative paths to the files in the project directory (handling .gitignore flies)
	auto files = ScanProject(std::filesystem::current_path());

	// Filter files
	// arg2: include glob patterns, arg3: exclude glob patterns
	auto filteredFiles = filterFiles(files, includeArgs, excludeArgs);

	// Build the output
	std::string output = "# Context\n\n## Project structure:\n\n";
	for (const auto &file : files)
	{
		output += file.string() + "\n";
	}
	output += "\n";
	for (const auto &file : filteredFiles)
	{
		output += "## File: " + file.string() + "\n\n";
		output += readFileToStringCodeBlock(file) + "\n\n";
	}

	output += "# Prompt\n\n";

	// Print the output (later convert to clipboard paste)
	cout << output;

	return 0;
}
