#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;

#include "OAHashTable.h"

const unsigned ID_LEN = 6;
struct Person
{
  char ID[ID_LEN + 1];
  char lastName[26];   
  char firstName[26];  
  float salary;        
  unsigned years;           
};

struct Person PEOPLE[] = {
  {"101001", "Faith", "Ian", 80000, 10},
  {"102001", "Tufnel", "Nigel", 90000, 12},
  {"103001", "Savage", "Viv", 50000, 4},
  {"104001", "Shrimpton", "Mick", 50000, 4},
  {"105001", "Besser", "Joe", 40000, 1},
  {"106001", "Smalls", "Derek", 80000, 10},
  {"107001", "St.Hubbins", "David", 90000, 12},
  {"108001", "Fleckman", "Bobbi", 120000, 8},
  {"109001", "Eton-Hogg", "Denis", 250000, 22},
  {"110001", "Upham", "Denny", 60000, 5},
  {"111001", "McLochness", "Ross", 60000, 5},
  {"112001", "Pudding", "Ronnie", 50000, 2},
  {"113001", "Schindler", "Danny", 60000, 3},
  {"114001", "Pettibone", "Jeanine", 85000, 3},
  {"115001", "Fame", "Duke", 95000, 8},
  {"116001", "Fufkin", "Artie", 45000, 1},
  {"117001", "DiBergi", "Marty", 15000, 7},
  {"118001", "Floyd", "Pink", 25000, 6},
  {"119001", "Zeppelin", "Led", 35000, 3},
  {"120001", "Mason", "Nick", 15000, 7},
  {"121001", "Wright", "Richard", 17000, 9},
  {"122001", "Waters", "Roger", 10000, 3},
  {"123001", "Gilmore", "David", 19000, 5}
};

Person **PersonRecs;

void FreePersonRecs()
{
  unsigned count = sizeof(PEOPLE) / sizeof(*PEOPLE);

  for (unsigned i = 0; i < count; i++)
    delete PersonRecs[i];

  delete[] PersonRecs;
}

void FillPersonRecs()
{
  unsigned count = sizeof(PEOPLE) / sizeof(*PEOPLE);
  PersonRecs = (Person **) new Person[count];

  unsigned index = 0;
  for (unsigned i = 0; i < count; i++)
  {
    Person *person = new Person;
    strcpy(person->ID, PEOPLE[i].ID);
    strcpy(person->lastName, PEOPLE[i].lastName);
    strcpy(person->firstName, PEOPLE[i].firstName);
    person->salary = PEOPLE[i].salary;
    person->years = PEOPLE[i].years;
    PersonRecs[index++] = person;
  }
}

const unsigned MAX_PERSONS = 10000;
//const unsigned NUM_PEOPLE = sizeof(PEOPLE) / sizeof(*PEOPLE);

int GetRandom(int Low, int High)
{
  return rand() % (High - Low + 1) + Low;
}

unsigned GetRandomIndex()
{
  return static_cast<unsigned>(GetRandom(0, MAX_PERSONS - 1));
}

void FillPersonRecs2()
{
  PersonRecs = (Person **) new Person[MAX_PERSONS];

  unsigned index = 0;
  for (unsigned i = 0; i < MAX_PERSONS; i++)
  {
    Person *person = new Person;
    strcpy(person->lastName, PEOPLE[0].lastName);
    strcpy(person->firstName, PEOPLE[0].firstName);
    person->salary = (float)GetRandom(1000, 20000);
    person->years = PEOPLE[0].years;
    PersonRecs[index++] = person;
  }
}

ostream& operator<<(ostream &os, const Person &person)
{
  char buffer[1000];
  sprintf(buffer, "Key: %8s, Name: %12s, %12s    Salary:%7.0f, Years: %2d", person.ID, person.lastName,
                  person.firstName, person.salary, person.years);
  os << buffer;
  return os;
}

unsigned ConstantHash(const char *, unsigned)
{
  return 1;
}

unsigned ReflexiveHash(const char *Key, unsigned TableSize)
{
  return static_cast<unsigned>(atoi(Key)) % TableSize;
}

void RevString(char *Key)
{
  unsigned len = static_cast<unsigned>(strlen(Key));
  for (unsigned i = 0; i < len / 2; i++)
  {
    char t = Key[i];
    Key[i] = Key[len - i - 1];
    Key[len - i - 1] = t;
  }
}

