//
//  PegasusIndigo.cpp
//  Pegasus Indigo Filter Wheel
//
//  Created by Rodolphe Pineau on 30/5/2022.
//  Copyright © 2022 RTI-Zone. All rights reserved.
//

#include "PegasusIndigo.h"

CPegasusIndigo::CPegasusIndigo()
{
    m_bIsConnected = false;
    m_nCurentFilterSlot = -1;
    m_nTargetFilterSlot = 0;

#ifdef PLUGIN_DEBUG
#if defined(SB_WIN_BUILD)
    m_sLogfilePath = getenv("HOMEDRIVE");
    m_sLogfilePath += getenv("HOMEPATH");
    m_sLogfilePath += "\\PegasusIndigoLog.txt";
#elif defined(SB_LINUX_BUILD)
    m_sLogfilePath = getenv("HOME");
    m_sLogfilePath += "/PegasusIndigoLog.txt";
#elif defined(SB_MAC_BUILD)
    m_sLogfilePath = getenv("HOME");
    m_sLogfilePath += "/PegasusIndigoLog.txt";
#endif
    m_sLogFile.open(m_sLogfilePath, std::ios::out |std::ios::trunc);
#endif

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [CPegasusIndigo] Version " << std::fixed << std::setprecision(2) << PLUGIN_VERSION << " build " << __DATE__ << " " << __TIME__ << std::endl;
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [CPegasusIndigo] Constructor Called." << std::endl;
    m_sLogFile.flush();
#endif

}

CPegasusIndigo::~CPegasusIndigo()
{

}

int CPegasusIndigo::Connect(const char *szPort)
{
    int nErr = PLUGIN_OK;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Connect Called." << std::endl;
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Trying to connect to port " << szPort<< std::endl;
    m_sLogFile.flush();
#endif

    // 9600 8N1
    if(m_pSerx->open(szPort, 9600, SerXInterface::B_NOPARITY, "-DTR_CONTROL 1") == 0)
        m_bIsConnected = true;
    else
        m_bIsConnected = false;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;


#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Connected." << std::endl;
    m_sLogFile.flush();
#endif

    nErr = getStatus();
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Error Getting status : " << nErr << std::endl;
        m_sLogFile.flush();
#endif

        m_bIsConnected = false;
        m_pSerx->close();
        return ERR_DEVICENOTSUPPORTED;
    }

    // if any of this fails we're not properly connected or there is a hardware issue.
    nErr = getFirmwareVersion(m_sFirmwareVersion);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Error Getting Firmware : " << nErr << std::endl;
        m_sLogFile.flush();
#endif

        m_bIsConnected = false;
        m_pSerx->close();
        return FIRMWARE_NOT_SUPPORTED;
    }

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Connect] Connected" << std::endl;
    m_sLogFile.flush();
#endif

    nErr = getCurrentSlot(m_nCurentFilterSlot);
    
    return nErr;
}



void CPegasusIndigo::Disconnect()
{

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [Disconnect] Called" << std::endl;
    m_sLogFile.flush();
#endif

    if(m_bIsConnected) {
        m_pSerx->purgeTxRx();
        m_pSerx->close();
    }
    m_bIsConnected = false;
}


#pragma mark - communication functions

int CPegasusIndigo::sendCommand(const std::string sCmd, std::string &sResp, int nTimeout)
{
    int nErr = PLUGIN_OK;
    unsigned long  ulBytesWrite;

    m_pSerx->purgeTxRx();
    sResp.clear();

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [sendCommand] sending "<< sCmd<< std::endl;
    m_sLogFile.flush();
#endif

    nErr = m_pSerx->writeFile((void *)sCmd.c_str(), sCmd.size(), ulBytesWrite);
    m_pSerx->flushTx();
    if(nErr)
        return nErr;

    // read response
    if(nTimeout == 0) // no response expected
        return nErr;

    nErr = readResponse(sResp, nTimeout);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [sendCommand] ***** ERROR READING RESPONSE **** error = " << nErr << " , response : " << sResp << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [sendCommand] response " << sResp <<  std::endl;
    m_sLogFile.flush();
#endif

    return nErr;
}


