#include <bits/stdc++.h>

using namespace std;

enum ESignalType
  {
   BOOLEAN=0,
   UINT8,
   UINT16,
   UINT32,
   ARRAY_UINT8,
   ARRAY_UINT16,

   FLOAT,
   DOUBLE,

   LAST  // signal type is not known
  };

typedef struct 
{
  unsigned int interval;   // todo: make it "guint!"

  std::size_t valueMax; // max value (todo: these can be float!!)

  std::size_t valueMin; // min value (todo: these can be float!!)

  string valueType;
}SSimSignalProps_t;

int main()
{
  fstream fin;
  SSimSignalProps_t sstruct;
  fin.open("sample1.csv", ios::in);

  //Unordered map
  std::unorderedmap<std::string, SSimSignalProps_t>simSigs;
  
  string temp, S, T, X;
  int Y=0;

  /* Ignore the header in the csv file */
  getline(fin, temp);
  getline(fin, temp);
  
  while(fin >> T)
    {
      /* Read the signal name */
      // getline(fin, T, ':');
      
      // cout << T << endl;

      /* Populate the structure with signal properties from the csv file */
      fin >> sstruct.interval >> sstruct.valueMax >> sstruct.valueMin >> X;

      // cout << sstruct.interval << ":" << sstruct.valueMax << ":" << sstruct.valueMin << ":" << X << endl;

      simSigs[T]=sstruct;
    }
  for (auto i:simSigs)
    cout<< i.first << " " <<i.second.interval << ".." << i.second.valueMax << ".." << i.second.valueMin <<endl;
  return 0;
}