unsigned PJWHash(const char *Key, unsigned TableSize)
{
    // Initial value of hash
  unsigned hash = 0;

    // Process each char in the string
  while (*Key)
  {
      // Shift hash left 4
    hash = (hash << 4);
      
      // Add in current char
    hash = hash + static_cast<unsigned>((*Key));   

      // Get the four high-order bits
    unsigned bits = hash & 0xF0000000;

      // If any of the four bits are non-zero,
    if (bits)
    {
        // Shift the four bits right 24 positions (...bbbb0000)
        // and XOR them back in to the hash
      hash = hash ^ (bits >> 24); 

        // Now, XOR the four bits back in
      hash = hash ^ bits;         
    }

      // Next char
    Key++;   
  }

    // Modulo so hash is within 0 - TableSize
  return hash % TableSize;  
}

unsigned SimpleHash(const char *Key, unsigned TableSize)
{
    // Initial value of hash
  unsigned hash = 0;

    // Process each char in the string
  while (*Key)
  {
      // Add in current char
    hash += static_cast<unsigned>(*Key);

      // Next char
    Key++;
  }

    // Modulo so hash is within the table
  return hash % TableSize;
}

unsigned RSHash(const char *Key, unsigned TableSize)
{
  unsigned hash = 0;         // Initial value of hash
  unsigned multiplier = 127; // Prevent anomalies

    // Process each char in the string
  while (*Key)
  {
      // Adjust hash total
    hash = hash * multiplier;

      // Add in current char and mod result
    hash = (hash + static_cast<unsigned>(*Key)) % TableSize;

      // Next char
    Key++;
  }

    // Hash is within 0 - TableSize
  return hash;
}

unsigned UHash(const char *Key, unsigned TableSize)
{
  unsigned hash = 0;       // Initial value of hash
  unsigned rand1 = 31415; // "Random" 1
  unsigned rand2 = 27183; // "Random" 2

    // Process each char in string
  while (*Key)
  {
      // Multiply hash by random
    hash = hash * rand1;

      // Add in current char, keep within TableSize
    hash = (hash + static_cast<unsigned>(*Key)) % TableSize;

      // Update rand1 for next "random" number
    rand1 = (rand1 * rand2) % (TableSize - 1);

      // Next char
    Key++;
  }
    // Hash value is within 0 - TableSize - 1
  return hash;
}

struct HashData
{
  HASHFUNC Fn;
  const char *Name;
};

enum HASHFUNCS {NONE, CONSTANT, REFLEXIVE, SIMPLE, RS, UNIVERSAL, PJW};

HashData HashingFuncs[] = {
  {0, "None (Linear probing)"}
 ,{ConstantHash, "Constant Hash (1)"}
 ,{ReflexiveHash, "Reflexive Hash"}
 ,{SimpleHash, "Simple Hash"}
 ,{RSHash, "RS Hash"}
 ,{UHash, "Universal Hash"}
 ,{PJWHash, "PJW Hash"}
};

void Dispose(Person *)
{
}

template <typename T>
void DumpTable(OAHashTable<T> &ht)
{
  char buffer[80];
  const typename OAHashTable<T>::OAHTSlot *slots = ht.GetTable();
  HASHFUNC phf = ht.GetStats().PrimaryHashFunc_;
  HASHFUNC shf = ht.GetStats().SecondaryHashFunc_;
  for (unsigned i = 0; i < ht.GetStats().TableSize_; i++)
  {
    const typename OAHashTable<T>::OAHTSlot *slot = &slots[i]; 
    if (slot->State == OAHashTable<T>::OAHTSlot::OCCUPIED)
    {
      if (!shf)
        sprintf(buffer, "Slot: %3d, Key: %s (%d)\n", i, slot->Key, phf(slot->Key, ht.GetStats().TableSize_));
      else
        sprintf(buffer, "Slot: %3d, Key: %s (%d:%d)\n", i, slot->Key, phf(slot->Key, ht.GetStats().TableSize_), 
                        shf(slot->Key, ht.GetStats().TableSize_ - 1) + 1);
      cout << buffer;
    }
    else if (slot->State == OAHashTable<T>::OAHTSlot::DELETED)
    {
      sprintf(buffer, "Slot: %3d, Key: -- Deleted --\n", i);
      cout << buffer;
    }
    else // UNOCCUPIED
    {
      sprintf(buffer, "Slot: %3d, Key: *** Empty ***\n", i);
      cout << buffer;
    }
  }
}

