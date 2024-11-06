#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath> 
#include "accumulator.h"
using namespace std;

class invertedFile {
	
    public:
        invertedFile(); 
        ~invertedFile(); 

        void openForWrite();
        void openForRead();

        void closeForWrite();
        void closeForRead();

        void writeMapRecord(const int docId, string filename);
        void writeDictRecord(const int recordNum, string term, const int numdocs, const int start);
        void writeDictRecord(const string term, const int numdocs, const int start);
        void writePostRecord(const int docId, const double weight);

        void readMapRecord(const int recordNum, int &docId, string &filename);
        void readDictRecord(const int recordNum, string &term, int &numdocs, int &start);
        void readPostRecord(const int recordNum, int &docId, double &weight);
		
		int getNumPostRecords() const;
		int getNumDictRecords() const;
		int getNumMapRecords() const;
		int getPostRecordSize() const;
		int setNumDictRecords(); 
		unsigned long calculateHash(const string& term);
		
		string Find(const string& term, Accumulator& accumulator);
    private:
        ifstream imapFile;
        ifstream idictFile;
        ifstream ipostFile;
        ifstream iconfigFile;

        ofstream omapFile;
        ofstream odictFile;
        ofstream opostFile;
        ofstream oconfigFile;
		

	
        const int MAP_RECORD_SIZE = 21;
        const int DICT_RECORD_SIZE = 31;
        const int POST_RECORD_SIZE = 12;
        int numMapRecords;
        int numDictRecords;
        int numPostRecords;
        const string configFilename = "config.txt";
        const string mapFilename = "map.txt";
        const string dictFilename = "dict.txt";
        const string postFilename = "post.txt";
        bool isOpen;
};
