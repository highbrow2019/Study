enum class ESignalType: unsigned char
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



/** 
 * struct that simulates the signal */

struct SSimSignalProps
{
  /* time interval in msec */
  guint interval;

  std::size_t valueMax; // max value

  std::size_t valueMin; // min value

  ESignalType valueType
};

/**
 * @brief simSigs key value pairs of signal names (as keys) and signal properties
 * as parsed from the .csv file for the test application
 */
std::unordered_map<std::string,SSimSignalProps>simSigs

  
  

  