template <typename T>
void DumpStats(OAHashTable<T> &ht, ostream &os = cout)
{
  os << "Number of probes: " << ht.GetStats().Probes_ << endl;
  os << "Number of expansions: " << ht.GetStats().Expansions_ << endl;
  os << "Items: " << ht.GetStats().Count_ << ", TableSize: " << ht.GetStats().TableSize_ << endl;
  os << "Load factor: " << setprecision(3) << (double) ht.GetStats().Count_ / (double) ht.GetStats().TableSize_ << endl;
}

void TestALot(HashData *phd, HashData *shd)
{
  cout << endl << "==================== TestALot ====================" << endl;

  HASHFUNC phf = phd->Fn;
  HASHFUNC shf = shd->Fn;

  unsigned initial_size = 13;
  double max_load_factor = 0.85;
  double growth_factor = 1.5;

  cout << endl << "Creating table:" << endl;
  cout << "Primary hash function: " << phd->Name << endl;
  cout << "Secondary hash function: " << shd->Name << endl;
  cout << "Initial size: " << initial_size << endl;
  cout << "Max load factor: " << max_load_factor << endl;
  cout << "Growth factor: " << growth_factor << endl << endl;

  OAHTDeletionPolicy policy = MARK;
  if (!shf)
    policy = PACK;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(initial_size, phf, shf, max_load_factor, growth_factor, policy, 0));
  DumpTable<T>(ht);
  DumpStats<T>(ht);
  cout << endl;

  try
  {
    unsigned size = 10;
    const char *keys[100];

      // Insert some items
    for (unsigned i = 0; i < size; i++)
    {
      Person *person = PersonRecs[i];
      keys[i] = person->ID;
      ht.insert(person->ID, person);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    Person *person;
    const char *key = "";

      // Find some valid items
    key = keys[0];
    cout << endl << "Finding key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ")" << endl;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding key " << key << " in TestALot" << endl << endl;
    }
    DumpStats<T>(ht);

    key = keys[2];
    cout << endl << "Finding key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ")" << endl;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding key " << key << " in TestALot" << endl << endl;
    }
    DumpStats<T>(ht);
    
    key = keys[9];
    cout << endl << "Finding key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ")" << endl;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding key " << key << " in TestALot" << endl << endl;
    }
    DumpStats<T>(ht);

      // Try to find a bogus item
    key = "123456";
    cout << endl << "Finding key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ")" << endl;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding in TestALot" << endl << endl;
    }

    DumpStats<T>(ht);

      // Grow the table by inserting some items
    for (unsigned j = size; j < size + 3; j++)
    {
      Person *person = PersonRecs[j];
      keys[j] = person->ID;
      ht.insert(person->ID, person);
      cout << endl << "Adding a person ..." << endl;
      DumpTable<T>(ht);
      DumpStats<T>(ht);
    }

      // Delete a valid item
    key = keys[10];
    cout << endl << "Deleting key: " << key << endl;
    ht.remove(key);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

      // Delete non-existent key that causes no probing
    key = "123456";
    cout << endl << "Deleting key: " << key << endl;
    try
    {
      ht.remove(key);
    }
    catch (OAHashTableException &e)
    {
      cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened inserting in TestALot" << endl << endl;
    }
    DumpStats<T>(ht);

      // Delete non-existent key that causes probing
    key = "123457";
    cout << endl << "Deleting key: " << key << endl;
    try
    {
      ht.remove(key);
    }
    catch (OAHashTableException &e)
    {
      cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened inserting in TestALot" << endl << endl;
    }
    DumpStats<T>(ht);

      // Clear the table
    ht.clear();
    DumpTable<T>(ht);
    DumpStats<T>(ht);

  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestALot" << endl << endl;
  }
}


