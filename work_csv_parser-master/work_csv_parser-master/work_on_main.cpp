/**
* Copyright (C) 2018 Continental AG and subsidiaries
*/

/**
* @file
*       main.cpp
* @brief
*       Data Collector 2+ library test
* @par
*       Project: Daimler RAMSES
* @par
*       SW Package: DC2+
*
* @author
*       Vergil Iliev , Jaime Pena
*/
#include "../include/TestApp.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <glib.h>
#include <csignal>
#include <list>
#include <memory>
#include <dlfcn.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include <chrono>

extern "C"
{
#include <tpsys.h>                       // for logging
#include <persistence_client_library.h>  // for persistency
}

constexpr static const gchar* DEFAULT_LIB_LOCATION{"/usr/lib/DRT/libdc2plus-sdb.so"};
constexpr static const gchar* SERVICE_NAME{"dc2plus-test"};

using namespace dc2plus;

[[noreturn]] static void SignalHandler(int);

//callbacks definition
void sendSignal(const std::vector<SSignalInfo>&);
bool getSignal(const std::string&, SSignalInfo&);
void subscribeSignal(const std::string&);
void unsubscribeSignal(const std::string&);
void keepAwake(bool);
static int pushData(void*);
static int pushDummyData(void*);
static void registerPlatform();
static void deregisterPlatform();

static GMainLoop* mainLoop = nullptr;
static SimSignalsT simSigs;
static IDC2PlusCommunication* dc2pCommInst = nullptr;
static std::vector<std::string*> heapAllocs; // keep track of data ptrs used by glib triggers

typedef void* (*pDC2ReleaseInstance)();
typedef dc2plus::IDC2PlusCommunication* (*pDC2GetInstance)();
static pDC2GetInstance m_pDC2GetInstance = nullptr;
static pDC2ReleaseInstance m_pDC2ReleaseInstance = nullptr;

void SignalHandler(int signal)
{
    tpLOG_warn("%d signal notification...", signal);

    // remove and free all elements
    heapAllocs.erase(std::remove_if(std::begin(heapAllocs),
                  std::end(heapAllocs),
                  [](std::string* el)
    {
        bool retVal{false};
        if(el != nullptr)
        {
            delete el;
            *el = nullptr;
            retVal = true;
        } else {
            retVal = true;
        }
        return retVal;
    }) , heapAllocs.end());

    // simulate LCM shutdown event when e.g. test application gets the kill signal
    if (dc2pCommInst != nullptr)
    {
        dc2pCommInst->setSuspendedMode(dc2plus::SuspendedMode::E_SM_SHUTDOWN);
    }

    if (mainLoop != nullptr)
    {
        if (g_main_loop_is_running(mainLoop))
        {
            tpLOG_info("Exit glib main loop");
            g_main_loop_quit(mainLoop);
        }
        g_main_loop_unref(mainLoop);
        mainLoop = nullptr;
    }

    if (dc2pCommInst != nullptr)
    {
        dc2pCommInst->deInitialize();
        m_pDC2ReleaseInstance();
    }

    deregisterPlatform();
    tpLOG_close();
    exit(EXIT_SUCCESS);
}

SimSignalsT saiFunc(const gchar *confPath)
{
    SimSignalsT conf;

    // for each line in a file
    {
        std::pair<std::string, std::pair<guint, SSimSignalProps>> entry{};

        // fill in sigName:
        //entry.first = // parsed string
//        entry.second.second.interval =
//        entry.second.second.valueMin =
//        entry.second.second.valueMax
        if(entry.second.second.valueMax < entry.second.second.valueMin)
        {
            entry.second.second.valueMax = entry.second.second.valueMin;
        }

        conf.insert(entry);
    }

    return conf;
}

static gchar * parseCmdLineOpts(int argc, char * argv[])
{
    gchar *res = nullptr;

    gboolean testMode{true};
    gchar *path = nullptr;
    gchar *confPath = nullptr;
    GOptionEntry mainEntries[] =
    {
        { "path", 'p', 0 , G_OPTION_ARG_STRING, &path , "Select path for libdc2plus-sdb.so, defalut /usr/lib/DRT/", nullptr },
        { "test", 't', 0, G_OPTION_ARG_NONE, &testMode, "Run default test mode", nullptr }, // depricated
        { "config", 'c', 0, G_OPTION_ARG_STRING, &path , "Select path for *.csv, defalut /tmp/", nullptr },
        { nullptr, 0 , 0, G_OPTION_ARG_NONE, nullptr, nullptr,nullptr },
    };

    GError *error = nullptr;
    GOptionContext *options;

    options = g_option_context_new ("- test tree model performance");
    g_option_context_add_main_entries (options, mainEntries, nullptr);
    if (!g_option_context_parse (options, &argc, &argv, &error))
    {
        g_print ("option parsing failed: %s\n", error->message);
        exit (EXIT_FAILURE);
    }

    const gchar* sufix{"/"};
    if(!g_str_has_suffix(path,  sufix))
    {
        res = g_strconcat(path, sufix, nullptr);
    }

    if(confPath != nullptr)
    {
        simSigs = saiFunc(confPath);
    }

    return res;
}

