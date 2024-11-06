#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "invertedFile.h"
using namespace std;

void processQuery(const char* filename, string* queries, int numTokens) {
    ifstream file(filename);
    string token;
    int index = 0;

    if (file.is_open()) {
        while (file >> token && index < numTokens) {
            queries[index++] = token;
        }
        file.close();
    } else {
        cout << "Unable to open file: " << filename << endl;
    }
}

int countTokens(const char* filename) {
    ifstream file(filename);
    string token;
    int count = 0;

    if (file.is_open()) {
        while (file >> token) {
            count++;
        }
        file.close();
    } else {
        cout << "Unable to open file: " << filename << endl;
    }

    return count;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <tokenized_query_file>" << endl;
        return 1;
    }
	string dictFilename = "dict.txt";
	string postFilename = "post.txt";
	string mapFilename = "map.txt";
	
    int numTokens = countTokens(argv[1]);
    string* queries = new string[numTokens];
	Accumulator accumulator(1000); 
	
	invertedFile invertedfile;
	invertedfile.openForRead();
	
    processQuery(argv[1], queries, numTokens);
	invertedfile.setNumDictRecords();
    cout << "Processing Query Tokens..." << endl << endl;
	//Find queries
    for (int i = 0; i < numTokens; ++i) {
		cout << invertedfile.Find(queries[i], accumulator) << endl;
    }
	
	invertedfile.closeForRead();
	
	cout << "Top 10 (or less if applicable) ranking documents for query: " << endl;
	accumulator.SortByWeight();
	accumulator.Print();
	accumulator.WriteRecordFile();
    delete[] queries;
    return 0;
}
