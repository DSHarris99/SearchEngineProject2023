
#include "invertedFile.h"
#include <iostream>
using namespace std;

// Constructor - initialize all num records to -1.
invertedFile::invertedFile() {
    numMapRecords = -1;
    numDictRecords = -1;
    numPostRecords = -1;
}

// Destructor - Do nothing.
invertedFile::~invertedFile() {
    // Do nothing
}

// Open functions
void invertedFile::openForWrite() {
    // Set num records map to 0.
    numMapRecords = 0;
    numDictRecords = 0;
    numPostRecords = 0;

    // open files for writing.
    omapFile.open(mapFilename);
    odictFile.open(dictFilename);
    opostFile.open(postFilename);

    // Set file open boolean to true.
    isOpen = true;
}

void invertedFile::openForRead() {
    // reads the number of records variables for the map, dict, and post files
    iconfigFile.open(configFilename);
    iconfigFile >> numMapRecords >> numDictRecords >> numPostRecords;
    iconfigFile.close();

    // open files for writing.
    imapFile.open(mapFilename);
    idictFile.open(dictFilename);
    ipostFile.open(postFilename);

    // Set file open boolean to true.
    isOpen = true;
}

// Close functions
void invertedFile::closeForWrite() {
    // writes the number of records variables for the map, dict, and post files
    oconfigFile.open(configFilename);
    oconfigFile << numMapRecords << " " << numDictRecords << " " << numPostRecords;
    oconfigFile.close();

    // closes the dict, post, and map files
    omapFile.close();
    odictFile.close();
    opostFile.close();

    // set num records to -1 for dict post and map.
    numMapRecords = -1;
    numDictRecords = -1;
    numPostRecords = -1;

    // Set file open boolean to false.
    isOpen = false;
}

void invertedFile::closeForRead() {
    // closes the dict, post, and map files
    imapFile.close();
    idictFile.close();
    ipostFile.close();

    // set num records to -1 for dict post and map.
    numMapRecords = -1;
    numDictRecords = -1;
    numPostRecords = -1;

    // Set file open boolean to false.
    isOpen = false;
}


/*  -----------------------------------------------------------------------------------------------------------------------------------------------------
                                                        WRITE FUNCTIONS
    -----------------------------------------------------------------------------------------------------------------------------------------------------
*/