template <typename T>
std::string varToData(T& value)
{
    std::string strValue;
    strValue.resize(sizeof(T));
    std::copy(
       reinterpret_cast<const char*>(&value),
       reinterpret_cast<const char*>(&value) + sizeof (T),
       &strValue[0]
    );

    return strValue;
}

static SNotificationCallbacks callbacks =
{
    &sendSignal,
    &getSignal,
    &subscribeSignal,
    &unsubscribeSignal,
    &keepAwake
};


void sendSignal (const std::vector<SSignalInfo>& sigInfList)
{
    std::for_each(std::begin(sigInfList),
                  std::end(sigInfList),
                  [](const SSignalInfo& el)
    {
        tpLOG_info("sendSignal() not implemented yet for signal %s",
                   el.m_signalName.c_str());
    });
}

bool getSignal(const std::string& sig, SSignalInfo& sigInf)
{
    bool retVal{false};
    sigInf.m_timeStamp = static_cast<unsigned long long>
            (std::chrono::system_clock::now().time_since_epoch().count());
    sigInf.m_signalName = sig;
    sigInf.m_signalStatus = ESignalStatus::AVAILABLE;

    if (simSigs.count(sig) != 0)
    {
        const SSimSignalProps& entry = simSigs.at(sig).second;
        sigInf.m_signalType = entry.valueType;
        const auto randomVal = std::rand() % (entry.valueMax + 1 - entry.valueMin) + entry.valueMin;
        switch (entry.valueType)
        {
        case ESignalType::BOOLEAN:
        {
            bool simVar = static_cast<bool>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::UINT8:
        {
            uint8_t simVar = static_cast<uint8_t>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::UINT16:
        {
            uint16_t simVar = static_cast<uint16_t>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::UINT32:
        {
            uint32_t simVar = static_cast<uint32_t>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::UINT64:
        {
            uint64_t simVar = static_cast<uint64_t>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::FLOAT:
        {
            float simVar = static_cast<float>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }
        case ESignalType::DOUBLE:
        {
            double simVar = static_cast<double>(randomVal);
            sigInf.m_signalValue = varToData(simVar);
            break;
        }

        default:
            tpLOG_info("unsupported type %d during value generation", static_cast<int>(entry.valueType));
            break;
        }
        retVal = true;
    } else {
        sigInf.m_signalValue = std::string{"Test"};
    }

    return retVal;
}

void subscribeSignal(const std::string& signal)
{
    tpLOG_info("Subscribe called for signal %s", signal.c_str());
    if (dc2pCommInst != nullptr)
    {
        if(simSigs.count(signal) != 0)
        {
            simSigs.at(signal).first = g_timeout_add(simSigs.at(signal).second.interval,
                                                     &pushData,
                                                     *heapAllocs.insert(std::end(heapAllocs),
                                                                        new std::string{signal}));
        } else {
            g_timeout_add(100, &pushDummyData, *heapAllocs.insert(std::end(heapAllocs),
                                                                  new std::string{signal}));
        }
    }
}

void unsubscribeSignal(const std::string& signal)
{
    tpLOG_error("Unsubscribe called for signal %s", signal.c_str());

    // remove and free unneded element
    heapAllocs.erase(std::remove_if(std::begin(heapAllocs),
                  std::end(heapAllocs),
                  [&signal](std::string* el)
    {
        bool retVal{false};
        if(el != nullptr)
        {
            if(*el == signal)
            {
                delete el;
                *el = nullptr;
                retVal = true;
            }
        } else {
            retVal = true;
        }
        return retVal;
    }) , heapAllocs.end());

    if(simSigs.count(signal) != 0)
    {
        g_source_remove(simSigs.at(signal).first);
    }
}

void keepAwake(bool keepAwake)
{
    tpLOG_info("keepAwake called with value %d", keepAwake);
    return;
}

static int pushData(void* data)
{
    int retval(0);
    if ((data != nullptr) && (dc2pCommInst != nullptr))
    {
        std::string* sigName = static_cast<std::string*>(data);

        SSignalInfo simSig{};

        if(getSignal(*sigName, simSig))
        {
            retval = 1;
            dc2pCommInst->onSignalUpdate(simSig);
        } else {
            delete sigName;
            *sigName = nullptr;
        }
    }
    return retval;
}

static int pushDummyData(void* data)
{
    int retval(0);
    if ((data != nullptr) && (dc2pCommInst != nullptr))
    {
        std::string* sigName = static_cast<std::string*>(data);

        SSignalInfo simSig{};

        getSignal(*sigName, simSig);
        retval = 1;
        dc2pCommInst->onSignalUpdate(simSig);
    }
    return retval;
}

void registerPlatform()
{
    //tpsys init
    tpSYS_ReturnCode_t retSystem = E_SYS_IPC_RET_LAST;

    retSystem = tpSYS_initIPC(E_SYS_IPC_DBUS, nullptr, nullptr);
    if(retSystem == E_SYS_IPC_RET_FAILURE)
    {
        tpLOG_error("Error initialization to system service");
    }

    //persistency
    tpPCL_Error_t pcl_retval(E_PCL_ERROR_GENERIC);

    pcl_retval = tpPCL_init(SERVICE_NAME, nullptr, true);
    if (E_PCL_ERROR_NONE == pcl_retval)
    {
        tpLOG_debug("Persistency Connector Initialization Success!");
    }
    else
    {
        tpLOG_error("tpPCL_init failed: %d", pcl_retval);
    }
}

void deregisterPlatform()
{
    //persistency
    tpPCL_Error_t pcl_retval(E_PCL_ERROR_GENERIC);

    pcl_retval = tpPCL_deinit();
    if (E_PCL_ERROR_NONE == pcl_retval)
    {
        tpLOG_debug("Persistency Connector Deinitialization Success!");
    }
    else
    {
        tpLOG_error("tpPCL_deinit failed: %d", pcl_retval);
    }

    //tpSys_deinit
    tpSYS_deinitIPC();
}

int main(int argc, char * argv[])
{

    tpLOG_open("DC2+ lib test", TP_LOG_DOMAIN_PRODUCT);

    //handle service signals
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    //register signal to have also the stack printed
    std::signal(SIGKILL, tpSYS_stackTraceSignalHandler);
    std::signal(SIGABRT, tpSYS_stackTraceSignalHandler);
    std::signal(SIGSEGV, tpSYS_stackTraceSignalHandler);

    mainLoop = g_main_loop_new(nullptr, FALSE);

    if (mainLoop != nullptr)
    {
        tpLOG_debug("Successfully initialized GMainLoop.");

        //register platform functions from main
        registerPlatform();

        gchar * pathToLib = parseCmdLineOpts(argc, argv);
        if (pathToLib == nullptr)
        {
            pathToLib = g_strdup(DEFAULT_LIB_LOCATION);
        }

        void* m_libHandle = nullptr;
        m_libHandle = dlopen(pathToLib, RTLD_NOW | RTLD_GLOBAL);
        if (!m_libHandle)
        {
            std::cout << "Libray file not found" << std::endl;
            tpLOG_debug("DC2: m_libHandle is NULL, dlopen() error = %s", dlerror());
        } else {
            const char* const DC2_get_instance = "getInstance";
            const char* const DC2_release_instance = "releaseInstance";

            char *error = nullptr;
            m_pDC2GetInstance = reinterpret_cast<pDC2GetInstance>(dlsym(m_libHandle, DC2_get_instance));
            if ((error = dlerror()) != nullptr)
            {
                tpLOG_debug("DC2: Error getting m_pDC2GetInstance %s", error);
            }

            m_pDC2ReleaseInstance = reinterpret_cast<pDC2ReleaseInstance>(dlsym(m_libHandle, DC2_release_instance));
            if ((error = dlerror()) != nullptr)
            {
                tpLOG_debug("DC2: Error getting m_pDC2ReleaseInstance %s", error);
            }

            if(m_pDC2GetInstance != nullptr)
            {
                tpLOG_debug("DC2: library loaded as plugin. Getting Instance ...");
                dc2pCommInst = m_pDC2GetInstance();
                tpLOG_debug("DC2: library Instance received!");
            }
        }
        g_free(pathToLib);

        if(dc2pCommInst)
        {
            dc2pCommInst->initialize(callbacks);
            tpLOG_debug("Init success. Starting GMainLoop. !!!");
            g_main_loop_run(mainLoop);

        } else {
            tpLOG_debug("dc2pCommInst is null");
        }
        //deregister platform after shutdown
        deregisterPlatform();
    }else{
        tpLOG_debug("Creation of GMainLoop failed.");
    }

    tpLOG_close();
    return EXIT_FAILURE;
}
