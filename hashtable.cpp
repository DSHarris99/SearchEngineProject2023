#include <assert.h>
#include <iostream>
#include <fstream>

#include "hashtable.h"

using namespace std;
/*-------------------------- Constructors/Destructors ----------------------*/

HashTable::HashTable( const HashTable &ht )
{
   size = ht.size;                    // set the size of the array
   used = ht.used;                    // set the size of the array
   if((hashtable = new StringIntPair[size]) == NULL)
       cout << "Out of memory at HashTable::HashTable(const HashTable)" << endl;
   assert( hashtable != 0 );

   for (unsigned long i=0; i < size; i++)     // make a _copy_ of the array elements
   {
      hashtable[i].key = ht.hashtable[i].key;
	  hashtable[i].num_docs = ht.hashtable[i].num_docs;
      hashtable[i].data = ht.hashtable[i].data;
   }
   
}
           
HashTable::HashTable(const unsigned long NumKeys)
{
   // allocate space for the table, init to null key
   size = NumKeys * 3;   // we want the hash table to be 2/3 empty
   used = 0;
   collisions = 0;
   lookups = 0;
   if((hashtable = new StringIntPair[size]) == NULL)
      cout << "Out of memory at HashTable::HashTable(unsigned long)" << endl;
   assert( hashtable != 0 );

   // initialize the hashtable
   for (unsigned long i=0; i < size; i++)
   {
      hashtable[i].key = "";
	  hashtable[i].num_docs = 0;
      hashtable[i].data = 0;
   }
}

HashTable::~HashTable()
{
   delete [] hashtable;
}

/*-------------------------- Accessors ------------------------------------*/

void HashTable::Print(const char *filename) const
{
   ofstream fpout(filename); 

   // Print out the non-zero contents of the hashtable
   for ( unsigned long i=0; i < size; i++ )
   {  
      if ( !(hashtable[i].key == ""))
          fpout << hashtable[i].key << " "
                << hashtable[i].data << endl;
   }
   fpout.close();
   //cout << "Collisions: " << collisions << ", Used: " << used <<  ", Lookups: " << lookups << endl;
}

int HashTable::GetData(const string Key)
{
unsigned long Index;

 lookups++; 
 Index = Find(Key);
 if (hashtable[Index].key == "")
    return -1;
 else   
    return (hashtable[Index].data);
}

void HashTable::GetUsage(int &Used, int &Collisions, int &Lookups) const
{
   Used = used;
   Collisions = collisions; 
   Lookups = lookups;
}


unsigned long HashTable::Find (const string Key) 
{
unsigned long Sum = 0;
unsigned long Index;

   // add all the characters of the key together
   for (long unsigned i=0; i < Key.length(); i++)
      Sum = (Sum * 19) + Key[i];  // Mult sum by 19, add byte value of char
  
   Index = Sum % size;

   // Check to see if word is in that location
   // If not there, do linear probing until word found
   // or empty location found.
   while (((hashtable[Index].key) != Key) &&
          ((hashtable[Index].key) != "") ) 
   {
      Index = (Index+1) % size;
      collisions++;
   }
   
   return Index;
}
unsigned long HashTable::GetSize() const {
    return size;
}

bool HashTable::IsUsed(unsigned long index) const {
    // Check if the entry at the given index is used
    if (index < size) {
        return !hashtable[index].key.empty();
    }
    return false;
}

const char* HashTable::GetKey(unsigned long index) const {
    // Get the key at the given index
    if (index < size) {
        return hashtable[index].key.c_str();
    }
    return nullptr;
}

void HashTable::InsertOrUpdate(const string Key, const int Data)
{
    unsigned long Index = Find(Key);

    // Check if the key already exists
    if (hashtable[Index].key == Key)
    {
        // Key already exists, update the value (frequency count)
        hashtable[Index].data += Data;
        //cout << "Token ='" << Key << "' already exists in the hashtable, it has been incremented to " << "Frequency ='" << hashtable[Index].data << "'" << endl;
    }
    else
    {
        // Key does not exist, insert a new key-value pair
        if (used >= size)
        {
            cerr << "The hashtable is full; cannot insert.\n";
        }
        else
        {
            // Find an empty slot or use linear probing
            while (!hashtable[Index].key.empty())
            {
                Index = (Index + 1) % size;
                collisions++;
            }

            // Insert the new key-value pair
            hashtable[Index].key = Key;
            hashtable[Index].data = Data;
            used++;
        }
    }
}


bool HashTable::tokenExists(const string token)
{
    for (unsigned long i = 0; i < size; i++)
    {
        if (hashtable[i].key == token)
        {
            return true; // Token exists in the hashtable
        }
    }
    return false; // Token does not exist in the hashtable
}


void HashTable::InsertOrUpdate(const string& key, const int data, const int num_docs) {
    unsigned long index = Find(key);

    // Handle case if the key already exists in the hashtable
    if (hashtable[index].key == key) {
        hashtable[index].data += data;
        hashtable[index].num_docs = num_docs;
        return;
    }

    // Handle case if the hashtable is full
    if (used >= size) {
        cerr << "The hashtable is full; cannot insert." << endl;
        return;
    }

    // Handle case if the slot is empty
    if (hashtable[index].key.empty()) {
        hashtable[index].key = key;
        hashtable[index].data = data;
        hashtable[index].num_docs = num_docs;
        used++;
        return;
    }

    // Handle linear probing for occupied slots
    while (hashtable[index].key != key && !hashtable[index].key.empty()) {
		cout << "Probing at index: " << index << endl; // Add this
        index = (index + 1) % size;
        collisions++;
    }

    if (hashtable[index].key == key) {
        hashtable[index].data += data;
        hashtable[index].num_docs = num_docs;
    } else {
        hashtable[index].key = key;
        hashtable[index].data = data;
        hashtable[index].num_docs = num_docs;
        used++;
    }
}


void HashTable::PrintGlobalTable()
{
    for (unsigned long i = 0; i < size; i++) {
        if (IsUsed(i)) {
            // Used slot
            string key = hashtable[i].key;
            int num_docs = GetNumDocs(key);
            int data = GetData(key);
            cout << "Token: " << key << " | NumDocs: " << num_docs << " | Frequency: " << data << endl;
        } else {
            // Unused slot
            cout << "Token: EMPTY | NumDocs: -1 | Frequency: -1" << endl;
        }
    }
}

void HashTable::PrintDocumentTable()
{
	for (unsigned long i = 0; i < size; i++) {
		if (IsUsed(i)) {
			// Used slot
			string key = hashtable[i].key;
			int data = GetData(key);
			cout << "Token: " << key << " | Frequency: " << data << endl;
		} else {
			// Unused slot
			cout << "Token: EMPTY | Frequency: -1" << endl;
		}
	}
}

int HashTable::GetNumDocs(const string Key)
{
    unsigned long Index;
    lookups++;
    Index = Find(Key);
    if (hashtable[Index].key == "")
        return -1;
    else
        return (hashtable[Index].num_docs);
}

void HashTable::Clear() {
    for (unsigned long i = 0; i < size; i++) {
        hashtable[i].key = "";
        hashtable[i].data = 0;
        hashtable[i].num_docs = 0;
    }
    used = 0;
    collisions = 0;
    lookups = 0;
}

int HashTable::GetUsed() {
	return used;
}

long HashTable::GetIndex(const string Key)
{
    unsigned long Index = Find(Key);
    
    // Check if the key actually exists in the hashtable before returning the index
    if (tokenExists(Key))
        return Index;

    // If the key doesn't exist, return -1
    return -1;
}


