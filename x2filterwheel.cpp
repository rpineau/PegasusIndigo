#include "x2filterwheel.h"


X2FilterWheel::X2FilterWheel(const char* pszDriverSelection,
				const int& nInstanceIndex,
				SerXInterface					* pSerX, 
				TheSkyXFacadeForDriversInterface	* pTheSkyX, 
				SleeperInterface					* pSleeper,
				BasicIniUtilInterface			* pIniUtil,
				LoggerInterface					* pLogger,
				MutexInterface					* pIOMutex,
				TickCountInterface				* pTickCount)
{
	m_nPrivateMulitInstanceIndex	= nInstanceIndex;
	m_pSerX							= pSerX;		
	m_pIniUtil						= pIniUtil;
	m_pIOMutex						= pIOMutex;

    m_bLinked = false;
    m_PegasusIndigo.SetSerxPointer(pSerX);

}

X2FilterWheel::~X2FilterWheel()
{
	if (m_pSerX)
		delete m_pSerX;
	if (m_pIniUtil)
		delete m_pIniUtil;
	if (m_pIOMutex)
		delete m_pIOMutex;
}


int	X2FilterWheel::queryAbstraction(const char* pszName, void** ppVal)
{
	X2MutexLocker ml(GetMutex());

	*ppVal = NULL;

    if (!strcmp(pszName, SerialPortParams2Interface_Name))
        *ppVal = dynamic_cast<SerialPortParams2Interface*>(this);

    return SB_OK;
}


#pragma mark - LinkInterface

int	X2FilterWheel::establishLink(void)
{
    int nErr;
    char szPort[DRIVER_MAX_STRING];

    X2MutexLocker ml(GetMutex());
    // get serial port device name
    portNameOnToCharPtr(szPort,DRIVER_MAX_STRING);
    nErr = m_PegasusIndigo.Connect(szPort);
    if(nErr)
        m_bLinked = false;
    else
        m_bLinked = true;

    return nErr;
}

int	X2FilterWheel::terminateLink(void)
{
    X2MutexLocker ml(GetMutex());
    m_PegasusIndigo.Disconnect();
    m_bLinked = false;
    return SB_OK;
}

bool X2FilterWheel::isLinked(void) const
{
    X2FilterWheel* pMe = (X2FilterWheel*)this;
    X2MutexLocker ml(pMe->GetMutex());
    return pMe->m_bLinked;
}

bool X2FilterWheel::isEstablishLinkAbortable(void) const	{

    return false;
}


#pragma mark - AbstractDriverInfo

void	X2FilterWheel::driverInfoDetailedInfo(BasicStringInterface& str) const
{
    str = "X2 Pegasus Astro Indigo Filter Wheel Plug In by Rodolphe Pineau";
}

double	X2FilterWheel::driverInfoVersion(void) const
{
	return PLUGIN_VERSION;
}

void X2FilterWheel::deviceInfoNameShort(BasicStringInterface& str) const
{
	str = "X2 Pegasus Astro Indigo Filter Wheel ";
}

void X2FilterWheel::deviceInfoNameLong(BasicStringInterface& str) const
{
	str = "X2 Pegasus Astro Indigo Filter Wheel ";

}

void X2FilterWheel::deviceInfoDetailedDescription(BasicStringInterface& str) const
{
    str = "X2 Pegasus Astro Indigo Filter Wheel Plug In";

}

void X2FilterWheel::deviceInfoFirmwareVersion(BasicStringInterface& str)
{
    if(m_bLinked) {
        X2MutexLocker ml(GetMutex());
        std::string sFirmware;
        m_PegasusIndigo.getFirmwareVersion(sFirmware);
        str = sFirmware.c_str();
    }
    else
        str = "N/A";
}
void X2FilterWheel::deviceInfoModel(BasicStringInterface& str)				
{
    if(m_bLinked) {
        X2MutexLocker ml(GetMutex());
        str = "Pegasus Astro Indigo Filter Wheel ";
    }
    else
        str = "N/A";
}

#pragma mark - FilterWheelMoveToInterface

int	X2FilterWheel::filterCount(int& nCount)
{
    int nErr = SB_OK;
    X2MutexLocker ml(GetMutex());
    nErr = m_PegasusIndigo.getFilterCount(nCount);
    if(nErr) {
        nErr = ERR_CMDFAILED;
    }
    return nErr;
}

int	X2FilterWheel::defaultFilterName(const int& nIndex, BasicStringInterface& strFilterNameOut)
{
	X2MutexLocker ml(GetMutex());
    switch(nIndex) {
        case 0:
            strFilterNameOut = "L";
            break;

        case 1:
            strFilterNameOut = "R";
            break;

        case 2:
            strFilterNameOut = "G";
            break;

        case 3:
            strFilterNameOut = "B";
            break;

        case 4:
            strFilterNameOut = "Ha";
            break;

        case 5:
            strFilterNameOut = "O-III";
            break;

        case 6:
            strFilterNameOut = "S-II";
            break;

        default:
            strFilterNameOut = "Unknown";
            break;

    }
    return SB_OK;
}

int	X2FilterWheel::startFilterWheelMoveTo(const int& nTargetPosition)
{
    int nErr = SB_OK;
    
    if(m_bLinked) {
        X2MutexLocker ml(GetMutex());
        nErr = m_PegasusIndigo.moveToFilterIndex(nTargetPosition+1);
        if(nErr)
            nErr = ERR_CMDFAILED;
    }
    return nErr;
}

int	X2FilterWheel::isCompleteFilterWheelMoveTo(bool& bComplete) const
{
    int nErr = SB_OK;

    if(m_bLinked) {
        X2FilterWheel* pMe = (X2FilterWheel*)this;
        X2MutexLocker ml(pMe->GetMutex());
        nErr = pMe->m_PegasusIndigo.isMoveToComplete(bComplete);
        if(nErr)
            nErr = ERR_CMDFAILED;
    }
    return nErr;
}

int	X2FilterWheel::endFilterWheelMoveTo(void)
{
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}

int	X2FilterWheel::abortFilterWheelMoveTo(void)
{
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}

#pragma mark -  SerialPortParams2Interface

void X2FilterWheel::portName(BasicStringInterface& str) const
{
    char szPortName[DRIVER_MAX_STRING];

    portNameOnToCharPtr(szPortName, DRIVER_MAX_STRING);

    str = szPortName;

}

void X2FilterWheel::setPortName(const char* szPort)
{
    if (m_pIniUtil)
        m_pIniUtil->writeString(PARENT_KEY, CHILD_KEY_PORTNAME, szPort);

}


void X2FilterWheel::portNameOnToCharPtr(char* pszPort, const int& nMaxSize) const
{
    if (NULL == pszPort)
        return;

    snprintf(pszPort, nMaxSize,DEF_PORT_NAME);

    if (m_pIniUtil)
        m_pIniUtil->readString(PARENT_KEY, CHILD_KEY_PORTNAME, pszPort, pszPort, nMaxSize);
    
}