void TestDoubleHashing(HashData *phd, HashData *shd)
{
  cout << endl << "==================== TestDoubleHashing ====================" << endl;

  HASHFUNC phf = phd->Fn;
  HASHFUNC shf = shd->Fn;

  unsigned initial_size = 5;
  double max_load_factor = 0.95;
  double growth_factor = 2.0;

  cout << endl << "Creating table:" << endl;
  cout << "Primary hash function: " << phd->Name << endl;
  cout << "Secondary hash function: " << shd->Name << endl;
  cout << "Initial size: " << initial_size << endl;
  cout << "Max load factor: " << max_load_factor << endl;
  cout << "Growth factor: " << growth_factor << endl << endl;

  // Set to MARK, but no deletions are taking place
  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(initial_size, phf, shf, max_load_factor, growth_factor, MARK, 0));
  DumpTable<T>(ht);
  DumpStats<T>(ht);
  cout << endl;

  try
  {
    unsigned size = 9;
    const char *key;
    Person *person;

      // Insert some items
    for (unsigned i = 0; i < size; i++)
    {
      Person *person = PersonRecs[i];
      key = person->ID;
      cout << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
      ht.insert(person->ID, person);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[9];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);

    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[10];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    cout << endl;
    for (unsigned j = 11; j < 17; j++)
    {
      person = PersonRecs[j];
      key = person->ID;
      cout << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
      ht.insert(person->ID, person);
    }

    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[17];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[18];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[19];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    person = PersonRecs[20];
    key = person->ID;
    cout << endl << "Inserting key: " << key << " (" << phf(key, ht.GetStats().TableSize_) << ":" << (shf(key, ht.GetStats().TableSize_ - 1) + 1) << ")" << endl;
    ht.insert(person->ID, person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);

  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestDoubleHashing" << endl << endl;
  }
}


void TestSimpleInsert1()
{
  cout << endl << "==================== TestSimpleInsert1 ====================" << endl;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(5, UHash, RSHash, .95, 2.0, PACK, 0));
  try
  {
    for (unsigned i = 0; i < 10; i++)
    {
      Person *person = PersonRecs[i];
      ht.insert(person->ID, person);
 #if 0
      cout << "-----------------------------------------------" << endl;      
      DumpTable<T>(ht);
      DumpStats<T>(ht);
#endif
   }
    DumpTable<T>(ht);
    DumpStats<T>(ht);
  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestSimpleInsert1" << endl << endl;
  }
}

void TestSimpleInsert2()
{
  cout << endl << "==================== TestSimpleInsert2 ====================" << endl;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(11, UHash, RSHash, .95, 2.0, PACK, 0));
  try
  {
    for (unsigned i = 0; i < 11; i++)
    {
      Person *person = PersonRecs[i];
      ht.insert(person->ID, person);
#if 0
      cout << "-----------------------------------------------" << endl;      
      DumpTable<T>(ht);
      DumpStats<T>(ht);
#endif
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);
  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestSimpleInsert2" << endl << endl;
  }
}


void TestInsertConstant()
{
  cout << endl << "==================== TestInsertConstant ====================" << endl;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(3, ConstantHash, NULL, 1.0, 2.0));
  try
  {
    for (unsigned i = 0; i < 11; i++)
    {
      Person *person = PersonRecs[i];
      ht.insert(person->ID, person);
      std::cout << "========================\n";
    DumpTable<T>(ht);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);
  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestSimpleInsert" << endl << endl;
  }
}


void TestSimpleDeletePresent()
{
  cout << endl << "==================== TestSimpleDeletePresent ====================" << endl;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(11, PJWHash, NULL, 1.0, 2.0, PACK, Dispose));
  try
  {
    for (unsigned i = 0; i < 11; i++)
    {
      Person *person = PersonRecs[i];
      ht.insert(person->ID, person);
    //DumpTable<T>(ht);
    //DumpStats<T>(ht);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    const char *key = "106001";
    Person *person;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding key " << key << " in TestSimpleDeletePresent" << endl << endl;
    }

    ht.remove(key);
    DumpTable<T>(ht);
    DumpStats<T>(ht);
  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestSimpleDeletePresent" << endl << endl;
  }
}

void TestSimpleDeleteMissing()
{
  cout << endl << "==================== TestSimpleDeleteMissing ====================" << endl;

  typedef Person* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(11, PJWHash, NULL, 1.0, 2.0, PACK, Dispose));
  try
  {
    for (unsigned i = 0; i < 11; i++)
    {
      Person *person = PersonRecs[i];
      ht.insert(person->ID, person);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);

    const char *key = "999999";
    Person *person;
    try
    {
      person = reinterpret_cast<Person *> (ht.find(key));
      cout << *person << endl;
    }
    catch (OAHashTableException &e)
    {
      cout << "Key " << key << " not found. ";
      cout << "errno: " << e.code() << ", " << e.what() << endl << endl;
    }
    catch (...) {
      cout << endl << "**** Something bad happened finding key " << key << " in TestSimpleDeleteMissing" << endl << endl;
    }
    ht.remove(key);
  }
  catch (OAHashTableException &e)
  {
    cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
  }
  catch (...) {
    cout << endl << "**** Something bad happened inserting in TestSimpleDeleteMissing" << endl << endl;
  }
  DumpStats<T>(ht);
}

