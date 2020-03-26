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

  ESignalType valueType;
}SSimSignalProps_t;

typedef std::unordered_map<std::string, SSimSignalProps_t>simSignalsT;

//declare a function to return unordered map
simSignalsT csv_parser();


int main()
{
  csv_parser();
  return 0;
}

simSignalsT csv_parser()
{
  fstream fin;

  //do not use the struct directly rather use the unordered map
  SSimSignalProps_t sstruct_temp;
  
  fin.open("sample1.csv", ios::in);

  //Unordered map
  //std::unordered_map<std::string, SSimSignalProps_t>simSigs;
  simSignalsT conf;
  
  string temp, S, T, X;
  int Y=0;

  /* Ignore the header in the csv file */
  getline(fin, temp);
  getline(fin, temp);
  
  cout<<"hellooooooo";
  while(fin >> T)
    {
      /* Read the signal name */
      // getline(fin, T, ':');
      
      // cout << T << endl;

      /* Populate the structure with signal properties from the csv file */
      fin >> sstruct_temp.interval >> sstruct_temp.valueMax >> sstruct_temp.valueMin >> X;

      // cout << conf.second.interval << ":" << conf.second.valueMax << ":" << conf.second.valueMin << ":" << X << endl;

      conf[T]=sstruct_temp;
    }
  //debug output...todo: REMOVE the debug output
  for (auto i:conf)
    cout<< i.first << " " <<i.second.interval << ".." << i.second.valueMax << ".." << i.second.valueMin <<endl;

  return conf;
}


