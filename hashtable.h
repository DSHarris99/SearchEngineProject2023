using namespace std;

class HashTable {
public:
   HashTable (const HashTable& ht );       // constructor for a copy
   HashTable(const unsigned long NumKeys);  // constructor of hashtable 
   ~HashTable();                           // destructor
   void Print (const char *filename) const;  
   
   int GetData (const string Key); 
   void GetUsage (int &Used, int &Collisions, int &Lookups) const;

   unsigned long GetSize() const;           // Get the size of the hashtable
   bool tokenExists(const string token); // Check if entry already exists
   bool IsUsed(unsigned long index) const;   // Check if an entry is used
   const char* GetKey(unsigned long index) const; // Get the key at an index
   
   void InsertOrUpdate(const string Key, const int Data);  // Original InsertOrUpdate function
   void InsertOrUpdate(const string& Key, const int Data, const int num_docs);  // Overloaded InsertOrUpdate function

   void PrintGlobalTable();
   int GetUsed();
   int GetNumDocs(const string Key);
   void PrintDocumentTable();
   void Clear();
   long GetIndex(const string Key);
   
   
protected:
   struct StringIntPair // the datatype stored in the hashtable
   {
      string key;
      int data;
      int num_docs;
   };
   unsigned long Find (const string Key); // the index of the ddr in the hashtable
private:
   StringIntPair *hashtable;        // the hashtable array itself
   unsigned long size;              // the hashtable size
   unsigned long used;
   unsigned long collisions;
   unsigned long lookups;
};