int CPegasusIndigo::readResponse(std::string &sResp, int nTimeout)
{
    int nErr = PLUGIN_OK;
    char pszBuf[SERIAL_BUFFER_SIZE];
    unsigned long ulBytesRead = 0;
    unsigned long ulTotalBytesRead = 0;
    char *pszBufPtr;
    int nBytesWaiting = 0 ;
    int nbTimeouts = 0;

    memset(pszBuf, 0, SERIAL_BUFFER_SIZE);
    pszBufPtr = pszBuf;

    do {
        nErr = m_pSerx->bytesWaitingRx(nBytesWaiting);
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 3
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] nBytesWaiting      : " << nBytesWaiting << std::endl;
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] nBytesWaiting nErr : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        if(!nBytesWaiting) {
            nbTimeouts += MAX_READ_WAIT_TIMEOUT;
            if(nbTimeouts >= nTimeout) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 3
                m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] bytesWaitingRx timeout, no data for " << nbTimeouts << " ms"<< std::endl;
                m_sLogFile.flush();
#endif
                nErr = PLUGIN_COMMAND_TIMEOUT;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_READ_WAIT_TIMEOUT));
            continue;
        }
        nbTimeouts = 0;
        if(ulTotalBytesRead + nBytesWaiting <= SERIAL_BUFFER_SIZE)
            nErr = m_pSerx->readFile(pszBufPtr, nBytesWaiting, ulBytesRead, nTimeout);
        else {
            nErr = ERR_RXTIMEOUT;
            break; // buffer is full.. there is a problem !!
        }
        if(nErr) {
#if defined PLUGIN_DEBUG
            m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] readFile error : " << nErr << std::endl;
            m_sLogFile.flush();
#endif
            return nErr;
        }

        if (ulBytesRead != nBytesWaiting) { // timeout
#if defined PLUGIN_DEBUG
            m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] rreadFile Timeout Error." << std::endl;
            m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] readFile nBytesWaiting : " << nBytesWaiting << std::endl;
            m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] readFile ulBytesRead   : " << ulBytesRead << std::endl;
            m_sLogFile.flush();
#endif
        }

        ulTotalBytesRead += ulBytesRead;
        pszBufPtr+=ulBytesRead;
    }  while (ulTotalBytesRead < SERIAL_BUFFER_SIZE  && *(pszBufPtr-1) != '\n');

    if(!ulTotalBytesRead)
        nErr = PLUGIN_COMMAND_TIMEOUT; // we didn't get an answer.. so timeout

    sResp.assign(pszBuf);
    sResp = rtrim(sResp, "\n\r");
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 3
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [readResponse] sResp : " << sResp << std::endl;
    m_sLogFile.flush();
#endif

    return nErr;
}


#pragma mark - Filter Wheel info commands
int CPegasusIndigo::getStatus()
{
    int nErr = PLUGIN_OK;
    std::string sResp;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    // OK_UPB or OK_PPB
    nErr = sendCommand("W#\n", sResp);
    if(nErr)
        return nErr;
    if(sResp.find("FW_OK") ==-1) {
        nErr = PLUGIN_COMMAND_FAILED;
    }
    return nErr;
}


int CPegasusIndigo::getFirmwareVersion(std::string &sVersion)
{
    int nErr = 0;
    std::string sResp;
    std::vector<std::string> vFieldsData;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [getFirmwareVersion] Called" << std::endl;
    m_sLogFile.flush();
#endif

    if(!m_bIsConnected)
        return PLUGIN_NOT_CONNECTED;

    nErr = sendCommand("WV\n", sResp);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [getFirmwareVersion] Error Getting response from sendCommand : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }

    nErr = parseFields(sResp, vFieldsData, ':');
    if(nErr)
        return ERR_CMDFAILED;

    if(vFieldsData.size()>1)
        sVersion = vFieldsData[1];
    else
        sVersion = "Unknown";

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [getFirmwareVersion] Firmware : " << sVersion << std::endl;
    m_sLogFile.flush();
