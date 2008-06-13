///////////////////////////////////////////////////////////////////////////////
//	File:		SocketManager.h
//
//	Author:		Ernest Laurentin
//	E-mail:		elaurentin@netzero.net
//
//	Implementation of the SocketManager and associated classes.
//
//	This code may be used in compiled form in any way you desire. This
//	file may be redistributed unmodified by any means PROVIDING it is
//	not sold for profit without the authors written consent, and
//	providing that this notice and the authors name and all copyright
//	notices remains intact.
//
//	This file is provided "as is" with no expressed or implied warranty.
//	The author accepts no liability for any damage/loss of business that
//	this c++ class may cause.
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETMANAGER_H__7403BD71_338A_4531_BD91_3D7E5B505793__INCLUDED_)
#define AFX_SOCKETMANAGER_H__7403BD71_338A_4531_BD91_3D7E5B505793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <vector>
using namespace std;

#include "SocketComm.h"
#include "SysTray.h"

#define WM_UPDATE_CONNECTION	WM_APP+0x1234


typedef struct CallInfo {
    CString cid;
	CString sdate, stime;
	time_t call_time;
	bool bOutlookContact;
} CallInfo;


struct stMessageProxy
{
  SockAddrIn address;
  BYTE byData[BUFFER_SIZE];
};

class CSocketManager : public CSocketComm  
{
public:
	CSocketManager();
	virtual ~CSocketManager();

	void SetMessageWindow(CEdit* pMsgCtrl);
	void AppendMessage(LPCTSTR strText );
public:

	virtual void OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount);
	virtual void OnEvent(UINT uEvent);

protected:
	void DisplayData(const LPBYTE lpData, DWORD dwCount, const SockAddrIn& sfrom);
	CEdit* m_pMsgCtrl;

public:
	BOOL authenticated;
	CString extension;
	CString acm;
	CSysTray *trayIcon;
	CFrameWnd *mainFrame;

	map<CString,CallInfo> m_MissedCalls;
	CString calledExtension, callerID1, callerID2, channel, callDate, callTime, QueueExtension; //we store here call info for program's main thread
	BOOL m_bManualSignOut;
	
	void SendData(CString data);

	map<CString, CString> m_sipUsers;
	map<CString, CString> m_iaxUsers;
	map<CString, CString> m_sccpUsers;

	CString FindCalledExtension(CString calleeID);

private:	

	BOOL m_AppendData;
	CString m_AppendMsg;
	void InsertCallIntoDB(CString channel, BOOL bMissed);
	void FormatDateAndTime(CString &sDate, CString &sTime);

	void GetEventValues(CString eventData, map<CString, CString> &ret);	
	void StripCallerID(CString &cid1, CString &cid2);

};

#endif // !defined(AFX_SOCKETMANAGER_H__7403BD71_338A_4531_BD91_3D7E5B505793__INCLUDED_)
