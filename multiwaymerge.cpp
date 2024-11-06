#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <cmath>
#include "hashtable.h" 
#include "invertedFile.h"


using namespace std;
const unsigned long Num_Keys = 15; // Replace with desired size
    unsigned long uniqueTokens = 0;
static int Num_Docs_In_Collection = 0;

double calculateIDF(int N, int df) {
    return (1 + log10(N / (df * 1.0)));
}

int readCumulativeTotal() {
    int cumulativeTotal = 0;
    ifstream file("cumulative_total.txt");
    if (file.is_open()) {
        file >> cumulativeTotal;
        file.close();
    }
    return cumulativeTotal;
}

int countDocsInDirectory(const string& inputDirectory) {
    DIR* dir;
    struct dirent* entry;
    int count = 0;
	int* DocumentTokens = new int[Num_Docs_In_Collection]();
    // Open the input directory
    if ((dir = opendir(inputDirectory.c_str())) != nullptr) {
        // Iterate through the input files
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {  // Count only regular files
                count++;
            }
        }
        closedir(dir);
    } else {
        cerr << "Error: Cannot open input directory: " << inputDirectory << endl;
    }
    return count;
}

struct TokenEntry {
    std::string token;
    int docID;
    int frequency;
};

//Multiway merge algorithm
void multiwayMerge(const string& inputDirectory, HashTable& globalHashTable, invertedFile& invertedFile) {
    DIR* dir;
    struct dirent* entry;
    
	int* DocumentTokens = new int[Num_Docs_In_Collection]();
    // Open the input directory
    if ((dir = opendir(inputDirectory.c_str())) != nullptr) {
        int doc_id = 0;

        // Define a buffer array for each document.
        TokenEntry* docBuffers = new TokenEntry[Num_Docs_In_Collection];
		ifstream* streams = new ifstream[Num_Docs_In_Collection];

        // Load the first lines from all docs into the buffer.
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type != DT_REG) {
                continue;
            }
			invertedFile.writeMapRecord(doc_id, entry->d_name);
            string inputFile = inputDirectory + "/" + string(entry->d_name);
            streams[doc_id].open(inputFile);
            if (streams[doc_id] >> docBuffers[doc_id].token >> docBuffers[doc_id].frequency) {
                docBuffers[doc_id].docID = doc_id;
            } else {
                docBuffers[doc_id].token = ""; // Indicating end of file
            }
            
            doc_id++;
        }
		
		long* startPositions = new long[globalHashTable.GetSize()];
		long currentStartPosition = 0;
		int totalLengthForCurrentToken = 0; 
		string previousToken;  
		int current_num_docs = 0;
		//Pass 1
		cout << "Starting Pass 1..." << endl;
		while (true) {

			int minIndex = -1;
			string minToken = "";

			// Find the smallest token across all docBuffers
			for (int i = 0; i < Num_Docs_In_Collection; ++i) {
				if (docBuffers[i].token != "" && (minToken == "" || docBuffers[i].token < minToken)) {
					minToken = docBuffers[i].token;
					minIndex = i;
				}
			}
			if (minIndex == -1) {
				// All files are done
				cout << "Pass 1 finished..." << endl;
				break;
			}

			string currentToken = docBuffers[minIndex].token;
			int tf = docBuffers[minIndex].frequency;
			
            // Update total tokens for the current document
            DocumentTokens[docBuffers[minIndex].docID] += tf;			
			// Look up minToken in global hashtable
			
			int num_docs = globalHashTable.GetNumDocs(minToken);


			if (num_docs > 0) {
				globalHashTable.InsertOrUpdate(minToken, tf, num_docs + 1);
			} else {
				globalHashTable.InsertOrUpdate(minToken, tf, 1);
			}
			
			// Reload buffer with next record from the file.
			if (!(streams[minIndex] >> docBuffers[minIndex].token >> docBuffers[minIndex].frequency)) {
				docBuffers[minIndex].token = ""; // Indicates end of file
			}
		}
		// At the end of Pass 1, close all open streams
		for (int i = 0; i < Num_Docs_In_Collection; ++i) {
			streams[i].close();
		}
		doc_id = 0;
		if ((dir = opendir(inputDirectory.c_str())) != nullptr) {
			while ((entry = readdir(dir)) != nullptr) {
				if (entry->d_type != DT_REG) {
					continue;
				}
				string inputFile = inputDirectory + "/" + string(entry->d_name);
				streams[doc_id].open(inputFile);
				if (streams[doc_id] >> docBuffers[doc_id].token >> docBuffers[doc_id].frequency) {
					docBuffers[doc_id].docID = doc_id;
				} else {
					docBuffers[doc_id].token = ""; // Indicating end of file
				}
				doc_id++;
			}
		}
		//Pass 2
		cout << "Starting Pass 2..." << endl;
		while (true) {
			int minIndex = -1;
			string minToken = "";

			// Find the smallest token across all docBuffers
			for (int i = 0; i < Num_Docs_In_Collection; ++i) {
				if (docBuffers[i].token != "" && (minToken == "" || docBuffers[i].token < minToken)) { //This is where pass 2 cannot enter, and because of this, breaks right away
					minToken = docBuffers[i].token;
					minIndex = i;
				}
			}
			if (minIndex == -1) {
				// All files are done
				cout << "Pass 2 finished... " << endl;
				break;
			}

			string currentToken = docBuffers[minIndex].token;
			int tf = docBuffers[minIndex].frequency;
			
			// Look up minToken in global hashtable
			int num_docs = globalHashTable.GetNumDocs(minToken);

			if (currentToken != previousToken) {
				currentStartPosition += totalLengthForCurrentToken;
				unsigned long tokenIndex = globalHashTable.GetIndex(currentToken);
				startPositions[tokenIndex] = currentStartPosition;
				totalLengthForCurrentToken = 0;  // reset the accumulator
			}
			//postings position for dict is determined by line #
			totalLengthForCurrentToken += 1;
			
			// Calculate term weight tf*idf and write postings record for the token.
			double idf = calculateIDF(Num_Docs_In_Collection, globalHashTable.GetNumDocs(currentToken));
			double rtf = ((tf*1.0)/(DocumentTokens[docBuffers[minIndex].docID]*1.0));
			double rtf_idf = rtf * idf;

			invertedFile.writePostRecord(docBuffers[minIndex].docID, rtf_idf);
			
			previousToken = currentToken;

			// Reload buffer with next record from the file.
			if (!(streams[minIndex] >> docBuffers[minIndex].token >> docBuffers[minIndex].frequency)) {
				docBuffers[minIndex].token = ""; // Indicates end of file
			}
		}
		
		// Writing to the dict file
		for (unsigned long i = 0; i < globalHashTable.GetSize(); i++) {
			if (globalHashTable.IsUsed(i)) {
				string key = globalHashTable.GetKey(i);
				int num_docs = globalHashTable.GetNumDocs(key);

				// Get the index for this key from globalHashTable and write to dict
				unsigned long tokenIndex = globalHashTable.GetIndex(key);
				long startPositionForThisToken = startPositions[tokenIndex];
				invertedFile.writeDictRecord(key, num_docs, startPositionForThisToken);
				uniqueTokens++;
			}
			else {
				// Write empty records for unused slots
				invertedFile.writeDictRecord("", -1, -1);
			}
		}
        closedir(dir);
    } 
	else {
        cerr << "Error: Cannot open input directory: " << inputDirectory << endl;
    }
	cout << "DocID = 0; " << "Total Tokens in Document = " << DocumentTokens[0] << endl;
	cout << "DocID = 1; " << "Total Tokens in Document = " << DocumentTokens[1] << endl;
	cout << "DocID = 2; " << "Total Tokens in Document = " << DocumentTokens[2] << endl;
	cout << "DocID = 3; " << "Total Tokens in Document = " << DocumentTokens[3] << endl;
	delete[] DocumentTokens;
}