void FreeSimpleRec(char *rec)
{
  delete [] (char *)rec;
}
  
void TestSimpleDispose()
{
  cout << endl << "==================== TestSimpleDispose ====================" << endl;

  double lf = 0.667;
  typedef char* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(11, PJWHash, NULL, lf, 2.0, PACK, FreeSimpleRec));
  for (unsigned i = 0; i < 11; i++)
  {
    char *ID = new char[8];
    sprintf(ID, "%07d", i);
    RevString(ID);
    try 
    {
      ht.insert(ID, ID);
    }
    catch (OAHashTableException &e)
    {
      cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
      break;
    }
    catch (...) {
      cout << endl << "**** Something bad happened inserting in TestSimpleDispose" << endl << endl;
      break;
    }
  }
  DumpStats<T>(ht);
}

void TestSimpleGrow1()
{
  cout << endl << "==================== TestSimpleGrow1 ====================" << endl;

  double lf = 0.90;
  typedef char* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(5, SimpleHash, NULL, lf, 2.0, PACK, FreeSimpleRec));
  for (unsigned i = 0; i < 30; i++)
  {
    char *ID = new char[8];
    sprintf(ID, "%07d", i);
    RevString(ID);
    try 
    {
      ht.insert(ID, ID);
#if 0
      cout << "-----------------------------------------------" << endl;      
      DumpTable<T>(ht);
      DumpStats<T>(ht);
#endif
   }
    catch (OAHashTableException &e)
    {
      cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
      break;
    }
    catch (...) {
      cout << endl << "**** Something bad happened inserting in TestSimpleGrow1" << endl << endl;
      break;
    }
  }
  DumpStats<T>(ht);
}

void TestSimpleGrow2()
{
  cout << endl << "==================== TestSimpleGrow2 ====================" << endl;

  double lf = 0.90;
  typedef char* T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(5, UHash, RSHash, lf, 2.0, MARK, FreeSimpleRec));
  for (unsigned i = 0; i < 30; i++)
  {
    char *ID = new char[8];
    sprintf(ID, "%07d", i);
    RevString(ID);
    try 
    {
      ht.insert(ID, ID);
 #if 0
      cout << "-----------------------------------------------" << endl;      
      DumpTable<T>(ht);
      DumpStats<T>(ht);
#endif
   }
    catch (OAHashTableException &e)
    {
      cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
      break;
    }
    catch (...) {
      cout << endl << "**** Something bad happened inserting in TestSimpleGrow2" << endl << endl;
      break;
    }
  }
  DumpStats<T>(ht);
}

// insert/delete (MARK)/insert
void TestSimpleMarkPack(HashData *phd, HashData *shd,  OAHTDeletionPolicy policy)
{
  const char *test = "TestSimpleMarkPack";
  cout << endl << "==================== " << test << " ====================" << endl;

  HASHFUNC phf = phd->Fn;
  HASHFUNC shf = shd->Fn;

  unsigned initial_size = 17;
  double max_load_factor = 0.95;
  double growth_factor = 2.0;

  cout << endl << "Creating table:" << endl;
  cout << "Primary hash function: " << phd->Name << endl;
  cout << "Secondary hash function: " << shd->Name << endl;
  cout << "Initial size: " << initial_size << endl;
  cout << "Max load factor: " << max_load_factor << endl;
  cout << "Growth factor: " << growth_factor << endl;
  cout << "Deletion policy: ";
  if (policy == PACK)
    cout << "PACK\n";
  else
    cout << "MARK\n";
  cout << endl;

  typedef Person * T;
  OAHashTable<T> ht(OAHashTable<T>::OAHTConfig(initial_size, phf, shf, max_load_factor, growth_factor, policy, 0));
  try
  {
    Person *person = 0;
    for (unsigned i = 0; i < initial_size - 2; i++)
    {
      person = PersonRecs[i];
      ht.insert(person->ID, person);
    }
    DumpTable<T>(ht);
    DumpStats<T>(ht);
    cout << endl;

    ht.remove("110001");
    ht.remove("101001");
    ht.remove("102001");
    ht.remove("111001");

    DumpTable<T>(ht);
    DumpStats<T>(ht);
    cout << endl;

    ht.insert("122001", person);
    DumpTable<T>(ht);
    DumpStats<T>(ht);
    cout << endl;

  }
  catch (OAHashTableException &e)
  {
    std::cout << "Caught OAHashTableException in " << test << ": ";
    int value = e.code();
    if (value == OAHashTableException::E_DUPLICATE)
      std::cout << "E_DUPLICATE" << std::endl;
    else if (value == OAHashTableException::E_NO_MEMORY)
      std::cout << "E_NO_MEMORY" << std::endl;
    else if (value == OAHashTableException::E_ITEM_NOT_FOUND)
      std::cout << "E_ITEM_NOT_FOUND" << std::endl;
    else
      std::cout << "Unknown error code." << std::endl;

    //cout << endl << "errno: " << e.code() << ", " << e.what() << endl << endl;
    //cout << endl << "**** Caught exception deleting non-existent item. (SimpleTest1) ****" << endl << endl;
  }
  catch (...) 
  {
    cout << endl << "**** Something bad happened inserting in " << test << endl << endl;
  }
}

