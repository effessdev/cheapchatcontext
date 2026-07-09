// CheapChatContext.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "ProjectScanner.h"

using namespace std;

int main()
{
	auto files = ScanProject(std::filesystem::current_path());

	cout << "Files:" << endl;
	for (const auto &file : files)
	{
		cout << "file: " << file << endl;
	};
}
