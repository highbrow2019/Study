#include<fstream>
#include<bits/stdc++.h>


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

//todo: change according to the csv file
const int MAX_SIGNALS =  2;


/** 
 * struct that simulates the signal */

typedef struct 
{
  /* time interval in msec */
  unsigned int interval;   // todo: make it "guint!"

  std::size_t valueMax; // max value (todo: these can be float!!)

  std::size_t valueMin; // min value (todo: these can be float!!)

  ESignalType valueType;
}SSimSignalProps_t;


int main()
{

SSimSignalProps_t signal_struct[MAX_SIGNALS];
  cout << "size of struct is : " << sizeof(signal_struct) << endl;
  int index = 0;
  //File Pointer
  fstream fin;
  //Open an existing file
  fin.open("sample1.csv", ios::in);
  //Read the data from the file as string vector
  vector<string>row;
  vector<string>row1;

  //Unordered map
  std::unordered_map<std::string, SSimSignalProps_t>simSigs;
  //std::unordered_map<std::string, std::string>simSigs;
  
  string line,temp,word;
  while(fin) {
    row.clear();
    row1.clear();
    /* read an entire row and store it in a string variable 'line'*/
    getline(fin,line);

    // used for breaking words
    stringstream s(line);

    // todo: read more about getline
    while(getline(s,word, ':')) {
      //cout << word << endl;
      row.push_back(word);
    }
    cout << row[0] << "<-->" << row[1] << endl;
    // todo: pass the struct instead of row[1]
    //simSigs(Odometer)=Odometer_ST3,30,50.0,0.0,FLOAT
    
    // translate the value string into a struct
    stringstream s1(row[1]);
    while(getline(s1, word, ',')){
      row1.push_back(word);
    }

    //todo: How to treat the float values for conversion?
    signal_struct[index].interval = stoi(row1[0]); //convert ASCII to int
    // (todo: these can be float!!), use stof
    signal_struct[index].valueMax = stoi(row1[1]);
    signal_struct[index].valueMin = stoi(row1[2]);
    //signal_struct[index].valueType = stoi(row1[3]);
    

    /*  cout << "interval is" << signal_struct[index].interval << endl;
    cout << "interval is" << signal_struct[index].valueMax << endl;
    cout << "interval is" << signal_struct[index].valueMin << endl; */
    if(row[0] != "\0"){
      simSigs[row[0]]=signal_struct[index];
    }
      
    index++;
    cout << "index is : " << index << endl;
    //fin >> temp;
  } 
  for( auto i:simSigs)
    cout << i.first << " " <<i.second.interval << ".." << i.second.valueMax << ".." << i.second.valueMin <<endl; 

  cout << signal_struct[2].valueMax << endl;
  //todo: Do we need dynamic memory allocation?
  return 0;
}