/*
  Why are the hashes so different when the same function is used for
  both primary and secondary hash? e.g. TableSize is 13:

  57 % 13 = 5
  57 % 12 = 9

  The modulo can vary significantly with a difference in 1 by
  the table size.

*/
void testhash()
{
  unsigned (*hf)(const char *Key, unsigned TableSize) = PJWHash;
  unsigned size = 13;
  const char *keys[] ={ "101001", "102001", "103001", "104001"};

  for (unsigned i = 0; i < sizeof(keys) / sizeof(*keys); i++)
  {
    unsigned step1 = hf(keys[i], size);
    unsigned step2 = hf(keys[i], size - 1);

    printf("key = %s, hash1 = %i, hash2 = %i\n", keys[i], step1, step2);
  }
  
}

int main(int argc, char **argv)
{

  FillPersonRecs();

  //OAHashTable<float *>::OAHTConfig config(11, ConstantHash);
  //OAHashTable<float *> ht(config); // <----if this is changed to <float *> the program


  //testhash();

  // ***************** Linear probing and PACK tests ************************
  size_t test_num = 0;
  if (argc > 1)
    test_num = static_cast<size_t>(std::atoi(argv[1]));


  switch (test_num)
  {
    case 1:
      TestALot(&HashingFuncs[SIMPLE], &HashingFuncs[NONE]);
      break;
    case 2:
      TestSimpleGrow1();         
      break;
    case 3:
      TestSimpleDeletePresent(); 
      break;
      
    case 4:
      TestSimpleDeleteMissing(); 
      break;
      
    case 5:
      TestSimpleDispose();       
      break;
      
    case 6:
      TestInsertConstant();      
      break;
      
  // ****************** Double hashing and MARK tests ***********************
    case 7:
      TestALot(&HashingFuncs[PJW], &HashingFuncs[RS]);
      break;
      
    case 8:
      TestSimpleInsert1();       
      break;
      
    case 9:
      TestSimpleInsert2();       
      break;
      
    case 10:
      TestSimpleGrow2();         
      break;
      
    case 11:
      TestSimpleMarkPack(&HashingFuncs[SIMPLE], &HashingFuncs[NONE], MARK);
      break;
      
    case 12:
      TestSimpleMarkPack(&HashingFuncs[SIMPLE], &HashingFuncs[PJW], MARK);
      break;
      
    case 13:
      TestDoubleHashing(&HashingFuncs[PJW], &HashingFuncs[SIMPLE]);
      break;

    default:
      TestALot(&HashingFuncs[SIMPLE], &HashingFuncs[NONE]);
      TestSimpleGrow1();         
      TestSimpleDeletePresent(); 
      TestSimpleDeleteMissing(); 
      TestSimpleDispose();       
      TestInsertConstant();      

      TestALot(&HashingFuncs[PJW], &HashingFuncs[RS]);
      TestSimpleInsert1();       
      TestSimpleInsert2();       
      TestSimpleGrow2();         
      TestSimpleMarkPack(&HashingFuncs[SIMPLE], &HashingFuncs[NONE], MARK);
      TestSimpleMarkPack(&HashingFuncs[SIMPLE], &HashingFuncs[PJW], MARK);
      TestDoubleHashing(&HashingFuncs[PJW], &HashingFuncs[SIMPLE]);
      break;
  }

  FreePersonRecs();
  return 0;
}