int main(int argc, char* argv[]) {

    if (argc <= 1) {
        cout << "Usage: " << argv[0] << " input_directory" << endl;
        return 1;
    }

    string inputDirectory = argv[1];
    cout << "Input directory: " << inputDirectory << endl;
	Num_Docs_In_Collection = countDocsInDirectory(inputDirectory);
	
	//HashTable globalHashTable(Num_Keys); 
    HashTable globalHashTable(Num_Keys*Num_Docs_In_Collection); 
	
    invertedFile invertedFile; 
    invertedFile.openForWrite();
	cout << "Number of documents in collection: " << Num_Docs_In_Collection << endl;
    cout << "Starting Multiway merge. For large amounts of files, this may take a while, so please wait..." << endl;
    multiwayMerge(inputDirectory, globalHashTable, invertedFile);
    cout << "Multiway merge complete." << endl << endl;

    invertedFile.closeForWrite();

	cout << "Global HashTable size: " << globalHashTable.GetSize() 
		 << ", Used: " << globalHashTable.GetUsed() << endl;
	//globalHashTable.PrintGlobalTable(); 

    cout << "Total number of tokens in collection: " << readCumulativeTotal() << endl;
    cout << "Total number of unique tokens in collection: " << uniqueTokens << endl;
    return 0;
}
