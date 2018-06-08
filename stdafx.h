
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#define _SZSOFTWARE_

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include "resource.h"

#include <afxdisp.h>        // MFC Automation classes
#include <stdio.h>
#include <tchar.h>
#include "targetver.h"
#include <minmax.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

#include <Windows.h>

#include <TLHELP32.H> 


#include"cxcore.h"
#include "highgui.h"
#include "cv.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#include "lucy.h"
#include "CvvImage.h"
//#if _DEBUG
//#pragma comment(lib, "cv210d.lib")
//#pragma comment(lib, "cxcore210d.lib")
//#pragma comment(lib, "highgui210d.lib")
//#else
//#pragma comment(lib, "cv210.lib")
//#pragma comment(lib, "cxcore210.lib")
//#pragma comment(lib, "highgui210.lib")
//#endif

//#pragma comment(lib, "cv210.lib")
//#pragma comment(lib, "cxcore210.lib")
//#pragma comment(lib, "highgui210.lib")
//#pragma comment(lib, "cv210d.lib")
//#pragma comment(lib, "cxcore210d.lib")
//#pragma comment(lib, "highgui210d.lib")



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxdhtml.h>









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


// add xukun 20130522
// txt translator
#define TRANS_XOR_CHAR	8
//

// add xukun 20130625
// error report
#include <list>
#include "pdflib.hpp"
#include "dog_api.h"
#include "dog_vcode.h"
#pragma comment(lib, "PDFLib.lib")
#pragma comment(lib, "dog_windows_3150259.lib")
using namespace std;

// add xukun 20131029
// XK_DEBUG:1=debug,0=no debug
const int XK_DEBUG = 0;
const int XK_DEBUG_STANDARD = 0;
// 是否是加入flag的新版本，实验室pumpcontrolv2对应的版本
const int NEW_FLAG_VERSION = 1;
const CString oldPumpControlPath = _T("SerialModule\\PumpControl.exe");
const CString newPumpControlPath = _T("D:\\pumpControl\\PumpControl.exe");
const string oldPumpControlExcName = "PumpControl.exe";
const string newPumpControlExcName = "PumpControl.exe";
// add xukun 20131130
// Relative path
const string rPath = "D:\\test";
const CString rCPath = _T("D:\\test");
// mod xukun 20131226
const string rPath_xk = "D:\\xukun\\apart-job\\enviroment\\testing";
const CString rCPath_xk = _T("D:\\xukun\\apart-job\\enviroment\\testing");

#include "Log.h"

inline void logRecordForFuncError(string funcName)
{
	CLog::LogRecord(funcName);
}
inline void logRecordForFuncError(CString funcName)
{
	CLog::LogRecord(funcName);
}

//add xukun 20141025
const int MIN_DATA_FILE_SIZE = 40000;
const DWORD MAX_SPY_INTERVAL = 2400000;
