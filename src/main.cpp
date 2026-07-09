// CheapChatContext.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "ProjectScanner.h"
#include "FileFilter.h"

using namespace std;

int main()
{
	auto files = ScanProject(std::filesystem::current_path());

	cout << "Files:" << endl
		 << endl;
	for (const auto &file : files)
	{
		cout << "file: " << file << endl;
	};

	auto filteredFiles = filterFiles(files, {"**/*.cpp"}, {"tests/"});

	cout << "Filtered Files:" << endl
		 << endl;
	for (const auto &file : filteredFiles)
	{
		cout << "file: " << file << endl;
	};

	return 0;
}