// Write to Map File
void invertedFile::writeMapRecord(const int docId, const string filename) {
    if (isOpen) {
        omapFile << setw(4) // 4 characters for docId
                << docId
                << " " // 1 character for space
                << setw(15)
                << filename.substr(0, min((int)filename.length(), 15)) // 15 characters for filename, truncate the rest.
                << endl; // 1 character for newline.
        
        numMapRecords++;
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

void invertedFile::writeDictRecord(const string term, const int numdocs, const int start) {
    if (isOpen) {
        odictFile << setw(20) // 20 characters term
                << term.substr(0, min((int)term.length(), 20))
                << " " // 1 character space
                << setw(4) // 4 character numdocs
                << numdocs
                << " " // 1 character space
                << setw(4) // 4 character start
                << start
                << endl; // 1 character newline
        numDictRecords++;
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

// Write to Post File
void invertedFile::writePostRecord(int docId, double weight) {
    if (isOpen) {
        opostFile << setw(4) // 4 characters for docId
                  << docId
                  << " " // 1 character for space
                  << right << setw(6); // 8 characters for weight, right-justified

        // Check if weight is 0 and format accordingly
        if (weight == 0.0) {
            opostFile << "0.0000";
        } else {
            // Round weight to 4 decimal places and print
            opostFile << fixed << setprecision(4) << round(weight * 10000.0) / 10000.0;
        }

        opostFile << endl;
        numPostRecords++;
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

/*  -----------------------------------------------------------------------------------------------------------------------------------------------------
                                                        READ FUNCTIONS
    -----------------------------------------------------------------------------------------------------------------------------------------------------
*/

// Read a Map Record
void invertedFile::readMapRecord(const int recordNum, int &docId, string &filename) {

    if (isOpen) {
        if (recordNum >= 0 && recordNum <= numMapRecords) {
            imapFile.seekg(recordNum * MAP_RECORD_SIZE, ios::beg);
            imapFile >> docId >> filename;
        } else {
            docId = -1;
            filename = "";
        }
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

// Read a Dict Record
void invertedFile::readDictRecord(const int recordNum, string &term, int &numdocs, int &start) {
    if (isOpen) {
        if (recordNum >= 0 && recordNum <= numDictRecords) {
            idictFile.seekg(0, ios::beg);
            idictFile.seekg(recordNum * DICT_RECORD_SIZE, ios::beg);
            idictFile >> term >> numdocs >> start;
        } else {
            term = "empty";
            numdocs = -1;
            start = -1;
        }
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

// Read a Post Record
void invertedFile::readPostRecord(const int recordNum, int &docId, double &weight) {
    if (isOpen) {
        if (recordNum >= 0 && recordNum <= numPostRecords) {
            ipostFile.seekg(recordNum * POST_RECORD_SIZE, ios::beg);
            ipostFile >> docId >> weight;
        } else {
            docId = -1;
            weight = -1;
        }
    } else {
        cout << "ERROR: File is not open." << endl;
    }
}

int invertedFile::getNumPostRecords() const {
    return numPostRecords;
}
int invertedFile::getNumDictRecords() const {
    return numDictRecords;
}
int invertedFile::getNumMapRecords() const {
    return numMapRecords;
}
int invertedFile::getPostRecordSize() const {
    return POST_RECORD_SIZE;
}

//----------------------------------------------------------------------------------------

string invertedFile::Find(const string& term, Accumulator& accumulator) {
    if (!isOpen) {
        cout << "ERROR: File is not open." << endl;
        return "";
    }

    unsigned long hashValue = calculateHash(term);
    long estimatedPosition = hashValue * DICT_RECORD_SIZE;
    long currentPosition = estimatedPosition;

    idictFile.seekg(0, ios::end);
    long fileSize = idictFile.tellg();
    idictFile.seekg(estimatedPosition, ios::beg);

    string line, foundTerm;
    int num_docs, start;
    bool hasWrapped = false;

    while (true) {
        getline(idictFile, line);
        if (!idictFile) {
            if (hasWrapped) break; //stop if already wrapped once

            idictFile.clear(); 
            idictFile.seekg(0, ios::beg); 
            currentPosition = 0;
            hasWrapped = true; 
            continue;
        }

        istringstream lineStream(line);
        lineStream >> foundTerm;

        if (foundTerm.empty() || foundTerm == "-1") continue; // skip empty lines

        if (foundTerm == term) {
            lineStream >> num_docs >> start;
            string postRecords;
            ipostFile.clear();
            ipostFile.seekg(start * POST_RECORD_SIZE, ios::beg);

            for (int i = 0; i < num_docs; ++i) {
                string postLine;
                getline(ipostFile, postLine);
                istringstream postLineStream(postLine);
                int docId;
                double weight;
                postLineStream >> docId >> weight;

                string mapRecord, documentName;
                imapFile.clear();
                imapFile.seekg(docId * MAP_RECORD_SIZE, ios::beg);
                getline(imapFile, mapRecord);
                istringstream mapLineStream(mapRecord);
                int mapDocId;
                mapLineStream >> mapDocId >> documentName; 

                accumulator.InsertOrUpdate(docId, documentName, weight);
            }
            
            return "Term '" + term + "' found in " + to_string(num_docs) + " documents.";
        }
        currentPosition = idictFile.tellg();
        if (hasWrapped && currentPosition >= estimatedPosition) break; // stop if file has wrapped and reached back to the estimated start position
    }

    return "Term '" + term + "' not found";
}




unsigned long invertedFile::calculateHash(const string& term) {
    unsigned long hashValue = 0;

    //same logic as used in HashTable::Find() for generating a hash value
    for (long unsigned i = 0; i < term.length(); i++) {
        hashValue = (hashValue * 19) + term[i];
    }

    return hashValue % getNumDictRecords();
}


int invertedFile::setNumDictRecords() {
	const string dictFilename = "dict.txt";
	const int DICT_RECORD_SIZE = 31;

	ifstream dictFile(dictFilename, ios::binary | ios::ate); // Open file for reading at the end
	if (!dictFile.is_open()) {
		cerr << "Unable to open file: " << dictFilename << endl;
		return -1; 
	}

	// get the size of dict.txt
	streampos fileSize = dictFile.tellg();
	dictFile.close();

	// calculate # records in dict.txt
    numDictRecords = static_cast<int>(fileSize) / DICT_RECORD_SIZE;
	return numDictRecords;
}