///////////////////////////////////////////////////////////////////////////////
//	File:		SocketManager.cpp
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

// SocketManager.cpp: implementation of the CSocketManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <atlconv.h>

#include "SocketManager.h"
#include "MainFrm.h"
#include "PBXClient.h"

#include "md5.h"
#include "base64.h"

#include <string>
#include "CPPSQLite3.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*
const UINT EVT_CONSUCCESS = 0x0000;	// Connection established
const UINT EVT_CONFAILURE = 0x0001;	// General failure - Wait Connection failed
const UINT EVT_CONDROP	  = 0x0002;	// Connection dropped
const UINT EVT_ZEROLENGTH = 0x0003;	// Zero length message
*/


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketManager::CSocketManager()
: m_pMsgCtrl(NULL)
{
	m_AppendData = FALSE;
	m_AppendMsg = "";
	authenticated = FALSE;
	acm = APP_NAME;
	m_bManualSignOut = FALSE;	
}

CSocketManager::~CSocketManager()
{

}


void CSocketManager::DisplayData(const LPBYTE lpData, DWORD dwCount, const SockAddrIn& sfrom)
{
	CString strData = lpData;
	//memcpy(strData.GetBuffer(dwCount), A2CT((LPSTR)lpData), dwCount);
	//strData.ReleaseBuffer();
	if (!sfrom.IsNull())
	{
		LONG  uAddr = sfrom.GetIPAddr();
		BYTE* sAddr = (BYTE*) &uAddr;
		short nPort = ntohs( sfrom.GetPort() );	// show port in host format...
		CString strAddr;
		// Address is stored in network format...
		strAddr.Format(_T("%u.%u.%u.%u (%d)>"),
					(UINT)(sAddr[0]), (UINT)(sAddr[1]),
					(UINT)(sAddr[2]), (UINT)(sAddr[3]), nPort);

		strData = strAddr + strData;
	}

	AppendMessage( strData );
}


void CSocketManager::AppendMessage(LPCTSTR strText )
{
	if (NULL == m_pMsgCtrl)
		return;

	HWND hWnd = m_pMsgCtrl->GetSafeHwnd();
	DWORD dwResult = 0;

	if (m_pMsgCtrl->GetLineCount()>=5000) {
		m_pMsgCtrl->SetWindowText(_T(""));
	}

	if (SendMessageTimeout(hWnd, WM_GETTEXTLENGTH, 0, 0, SMTO_NORMAL, 1000L, &dwResult) != 0)
	{
		int nLen = (int) dwResult;
		if (SendMessageTimeout(hWnd, EM_SETSEL, nLen, nLen, SMTO_NORMAL, 1000L, &dwResult) != 0)
		{
			if (SendMessageTimeout(hWnd, EM_REPLACESEL, FALSE, (LPARAM)strText, SMTO_NORMAL, 1000L, &dwResult) != 0)
			{
			}
		}

	}
}


void CSocketManager::SetMessageWindow(CEdit* pMsgCtrl)
{
	m_pMsgCtrl = pMsgCtrl;
}