#endif

    return nErr;
}


#pragma mark - Filter Wheel move commands

int CPegasusIndigo::moveToFilterIndex(int nTargetPosition)
{
    int nErr = 0;
    std::stringstream ssTmp;
    std::string sResp;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] Moving to filter  : " << nTargetPosition << std::endl;
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] m_nCurentFilterSlot      : " << m_nCurentFilterSlot << std::endl;
    m_sLogFile.flush();
#endif

    ssTmp << "WM:" << nTargetPosition << "\n";
    nErr = sendCommand(ssTmp.str(), sResp);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] Error Getting response from sendCommand : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }
    m_nTargetFilterSlot = nTargetPosition;

    return nErr;
}

int CPegasusIndigo::isMoveToComplete(bool &bComplete)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
    std::vector<std::string> vFieldsData;
    int nFilterSlot;

    bComplete = false;

    if(m_nCurentFilterSlot == m_nTargetFilterSlot) {
        bComplete = true;
        return nErr;
    }

    
    nErr = sendCommand("WR\n", sResp);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [isMoveToComplete] Error Getting response from sendCommand : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }


    nErr = parseFields(sResp, vFieldsData, ':');
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [isMoveToComplete] Errorparsing response : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }

    if(vFieldsData.size()>1 && vFieldsData[1] == "0")
        bComplete = true;
    else
        bComplete = false;

    // check that we are on the right slot
    nErr = getCurrentSlot(nFilterSlot);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [isMoveToComplete] Error Getting current slot : " << nErr << std::endl;
        m_sLogFile.flush();
#endif

    }

    if(nFilterSlot == m_nTargetFilterSlot) {
        bComplete = true;
        m_nCurentFilterSlot = nFilterSlot;
    }

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
    m_sLogFile << "["<<getTimeStamp()<<"]"<< " [isMoveToComplete] bComplete : " << (bComplete?"Yes":"No") << std::endl;
    m_sLogFile.flush();
#endif

    return nErr;
}


#pragma mark - filters and device params functions
int CPegasusIndigo::getFilterCount(int &nCount)
{
    nCount = 7;
    return PLUGIN_OK;
}


int CPegasusIndigo::getCurrentSlot(int &nSlot)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
    std::vector<std::string> vFieldsData;


    nErr = sendCommand("WF\n", sResp);
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] Error Getting response from sendCommand : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }


    nErr = parseFields(sResp, vFieldsData, ':');
    if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] Error parsing response : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        return nErr;
    }

    if(vFieldsData.size()>1) {
        nSlot = std::stoi(vFieldsData[1]);
    }
    else {
        nErr = PLUGIN_COMMAND_FAILED;
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
        m_sLogFile << "["<<getTimeStamp()<<"]"<< " [moveToFilterIndex] Error getting current slot : " << nErr << std::endl;
        m_sLogFile.flush();
#endif
        nSlot = 0;
    }
    return nErr;
}

int CPegasusIndigo::parseFields(const std::string szIn, std::vector<std::string> &svFields, char cSeparator)
{
    int nErr = PLUGIN_OK;
    std::string sSegment;
    std::stringstream ssTmp(szIn);

    svFields.clear();
    // split the string into vector elements
    while(std::getline(ssTmp, sSegment, cSeparator))
    {
        svFields.push_back(sSegment);
    }

    if(svFields.size()==0) {
        nErr = ERR_PARSE;
    }
    return nErr;
}

std::string& CPegasusIndigo::trim(std::string &str, const std::string& filter )
{
    return ltrim(rtrim(str, filter), filter);
}

std::string& CPegasusIndigo::ltrim(std::string& str, const std::string& filter)
{
    str.erase(0, str.find_first_not_of(filter));
    return str;
}

std::string& CPegasusIndigo::rtrim(std::string& str, const std::string& filter)
{
    str.erase(str.find_last_not_of(filter) + 1);
    return str;
}


#ifdef PLUGIN_DEBUG
const std::string CPegasusIndigo::getTimeStamp()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}
#endif
