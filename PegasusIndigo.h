//
//  PegasusIndigo.h
//  Pegasus Indigo Filter Wheel
//
//  Created by Rodolphe Pineau on 30/5/2022.
//  Copyright © 2022 RTI-Zone. All rights reserved.
//

#ifndef PegasusIndigo_h
#define PegasusIndigo_h
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <time.h>
#ifdef SB_MAC_BUILD
#include <unistd.h>
#endif

// C++ includes
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <algorithm>


#include "../../licensedinterfaces/sberrorx.h"
#include "../../licensedinterfaces/serxinterface.h"

#define PLUGIN_DEBUG 2
#define PLUGIN_VERSION      1.0


#define SERIAL_BUFFER_SIZE 1024
#define MAX_TIMEOUT 1000
#define ERR_PARSE   1

#define MAX_READ_WAIT_TIMEOUT 25
#define NB_RX_WAIT 10

enum PegasusIndigoFilterWheelErrors {PLUGIN_OK=0, PLUGIN_NOT_CONNECTED, PLUGIN_CANT_CONNECT, PLUGIN_BAD_CMD_RESPONSE, PLUGIN_COMMAND_FAILED, PLUGIN_COMMAND_TIMEOUT};

class CPegasusIndigo
{
public:
    CPegasusIndigo();
    ~CPegasusIndigo();

    int             Connect(const char *szPort);
    void            Disconnect(void);
    bool            IsConnected(void) { return m_bIsConnected; };

    void            SetSerxPointer(SerXInterface *p) { m_pSerx = p; };

    // filter wheel communication
    int             sendCommand(const std::string sCmd, std::string &sResp, int nTimeout = MAX_TIMEOUT);
    int             readResponse(std::string &sResult, int nTimeout = MAX_TIMEOUT);

    // Filter Wheel commands
    int             getFirmwareVersion(std::string &sVersion);
    int             getStatus();
    
    int             moveToFilterIndex(int nTargetPosition);
    int             isMoveToComplete(bool &bComplete);

    int             getFilterCount(int &nCount);
    int             getCurrentSlot(int &nSlot);

protected:
    SerXInterface   *m_pSerx;

    bool            m_bIsConnected;

    std::string     m_sFirmwareVersion;


    int             m_nCurentFilterSlot;
    int             m_nTargetFilterSlot;
    
    int             parseFields(const std::string szIn, std::vector<std::string> &svFields, char cSeparator);
    std::string&    trim(std::string &str, const std::string &filter );
    std::string&    ltrim(std::string &str, const std::string &filter);
    std::string&    rtrim(std::string &str, const std::string &filter);

#ifdef PLUGIN_DEBUG
    const std::string getTimeStamp();
    std::ofstream m_sLogFile;
    std::string m_sLogfilePath;
#endif

};
#endif /*PegasusIndigo_h */
