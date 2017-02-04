#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

void usage(char * prog) {
	cout << "USAGE: " << prog << " in-file out-file" << endl;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}
	
	stringstream cmd;
	cmd << "rm -f " << argv[2];
	system(cmd.str().c_str());
	
	ifstream inFile(argv[1]);
	ofstream outFile(argv[2]);
	
	string token;
	
	vector<string> deps;
	
	int n = 0;
	
	while (!inFile.eof() && !inFile.fail()) {
		inFile >> token;
		
		if (!inFile.fail()) {
			if (token != "\\") {
				//cout << token << endl;
				
				if (n != 1) {
					outFile << token << " ";
				}
				
				if (n > 1) {
					deps.push_back(token);
				}
				
				++n;
			}
		}
	}
	
	outFile << endl;
	
	for (unsigned int i = 0; i < deps.size(); ++i) {
		outFile << deps[i] << ": " << endl;
	}
	
	outFile.close();
	inFile.close();	
	
	return 0;
}
