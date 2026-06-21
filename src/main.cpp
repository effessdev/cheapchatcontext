// CheapChatContext.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "ProjectScanner.h"

using namespace std;

int main()
{
	auto [files, tree] = ScanProject(std::filesystem::current_path());

	for (const auto& file : files) {
		cout << "file: " << file << endl;
	};

	cout << "tree:\n" << tree << endl;


}