void CSocketManager::OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount)
{
	LPBYTE lpData = lpBuffer;
	SockAddrIn origAddr;
	stMessageProxy msgProxy;
	if (IsSmartAddressing())
	{
		dwCount = __min(sizeof(msgProxy), dwCount);
		memcpy(&msgProxy, lpBuffer, dwCount);
		origAddr = msgProxy.address;
		if (IsServer())
		{
			// broadcast message to all
			msgProxy.address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
			WriteComm((const LPBYTE)&msgProxy, dwCount, 0L);
		}
		dwCount -= sizeof(msgProxy.address);
		lpData = msgProxy.byData;
	}

	CString strData, eventData, callerID, callerID1, callerID2, callee, channel;
	CString callDate, callTime;	
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	
	int nIndex, nIndex2, nRet;
	int esIndex; // Event Separator: "\r\n\r\n"	
	char *recivedStr = new char[dwCount+2];
	memcpy(recivedStr, lpData, dwCount);
	recivedStr[dwCount]='\0';
	strData = recivedStr;
	delete recivedStr;
	
	if (!authenticated) {	
		if (strData.Find(_T("Message: Authentication accepted"))!=-1) {
						
			((CMainFrame*)mainFrame)->m_bSigningIn = FALSE;
			((CMainFrame*)mainFrame)->m_bCancelSignIn = FALSE;			

			authenticated = TRUE;
						
			trayIcon->SetTooltipText(CString(APP_NAME) + " - " + _("Signed In"));
			trayIcon->SetIcon(IDI_ICON_CONNECTED);
			((CMainFrame*)mainFrame)->ShowNotificationDialog(_("Signed In") + CString(" - ") + acm);			
			if (AUTH_ENABLED) {
				SendData("Action: Command\r\nCommand: sip show users\r\nActionID: OUTCALL_SIP_USERS\r\n\r\n");						
			}
		} else if (strData.Find(_T("Message: Authentication failed"))!=-1) {			
			((CMainFrame*)mainFrame)->m_bSigningIn = FALSE;
			((CMainFrame*)mainFrame)->m_bCancelSignIn = FALSE;
			trayIcon->SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
			trayIcon->SetIcon(IDI_ICON_DISCONNECTED);
			PostMessage(mainFrame->GetSafeHwnd(), STOP_COMM_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
			MessageBox(NULL, _("Authentication failed!"), APP_NAME, MB_OK | MB_ICONERROR);						
		} else if ((nIndex=strData.Find(_T("Challenge: ")))!=-1) {
			CString packetString;
			CString finalString;
			int i;
			nIndex += 11;
			nRet=strData.Find(_T("\r\n"), nIndex);
			if (nRet==-1)
				finalString = strData.Mid(nIndex);
			else
				finalString = strData.Mid(nIndex, nRet-nIndex);

			CString username = ::theApp.GetProfileString("Settings", "username", "");
			//decode password from registry
			/*std::string encoded = (LPCTSTR)(::theApp.GetProfileString("Settings", "password", ""));
			std::string decoded = base64_decode(encoded);
			
			for (i=0; i<decoded.length(); i++)
				decoded[i]-=10;*/
			CString password = ::theApp.DecodePassword(::theApp.GetProfileString("Settings", "password", ""));
			finalString += password; //CString(decoded.c_str());
			
			CString key;			
			char buf[50];
			//char *ch;

#ifdef _UNICODE
			CStringA ansi(finalString);
            MD5String(ansi.GetBuffer(), buf);
			key = buf;			
			/*ch = ::theApp.ConvertFromUnicodeToMB(finalString);
			if (ch) {
                MD5String(ch, buf);
				free(ch);
				key = buf;
			}*/
#else            			
			MD5String(finalString.GetBuffer(), buf);
			key = buf;
#endif

			packetString = "Action: Login\r\n";
			packetString += "AuthType: MD5\r\n";
			packetString += "Username: " + username + "\r\n";
			packetString += "Key: " + key + "\r\n";
			packetString += "\r\n";

            SendData(packetString);

//#ifdef _UNICODE
//			ch = ::theApp.ConvertFromUnicodeToMB(packetString);
//			if (ch) {
//				WriteComm((LPBYTE)ch, packetString.GetLength(), INFINITE);                
//				free(ch);				
//			}
//#else            			
//			BYTE byBuffer[512] = { 0 };
//			int nLen = packetString.GetLength();
//			strcpy((LPSTR)byBuffer, packetString);
//			WriteComm(byBuffer, nLen, INFINITE);
//#endif
						
		}		
	} else {
		if (strData.Right(4)!="\r\n\r\n") {
			m_AppendMsg += strData;				
			if (m_AppendMsg.Right(4)=="\r\n\r\n") {
                m_AppendData = FALSE;
				strData = m_AppendMsg;
				m_AppendMsg = "";
			} else {
				m_AppendData = TRUE;
				return;
			}
		}

		if (m_AppendData) {
			m_AppendData = FALSE;
			strData = m_AppendMsg + strData;
			m_AppendMsg = "";
		}


		while (1) {
			esIndex = strData.Find(_T("\r\n\r\n"));
			if (esIndex!=-1)
				eventData = strData.Left(esIndex+2);
			else
				eventData = strData;

			if (AUTH_ENABLED) {
				if ((eventData.Find(_T("OUTCALL_SIP_USERS"))==-1) &&
					(eventData.Find(_T("OUTCALL_IAX_USERS"))==-1) && 
					(eventData.Find(_T("OUTCALL_SKINNY_LINES"))==-1) &&	eventData!="") {
						AppendMessage(eventData+"\r\n");
					}
			} else {
                AppendMessage(eventData+"\r\n");
			}

			// parse event
			if (eventData.Find(_T("Event: Reload"))!=-1) {
				if (AUTH_ENABLED) {
					SendData("Action: Command\r\nCommand: sip show users\r\nActionID: OUTCALL_SIP_USERS\r\n\r\n");
				}
			} else if (eventData.Find(_T("ActionID: OUTCALL_SIP_USERS"))!=-1) {
                CString line, user, pass;
				int nRet, nStart=0, nSecret;
				m_sipUsers.clear();
				nIndex = eventData.Find(_T("Username"));
				nStart = eventData.Find(_T("\n"), nIndex)+1;
				line = eventData.Mid(nIndex, nStart-nIndex-1);
				nSecret = line.Find(_T("Secret"));
				while ((nRet=eventData.Find(_T("\n"), nStart))!=-1) {
					line = eventData.Mid(nStart, nRet-nStart);
					if (line.Find(_T("--END COMMAND--"))!=-1)
						break;
					nIndex = line.Find(_T(" "));
					user = line.Left(nIndex);
					pass = line.Mid(nSecret, line.Find(_T(" "), nSecret)-nSecret);
					m_sipUsers[user]=pass;
					nStart = nRet+1;					
				}
				SendData("Action: Command\r\nCommand: iax2 show users\r\nActionID: OUTCALL_IAX_USERS\r\n\r\n");				

				/*SendData("Action: Command\r\nCommand: sccp show devices\r\nActionID: OUTCALL_SKINNY_DEVICES\r\n\r\n");

				SendData("Action: Command\r\nCommand: sccp show lines\r\nActionID: OUTCALL_SKINNY_LINES\r\n\r\n");

				SendData("Action: Command\r\nCommand: sccp show channels\r\nActionID: OUTCALL_SKINNY_LINES\r\n\r\n");

				SendData("Action: Command\r\nCommand: help\r\nActionID: OUTCALL_HELP\r\n\r\n");*/
			} else if (eventData.Find(_T("ActionID: OUTCALL_IAX_USERS"))!=-1) {
				CString line, user, pass;
				int nRet, nStart=0, nSecret;
				m_iaxUsers.clear();
				nIndex = eventData.Find(_T("Username"));
				nStart = eventData.Find(_T("\n"), nIndex)+1;
				line = eventData.Mid(nIndex, nStart-nIndex-1);
				nSecret = line.Find(_T("Secret"));
				while ((nRet=eventData.Find(_T("\n"), nStart))!=-1) {
					line = eventData.Mid(nStart, nRet-nStart);
					if (line.Find(_T("--END COMMAND--"))!=-1)
						break;
					nIndex = line.Find(_T(" "));
					user = line.Left(nIndex);
					pass = line.Mid(nSecret);					
					if (pass.Left(4)=="Key:") {
						pass = pass.Mid(5, pass.Find(_T(" "), 5)-5);
					} else if (pass.Find(_T("no secret"))!=-1) {
						pass="";						
					} else {
						pass = pass.Left(pass.Find(_T(" ")));
					}
					m_iaxUsers[user]=pass;
					nStart = nRet+1;					
				}

				SendData("Action: Command\r\nCommand: sccp show lines\r\nActionID: OUTCALL_SKINNY_LINES\r\n\r\n");
			} else if (eventData.Find(_T("ActionID: OUTCALL_SKINNY_LINES"))!=-1) {
				vector<CString> lines;
				SplitString(eventData, "\r\n", lines);
				int index;
				CString name, device, line;
				
				

				if (lines.size()>=4) {
					SplitString(lines[3], "\n", lines);
					
					for (int i=0; i<lines.size(); i++) {
						line = lines[i];
						if (line=="--END COMMAND--") {
							break;
						}
						index = line.Find(_T(" "));
						if (index!=-1) {
							name = line.Left(index);
							device = line.Mid(index+1, line.GetLength());
							device = device.TrimLeft();
							if ((index=device.Find(_T(" ")))!=-1) {
								device = device.Left(index);
								m_sccpUsers[name] = device;
							}
						}
					}
				}
			} else if (eventData.Find(_T("Event: Dial"))!=-1 || eventData.Find(_T("Event: AgentCalled"))!=-1) {
                map<CString, CString> map;
				CString outcall_channel;
				CString source_channel, destination_channel;
				CString caller, from;
				bool bCallFromQueue;
				GetEventValues(eventData, map);				

				if (eventData.Find(_T("Event: Dial"))!=-1) {
					source_channel = map["source"];
					destination_channel = map["destination"];
					callerID1 = map["callerid"];
					callerID2 = map["calleridname"];
					bCallFromQueue = false;
				} else {
                    source_channel = map["channelcalling"];
					destination_channel = map["agentcalled"];
					callerID1 = map["callerid"];
					callerID2 = map["calleridname"];					
					bCallFromQueue = true;
				}

				if (((callerID1=="<unknown>") || (callerID1=="unknown")) && 
					((callerID2=="<unknown>") || (callerID2=="unknown")))
						goto next_event;				

				//now, let's find the caller
				caller = source_channel;
				if ((nIndex=caller.ReverseFind('-'))!=-1)
					caller = caller.Left(nIndex);
				caller.MakeLower();
                                                           
				//now, let's find the callee
				callee = destination_channel;
				if ((nIndex=callee.ReverseFind('-'))!=-1)
					callee = callee.Left(nIndex);			
				callee.MakeLower();

				if ((calledExtension=FindCalledExtension(callee))!="") {
					int number, maxDigits;
					maxDigits = ::theApp.GetProfileInt("Settings", "IgnoreCalls", 0);
					if (maxDigits!=0) {
						number = _tstoi(callerID1.GetBuffer());
						if (number!=0) {
							if (callerID1.GetLength()<maxDigits)
								goto next_event;							
						}
						number = _tstoi(callerID2.GetBuffer());
						if (number!=0) {
							if (callerID2.GetLength()<maxDigits)
								goto next_event;							
						}
					}

					StripCallerID(callerID1, callerID2);

					FormatDateAndTime(callDate, callTime);
					this->channel = destination_channel;
					this->callDate = callDate;
					this->callTime = callTime;
					this->callerID1=callerID1;
					this->callerID2=callerID2;

					if (bCallFromQueue)
						this->QueueExtension = _("Queue:") + CString(" ") + map["extension"];
					else
						this->QueueExtension = "";

					mainFrame->PostMessage(NEW_CALL_MESSAGE, 0, 0);
					::theApp.m_OutcallChannels[destination_channel] = callerID1 + "||" + calledExtension;
					HWND hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Active Calls"));
					if (hwnd!=NULL)
						PostMessage(hwnd, WM_REFRESH_ACTIVE_CALLS, 0, 0);							
				} else if ((from=FindCalledExtension(caller))!="") { // placed call
					CppSQLite3DB db;
					db.open(CStringA(OUTCALL_DB));
					CString query, callDate, callTime;
					CString protocol = caller.Left(caller.Find(_T("/")));
					FormatDateAndTime(callDate, callTime);
					callee = callee.Mid(callee.Find(_T("/"))+1, callee.GetLength());
					::theApp.m_OutcallChannels[source_channel] = from + "||" + callee;

					from = from + "  (" + protocol + ")";
					time_t now;
					time(&now);
					char call_time[40];
					ltoa(now, call_time, 10);
					callee.Replace(_T("'"), _T("''"));
					from.Replace(_T("'"), _T("''"));
					query = "insert into PlacedCalls(Date,Time,Callee,CallerID,Sec) values('"+callDate+"','"+callTime+"','"+callee+"','"+from+"'," + CString(call_time) + ")";
					try {
						db.execDML(query.GetBuffer());					
					} catch (CppSQLite3Exception& e) { }
					HWND hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Call History"));
					if (hwnd!=NULL)
						PostMessage(hwnd, WM_REFRESH_MC_LIST, 0, 0);					
					hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Active Calls"));
					if (hwnd!=NULL)
						PostMessage(hwnd, WM_REFRESH_ACTIVE_CALLS, 0, 0);
				}
			} else if (eventData.Find(_T("Event: Newchannel"))!=-1) { // we monitor this event for compatibility with Asterisk 1.0
				map<CString, CString> map;
				CString outcall_channel;
				GetEventValues(eventData, map);
				int index;				

				if (map["state"].MakeLower()!="ringing")
                    goto next_event;	

				if (map["calleridname"]!="") // this is above asterisk 1.0, we use Dial event for versions above 1.0
					goto next_event;

				channel = map["channel"];

				if (::theApp.m_OutcallChannels.find(channel)!=::theApp.m_OutcallChannels.end())
					goto next_event;


				callerID1 = map["callerid"];

				index = callerID1.Find(_T(" <"));
				if (index!=-1) {
					callerID2 = callerID1.Mid(index+2, callerID1.GetLength()-index-3);
					callerID1 = callerID1.Left(index);
				} else {
                    callerID2 = "";
				}


				if (((callerID1=="<unknown>") || (callerID1=="unknown")) && 
					((callerID2=="<unknown>") || (callerID2=="unknown")))
						goto next_event;
                                                           
				int number, maxDigits;
				maxDigits = ::theApp.GetProfileInt("Settings", "IgnoreCalls", 0);
				if (maxDigits!=0) {
					number = _tstoi(callerID1.GetBuffer());
					if (number!=0) {
						if (callerID1.GetLength()<maxDigits)
							goto next_event;							
					}
					number = _tstoi(callerID2.GetBuffer());
					if (number!=0) {
						if (callerID2.GetLength()<maxDigits)
							goto next_event;							
					}
				}

				//now, let's find the callee
				callee = channel;
				if ((nIndex=callee.ReverseFind('-'))!=-1)
					callee = callee.Left(nIndex);

				if ((callee=="SIP/"+callerID1) || (callee=="IAX2/"+callerID1) || (callee=="IAX/"+callerID1)) {
					goto next_event;
				}
				
				callee.MakeLower();
				if ((calledExtension=FindCalledExtension(callee))!="") {
					FormatDateAndTime(callDate, callTime);

					StripCallerID(callerID1, callerID2);

					this->channel = channel;
					this->callDate = callDate;
					this->callTime = callTime;									
					this->callerID1=callerID1;
					this->callerID2=callerID2;								
					this->QueueExtension = "";

					mainFrame->PostMessage(NEW_CALL_MESSAGE, 0, 0);								
					::theApp.m_OutcallChannels[channel] = callerID + "||" + calledExtension;
					HWND hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Active Calls"));
					if (hwnd!=NULL)
						PostMessage(hwnd, WM_REFRESH_ACTIVE_CALLS, 0, 0);							
				}				
			} else if (eventData.Find(_T("Event: Hangup"))!=-1) {
				map<CString,CallInfo>::iterator iter;
				nIndex = eventData.Find(_T("Channel: "));
				if (nIndex!=-1) {
					nRet = eventData.Find(_T("\r\n"), nIndex+8);
					channel = eventData.Mid(nIndex+9, nRet-nIndex-9);
                    for (iter=m_MissedCalls.begin(); iter != m_MissedCalls.end(); iter++) {
						if (iter->first==channel) {
							InsertCallIntoDB(channel,1);
							m_MissedCalls.erase(iter);
							break;
						}						
					}
					map<CString, CString> &OutcallChannels = ::theApp.m_OutcallChannels;
					map<CString,CString>::iterator oc;
					for (oc=OutcallChannels.begin(); oc != OutcallChannels.end(); oc++) {
						if (oc->first==channel) {
							OutcallChannels.erase(oc);
							HWND hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Active Calls"));
							if (hwnd!=NULL)
								PostMessage(hwnd, WM_REFRESH_ACTIVE_CALLS, 0, 0);
							break;
						}						
					}
				}
			} else if (eventData.Find(_T("Event: Newstate"))!=-1) {
				if ((eventData.Find(_T("State: Up"))!=-1) || (eventData.Find(_T("State: up"))!=-1)) {
					map<CString,CallInfo>::iterator iter;
					nIndex = eventData.Find(_T("Channel: "));
					if (nIndex!=-1) {
						nRet = eventData.Find(_T("\r\n"), nIndex+8);
						channel = eventData.Mid(nIndex+9, nRet-nIndex-9);
						for (iter=m_MissedCalls.begin(); iter != m_MissedCalls.end(); iter++) {
							if (iter->first==channel) {
								InsertCallIntoDB(channel, 0);
								m_MissedCalls.erase(iter);
								break;
							}						
						}					
					}
				}
			} else if (eventData.Find(_T("Event: PeerStatus"))!=-1) {
				map<CString,CallInfo>::iterator iter;
				int length;
				if ((eventData.Find(_T("PeerStatus: Unregistered"))!=-1) || (eventData.Find(_T("PeerStatus: unregistered"))!=-1)) {
					nIndex = eventData.Find(_T("Peer: "));
					if (nIndex!=-1) {
						nRet = eventData.Find(_T("\r\n"), nIndex+5);
						channel = eventData.Mid(nIndex+6, nRet-nIndex-6);
						length = channel.GetLength();
						for (iter=m_MissedCalls.begin(); iter != m_MissedCalls.end(); iter++) {
							if (iter->first.Left(length)==channel) {
								InsertCallIntoDB(iter->first, 1);
								m_MissedCalls.erase(iter);
								iter = m_MissedCalls.begin();
								//break;
							}						
						}					
					}
				}
			} /*else if (eventData.Find("Message: Originate failed")!=-1) {
				MessageBox(NULL, _("Originate failed! Please check your extensions in Settings->Extensions, and make sure you are using the right one in the 'Dial From' select box."), APP_NAME, MB_ICONERROR);
			}*/
			else if (eventData.Find(_T("ActionID: outcall_dial_action"))!=-1) {
                map<CString, CString> map;
				GetEventValues(eventData, map);                
				CString response = map["response"];
				if (response=="Error") {
                    MessageBox(NULL, _("Originate failed! Please check your extensions in Settings->Extensions, and make sure you are using the right one in the 'Dial From' select box."), APP_NAME, MB_ICONERROR);
				}
				HWND hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Dial"));
				if (hwnd) {
					PostMessage(hwnd, WM_ENABLE_DIAL_BUTTON, 0, 0);
				}
				hwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Find"));
				if (hwnd) {
					PostMessage(hwnd, WM_ENABLE_DIAL_BUTTON, 0, 0);					
				}				
			}


			// end parse event
next_event:
			
			if (esIndex==-1)
				break;
			else
				strData = strData.Mid(esIndex + 4);
		}					
	}
	
	//TRACE(strData);
}

///////////////////////////////////////////////////////////////////////////////
// OnEvent
// Send message to parent window to indicate connection status
void CSocketManager::OnEvent(UINT uEvent)
{
	switch( uEvent )
	{
		case EVT_CONSUCCESS:
			//AppendMessage( _T("Connection Established\r\n") );
			break;
		case EVT_CONFAILURE:
			//AppendMessage( _T("Connection Failed\r\n") );
			authenticated = FALSE;
			trayIcon->SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
			trayIcon->SetIcon(IDI_ICON_DISCONNECTED);
			PostMessage(mainFrame->GetSafeHwnd(), STOP_COMM_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
			break;
		case EVT_CONDROP:
			//AppendMessage( _T("Connection Abandonned\r\n") );
			authenticated = FALSE;
			trayIcon->SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
			trayIcon->SetIcon(IDI_ICON_DISCONNECTED);
			PostMessage(mainFrame->GetSafeHwnd(), STOP_COMM_MESSAGE, (WPARAM)NULL, (LPARAM)NULL);
			if (!m_bManualSignOut) {
				if (::theApp.GetProfileInt("Settings", "AutomaticSignIn", 1)==1)
					((CMainFrame*)mainFrame)->SetTimer(TIMER_SIGN_IN, 1500, NULL);				
			}
			m_bManualSignOut = FALSE;
			break;
		case EVT_ZEROLENGTH:
			//AppendMessage( _T("Zero Length Message\r\n") );
			break;
		default:
			TRACE("Unknown Socket event\n");
			break;
	}	
}


CString CSocketManager::FindCalledExtension(CString calleeID) {
	HKEY hKey = HKEY_CURRENT_USER;
	CString sKeyName = REG_KEY_EXTENSIONS;
	CString ret;
	BOOL found=FALSE;
	LONG retcode;
	HKEY hOpenKey = NULL;
	DWORD dwType = REG_SZ;
	TCHAR str[MAX_REG_KEY_NAME];
	CString user, pass, val;
	memset( str, '\0', sizeof(str));

	calleeID.MakeLower();
	
	retcode = RegOpenKeyEx(hKey, (LPCTSTR)sKeyName, 0, KEY_READ, &hOpenKey);
	
	if( retcode != (DWORD)ERROR_SUCCESS )
		return "";

	BYTE *data;
	data = new BYTE[100];
	memset( data, '\0', sizeof(data));

	DWORD Size;
	DWORD dwNo = 0;
	int nIndex;

	map<CString, CString>::iterator iter;


	for (int i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++) 
    {		
		Size = MAX_REG_KEY_NAME;
		DWORD dwNo = 100;
		DWORD value;
		BOOL bFound;
        
		retCode = RegEnumValue(hOpenKey,i,str,&Size,NULL,&dwType,data,&dwNo);

		if (retCode != (DWORD) ERROR_NO_MORE_ITEMS)// && retCode != ERROR_INSUFFICIENT_BUFFER)
		{
			memcpy(&value, data, sizeof(DWORD));			
			val = str;
			if ((nIndex=val.Find(_T("#####")))==-1)
				continue;
			user = val.Left(nIndex);
			pass = val.Mid(nIndex+5);
			bFound = FALSE;

			if (value==0) {
				if (AUTH_ENABLED==0) {
                    bFound = TRUE;
				} else {
					for(iter = m_sipUsers.begin(); iter != m_sipUsers.end(); iter++) {
						if (iter->first==user && iter->second==pass) {
							bFound = TRUE;
							break;						
						}
					}
				}
				if (!bFound)
					goto next;
				ret = ("sip/" + CString(user));
			} else if (value==1) {
				if (AUTH_ENABLED==0) {
                    bFound = TRUE;
				} else {
					for(iter = m_iaxUsers.begin(); iter != m_iaxUsers.end(); iter++) {
						if (iter->first==user && iter->second==pass) {
							bFound = TRUE;
							break;							
						}
					}
				}
				if (!bFound)
					goto next;
				ret = ("iax2/" + CString(user));
			} else if (value==2) {
                if (AUTH_ENABLED==0) {
                    bFound = TRUE;
				} else {
					for(iter = m_sccpUsers.begin(); iter != m_sccpUsers.end(); iter++) {
						if (iter->first==user && iter->second==pass) {
							bFound = TRUE;
							break;						
						}
					}
				}
				if (!bFound)
					goto next;
				ret = ("sccp/" + CString(user));
			}

			ret.MakeLower();

			if (ret==calleeID) {
				found = TRUE;
				ret = user;
				break;
			}

next:
						
			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );

	delete[] data;

	if (found)
		return ret;
	else
		return "";
}


void CSocketManager::FormatDateAndTime(CString &sDate, CString &sTime) {	
	struct tm *currentTime;
	time_t long_time;
	
	sDate = "";
	sTime = "";
    	
	time(&long_time);
	currentTime = localtime(&long_time);

	sDate.Format(_T("%02d.%02d.%d"), currentTime->tm_mday, currentTime->tm_mon + 1, currentTime->tm_year + 1900);
	sTime.Format(_T("%02d:%02d:%02d"), currentTime->tm_hour, currentTime->tm_min , currentTime->tm_sec);

	/*_stprintf(sDate.GetBuffer(15), _T("%02d.%02d.%d"), currentTime->tm_mday, currentTime->tm_mon + 1, currentTime->tm_year + 1900);
	sDate.ReleaseBuffer();
	_stprintf(sTime.GetBuffer(15), _T("%02d:%02d:%02d"), currentTime->tm_hour, currentTime->tm_min , currentTime->tm_sec);
	sTime.ReleaseBuffer();*/
}


void CSocketManager::InsertCallIntoDB(CString channel, BOOL bMissed) {
	CppSQLite3DB db;
	db.open(CStringA(OUTCALL_DB));

	CString from, to, sDate, sTime, protocol;
	CString OC; //this call is from some of outlook contacts
	CallInfo call_info = m_MissedCalls[channel];
	char call_time[40];
	int nIndex;
	
	from = call_info.cid;
	sDate = call_info.sdate;
	sTime = call_info.stime;	
	ltoa(call_info.call_time, call_time, 10);

	nIndex = channel.ReverseFind('-');
	to = channel.Left(nIndex);
	nIndex = to.Find(_T("/"));
	protocol = to.Left(nIndex);
	if (protocol.MakeLower()=="iax2")
		protocol = "IAX";
	else if (protocol.MakeLower()=="sip")
		protocol = "SIP";
	else if (protocol.MakeLower()=="sccp")
		protocol = "SCCP";
	else
		return;

	to = to.Mid(nIndex+1);
	to = to + "  (" + protocol + ")";
	
	CString query;
	to.Replace(_T("'"), _T("''"));
	from.Replace(_T("'"), _T("''"));
	if (bMissed)
		query = "insert into MissedCalls(NewCall,Date,Time,Callee,CallerID,Sec) values(1,'"+sDate+"','"+sTime+"','"+to+"','"+from+"'," + CString(call_time) + ")";
	else
		query = "insert into RecivedCalls(Date,Time,Callee,CallerID,Sec) values('"+sDate+"','"+sTime+"','"+to+"','"+from+"'," + CString(call_time) + ")";
	
	try {
		db.execDML(query.GetBuffer());
	} catch (CppSQLite3Exception& e) { }

	HWND mcDlgHwnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Call History"));
	if (mcDlgHwnd!=NULL)
		PostMessage(mcDlgHwnd, WM_REFRESH_MC_LIST, 0, 0);
}

void CSocketManager::GetEventValues(CString eventData, map<CString, CString> &ret) {
	CString key, value, line;
	int nRet, nStart=0, nIndex;	
	while ((nRet=eventData.Find(_T("\r\n"), nStart))!=-1) {
		line = eventData.Mid(nStart, nRet-nStart);
		nIndex = line.Find(_T(":"));
		key = line.Left(nIndex);
		key = key.MakeLower();
		value = line.Mid(nIndex+2);
        ret[key] = value;
		nStart = nRet+2;	
	}
}


void CSocketManager::StripCallerID(CString &cid1, CString &cid2) {
	CString StripOptions = ::theApp.GetProfileString("Settings", "StripOptions", "");
	if (StripOptions=="")
		return;

	vector<CString> prefixes_temp;
	SplitString(StripOptions, ";", prefixes_temp);
    vector<CString> prefixes;

	/*** sorty by length ***/
	for (int i=0; i<prefixes_temp.size(); i++) {		
		if (prefixes_temp[i]=="")
			continue;
		
		int j=0;
		for (j=0; j<prefixes.size(); j++) {
			if (prefixes[j].GetLength()<=prefixes_temp[i].GetLength())
				break;			
		}
		prefixes.insert(prefixes.begin()+j, prefixes_temp[i]);
	}

	if (cid1!="") {
		for (int i=0; i<prefixes.size(); i++) {
			if (cid1.Find(prefixes[i])==0) {
				cid1 = cid1.Mid(prefixes[i].GetLength(), cid1.GetLength());
				break;
			}
		}
	}

	if (cid2!="") {
        for (int i=0; i<prefixes.size(); i++) {
			if (cid2.Find(prefixes[i])==0) {
				cid2 = cid2.Mid(prefixes[i].GetLength(), cid2.GetLength());
				break;
			}
		}
	}
}


void CSocketManager::SendData(CString data) {		
#ifdef _UNICODE
	CStringA ansi(data);
	WriteComm((LPBYTE)ansi.GetBuffer(), ansi.GetLength(), INFINITE);
	/*char *ch = ::theApp.ConvertFromUnicodeToMB(data);
	if (ch) {
		WriteComm((LPBYTE)ch, strlen(ch), INFINITE);
		free(ch);
	}*/
#else            	
	WriteComm((LPBYTE)data.GetBuffer(), data.GetLength(), INFINITE);
	/*int nLen = data.GetLength();
	BYTE *buffer = new BYTE[nLen+1];	 
	strcpy((LPSTR)buffer, data);
	WriteComm(buffer, nLen, INFINITE);
	delete buffer;*/
#endif	
}
