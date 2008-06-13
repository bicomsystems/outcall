////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// File: MAPIEx.h
// Description: Windows Extended MAPI class
//
// Copyright (C) 2005, Noel Dillabough
//
// This source code is free to use and modify provided this notice remains intact and that any enhancements
// or bug fixes are posted to the CodeProject page hosting this class for the community to benefit.
//
// Usage: see the Codeproject article at http://www.codeproject.com/internet/CMapiEx.asp
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MAPIDLL.h"
#include "MAPIEx.h"
#include <MapiUtil.h>

#ifdef _WIN32_WCE
#pragma comment(lib,"cemapi.lib")
#pragma comment(lib,"UUID.lib")
#else
#pragma comment (lib,"mapi32")
#endif
#pragma comment(lib,"Ole32.lib")

/////////////////////////////////////////////////////////////
// CMAPIEx

#ifdef _UNICODE
int CMAPIEx::cm_nMAPICode=MAPI_UNICODE;
#else
int CMAPIEx::cm_nMAPICode=0;
#endif

CMAPIEx::CMAPIEx()
{
	m_pSession=NULL;
	m_pMsgStore=NULL;
	m_pFolder=NULL;	
	m_pContents=NULL;
	m_pHierarchy=NULL;
}

CMAPIEx::~CMAPIEx()
{
	Logout();
}

BOOL CMAPIEx::Init()
{
#ifdef _WIN32_WCE
	if(CoInitializeEx(NULL, COINIT_MULTITHREADED)!=S_OK) return FALSE;
#endif
	if(MAPIInitialize(NULL)!=S_OK) return FALSE;
	return TRUE;
}

void CMAPIEx::Term()
{
	MAPIUninitialize();
#ifdef _WIN32_WCE
	CoUninitialize();
#endif
}

BOOL CMAPIEx::Login(LPCTSTR szProfileName)
{
	return (MAPILogonEx(NULL,(LPTSTR)szProfileName,NULL,0,&m_pSession)==S_OK);
}

void CMAPIEx::Logout()
{
	RELEASE(m_pHierarchy);
	RELEASE(m_pContents);
	RELEASE(m_pFolder);
	RELEASE(m_pMsgStore);
	RELEASE(m_pSession);
}

LPCTSTR CMAPIEx::GetProfileName()
{
	if(!m_pSession) return NULL;

	static CString strProfileName;
	LPSRowSet pRows=NULL;
	const int nProperties=2;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME, PR_RESOURCE_TYPE}};

	IMAPITable*	pStatusTable;
	if(m_pSession->GetStatusTable(0,&pStatusTable)==S_OK) {
		if(pStatusTable->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) {
			while(TRUE) {
				if(pStatusTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
				else if(pRows->cRows!=1) FreeProws(pRows);
				else if(pRows->aRow[0].lpProps[1].Value.ul==MAPI_SUBSYSTEM) {
					strProfileName=pRows->aRow[0].lpProps[0].Value.LPSZ;
					FreeProws(pRows);
				} else {
					FreeProws(pRows);
					continue;
				}
				break;
			}
		}
		RELEASE(pStatusTable);
	}
	return strProfileName;
}

BOOL CMAPIEx::OpenMessageStore(LPCTSTR szStore)
{
	if(!m_pSession) return FALSE;

	LPSRowSet pRows=NULL;
	const int nProperties=3;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME, PR_ENTRYID, PR_DEFAULT_STORE}};

	BOOL bResult=FALSE;
	IMAPITable*	pMsgStoresTable;
	if(m_pSession->GetMsgStoresTable(0, &pMsgStoresTable)==S_OK) {
		if(pMsgStoresTable->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) {
			while(TRUE) {
				if(pMsgStoresTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
				else if(pRows->cRows!=1) FreeProws(pRows);
				else {
					if(!szStore) { 
						if(pRows->aRow[0].lpProps[2].Value.b) bResult=TRUE;
					} else {
						CString strStore=pRows->aRow[0].lpProps[0].Value.LPSZ;
						if(strStore.Find(szStore)!=-1) bResult=TRUE;
					}
					if(!bResult) {
						FreeProws(pRows);
						continue;
					}
				}
				break;
			}
			if(bResult) {
				RELEASE(m_pMsgStore);
				bResult=(m_pSession->OpenMsgStore(NULL,pRows->aRow[0].lpProps[1].Value.bin.cb,(ENTRYID*)pRows->aRow[0].lpProps[1].Value.bin.lpb,NULL,MDB_NO_DIALOG | MAPI_BEST_ACCESS,&m_pMsgStore)==S_OK);
				FreeProws(pRows);
			}
		}
		RELEASE(pMsgStoresTable);
	}
	return bResult;
}

LPMAPIFOLDER CMAPIEx::OpenInbox()
{
	if(!m_pMsgStore) return NULL;
	RELEASE(m_pFolder);

#ifdef _WIN32_WCE
	LPSPropValue props=NULL;
	ULONG cValues=0;
    ULONG rgTags[]={ 1, PR_CE_IPM_INBOX_ENTRYID };

	if(m_pMsgStore->GetProps((LPSPropTagArray) rgTags, m_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, MAPI_MODIFY,NULL,(LPUNKNOWN*)&m_pFolder);
	MAPIFreeBuffer(props);
#else
	ULONG cbEntryID;
	LPENTRYID pEntryID;
	DWORD dwObjType;

	if(m_pMsgStore->GetReceiveFolder(NULL,0,&cbEntryID,&pEntryID,NULL)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(cbEntryID,pEntryID, NULL, MAPI_MODIFY,&dwObjType,(LPUNKNOWN*)&m_pFolder);
	MAPIFreeBuffer(pEntryID);
#endif

	return m_pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenOutbox()
{
	if(!m_pMsgStore) return NULL;
	RELEASE(m_pFolder);

    LPSPropValue props=NULL;
    ULONG cValues=0;
	DWORD dwObjType;
	ULONG rgTags[]={ 1, PR_IPM_OUTBOX_ENTRYID };

	if(m_pMsgStore->GetProps((LPSPropTagArray) rgTags, cm_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, MAPI_MODIFY, &dwObjType,(LPUNKNOWN*)&m_pFolder);
	MAPIFreeBuffer(props);

	return m_pFolder;
}

LPMAPITABLE CMAPIEx::GetHierarchy(LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return NULL;
	}
	RELEASE(m_pHierarchy);
	if(pFolder->GetHierarchyTable(0,&m_pHierarchy)!=S_OK) return NULL;

	const int nProperties=2;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME, PR_ENTRYID}};
	if(m_pHierarchy->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) return m_pHierarchy;
	return NULL;
}

LPMAPIFOLDER CMAPIEx::GetNextSubFolder(CString& strFolderName,LPMAPIFOLDER pFolder)
{
	if(!m_pHierarchy) return NULL;
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	DWORD dwObjType;
	LPSRowSet pRows=NULL;

	LPMAPIFOLDER pSubFolder=NULL;
	if(m_pHierarchy->QueryRows(1,0,&pRows)==S_OK) {
		if(pRows->cRows) {
			if(pFolder->OpenEntry(pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin.cb,(LPENTRYID)pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin.lpb, NULL, MAPI_MODIFY, &dwObjType,(LPUNKNOWN*)&pSubFolder)==S_OK) {
				strFolderName=pRows->aRow[0].lpProps[0].Value.LPSZ;
			}
		}
		FreeProws(pRows);
	}
	MAPIFreeBuffer(pRows);
	return pSubFolder;
}

// High Level function to open a sub folder by iterating recursively (DFS) over all folders 
// (use instead of manually calling GetHierarchy and GetNextSubFolder)
LPMAPIFOLDER CMAPIEx::OpenSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	LPMAPIFOLDER pSubFolder=NULL;
	LPMAPITABLE pHierarchy;

	RELEASE(m_pHierarchy);
	pHierarchy=GetHierarchy(pFolder);
	if(pHierarchy) {
		CString strFolder;
		LPMAPIFOLDER pRecurse=NULL;
		do {
			RELEASE(pSubFolder);
			m_pHierarchy=pHierarchy;
			pSubFolder=GetNextSubFolder(strFolder,pFolder);
			if(pSubFolder) {
				if(!strFolder.CompareNoCase(szSubFolder)) break;
				m_pHierarchy=NULL; // so we don't release it in subsequent drilldown
				pRecurse=OpenSubFolder(szSubFolder,pSubFolder);
				if(pRecurse) {
					RELEASE(pSubFolder);
					pSubFolder=pRecurse;
					break;
				}
			}
		} while(pSubFolder);
		RELEASE(pHierarchy);
		m_pHierarchy=NULL;
	}
	// this may occur many times depending on how deep the recursion is; make sure we haven't already assigned m_pFolder
	if(pSubFolder && m_pFolder!=pSubFolder) {
		RELEASE(m_pFolder);
		m_pFolder=pSubFolder;
	}
	return pSubFolder;
} 

BOOL CMAPIEx::GetContents(LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}
	RELEASE(m_pContents);
	if(pFolder->GetContentsTable(0,&m_pContents)!=S_OK) return FALSE;

	const int nProperties=MESSAGE_COLS;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_MESSAGE_FLAGS, PR_ENTRYID }};
	return (m_pContents->SetColumns((LPSPropTagArray)&Columns,0)==S_OK);
}

BOOL CMAPIEx::GetNextMessage(CMAPIMessage& message,BOOL bUnreadOnly)
{
	if(!m_pContents) return FALSE;
	
	DWORD dwMessageFlags;
	LPSRowSet pRows=NULL;

	BOOL bResult=FALSE;
	while(m_pContents->QueryRows(1,0,&pRows)==S_OK) {
		if(pRows->cRows) {
			dwMessageFlags=pRows->aRow[0].lpProps[PROP_MESSAGE_FLAGS].Value.ul;
			if(bUnreadOnly &&  dwMessageFlags&MSGFLAG_READ) {
				FreeProws(pRows);
				continue;
			}
			bResult=message.Open(m_pSession,pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin,dwMessageFlags);
		}
		FreeProws(pRows);
		break;
	}
	MAPIFreeBuffer(pRows);
	return bResult;
}

// Creates a subfolder under pFolder, opens the folder if it already exists
LPMAPIFOLDER CMAPIEx::CreateSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return NULL;
	}
	
	LPMAPIFOLDER pSubFolder=NULL;
	ULONG ulFolderType=FOLDER_GENERIC;
	ULONG ulFlags=OPEN_IF_EXISTS | cm_nMAPICode;

	pFolder->CreateFolder(ulFolderType,(LPTSTR)szSubFolder,NULL,NULL,ulFlags,&pSubFolder);
	return pSubFolder;
}

// Deletes a sub folder and ALL sub folders/messages
BOOL CMAPIEx::DeleteSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	LPMAPIFOLDER pSubFolder=NULL;
	if(GetHierarchy(pFolder)) {
		CString strFolder;
		do {
			RELEASE(pSubFolder);
			pSubFolder=GetNextSubFolder(strFolder,pFolder);
			if(pSubFolder && !strFolder.CompareNoCase(szSubFolder)) break;
		} while(pSubFolder);
	}
	return DeleteSubFolder(pSubFolder,pFolder);
}

// Deletes a sub folder and ALL sub folders/messages
BOOL CMAPIEx::DeleteSubFolder(LPMAPIFOLDER pSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pSubFolder) return FALSE;

	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	LPSPropValue props=NULL;
	ULONG cValues=0;
	ULONG rgTags[]={ 1, PR_ENTRYID };

	if(pSubFolder->GetProps((LPSPropTagArray) rgTags, CMAPIEx::cm_nMAPICode, &cValues, &props)==S_OK) {
		HRESULT hr=pFolder->DeleteFolder(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb,NULL,NULL,DEL_FOLDERS|DEL_MESSAGES);
		MAPIFreeBuffer(props);
		return (hr==S_OK);
	}
	return FALSE;
}

BOOL CMAPIEx::DeleteMessage(CMAPIMessage& message,LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	ENTRYLIST entries={ 1, message.GetEntryID() };
	HRESULT hr=pFolder->DeleteMessages(&entries,NULL,NULL,0);
	return (hr==S_OK);
}

BOOL CMAPIEx::CopyMessage(CMAPIMessage& message,LPMAPIFOLDER pFolderDest,LPMAPIFOLDER pFolderSrc)
{
	if(!pFolderDest) return FALSE;

	if(!pFolderSrc) {
		pFolderSrc=m_pFolder;
		if(!pFolderSrc) return FALSE;
	}

	ENTRYLIST entries={ 1, message.GetEntryID() };
	HRESULT hr=pFolderSrc->CopyMessages(&entries,NULL,pFolderDest,NULL,NULL,0);
	return (hr==S_OK);
}

BOOL CMAPIEx::MoveMessage(CMAPIMessage& message,LPMAPIFOLDER pFolderDest,LPMAPIFOLDER pFolderSrc)
{
	if(!pFolderDest) return FALSE;

	if(!pFolderSrc) {
		pFolderSrc=m_pFolder;
		if(!pFolderSrc) return FALSE;
	}

	ENTRYLIST entries={ 1, message.GetEntryID() };
	HRESULT hr=pFolderSrc->CopyMessages(&entries,NULL,pFolderDest,NULL,NULL,MESSAGE_MOVE);
	return (hr==S_OK);
}

/////////////////////////////////////////////////////////////
// CMAPIMessage

CMAPIMessage::CMAPIMessage()
{
	m_pMessage=NULL;
	m_bAttachments=FALSE;
	m_pMAPI=NULL;
	m_entry.cb=0;
	SetEntryID(NULL);
}

CMAPIMessage::CMAPIMessage(CMAPIMessage& message)
{
	m_pMessage=NULL;
	m_bAttachments=FALSE;
	m_pMAPI=NULL;
	m_entry.cb=0;
	*this=message;
}

CMAPIMessage::~CMAPIMessage()
{
	SetEntryID(NULL);
	Close();
}

void CMAPIMessage::SetEntryID(SBinary* pEntry)
{
	if(m_entry.cb) delete [] m_entry.lpb;
	m_entry.lpb=NULL;

	if(pEntry) {
		m_entry.cb=pEntry->cb;
		if(m_entry.cb) {
			m_entry.lpb=new BYTE[m_entry.cb];
			memcpy(m_entry.lpb,pEntry->lpb,m_entry.cb);
		}
	} else {
		m_entry.cb=0;
	}
}

BOOL CMAPIMessage::Open(IMAPISession* pSession,SBinary entry,DWORD dwMessageFlags)
{
	Close();
	m_dwMessageFlags=dwMessageFlags;
	ULONG ulObjType;
	if(pSession->OpenEntry(entry.cb,(LPENTRYID)entry.lpb,NULL,MAPI_BEST_ACCESS,&ulObjType,(LPUNKNOWN*)&m_pMessage)!=S_OK) return FALSE;

	SetEntryID(&entry);
	FillSenderName();
	FillSenderEmail();
	FillSubject();
	FillBody();
	FillHasAttachments();
	FillReceivedTime();
	
	return TRUE;
}

void CMAPIMessage::Close()
{
	RELEASE(m_pMessage);
	m_bAttachments=FALSE;
	m_pMAPI=NULL;
}

HRESULT CMAPIMessage::GetProperty(ULONG ulProperty,LPSPropValue &prop)
{
	ULONG ulPropCount;
	ULONG p[2]={ 1,ulProperty };
	return m_pMessage->GetProps((LPSPropTagArray)p, CMAPIEx::cm_nMAPICode, &ulPropCount, &prop);
}

void CMAPIMessage::FillSenderName()
{
	LPSPropValue prop;
	if(GetProperty(PR_SENDER_NAME,prop)==S_OK) m_strSenderName=prop->Value.LPSZ;
	else m_strSenderName=_T("");
	MAPIFreeBuffer(prop);
}

void CMAPIMessage::FillSenderEmail()
{
	LPSPropValue prop;
	if(GetProperty(PR_SENDER_EMAIL_ADDRESS,prop)==S_OK) m_strSenderEmail=prop->Value.LPSZ;
	else m_strSenderEmail=_T("");
	MAPIFreeBuffer(prop);
}

void CMAPIMessage::FillSubject()
{
	LPSPropValue prop;
	if(GetProperty(PR_SUBJECT,prop)==S_OK) m_strSubject=prop->Value.LPSZ;
	else m_strSubject=_T("");
	MAPIFreeBuffer(prop);
}

void CMAPIMessage::FillBody()
{
	m_strBody=_T("");
	IStream* pStream;
	if(m_pMessage->OpenProperty(PR_BODY,&IID_IStream,STGM_READ,NULL,(LPUNKNOWN*)&pStream)!=S_OK) return;

#ifdef _WIN32_WCE
	const int BUF_SIZE=1024;
#else
	const int BUF_SIZE=4096;
#endif
	TCHAR szBuf[BUF_SIZE+1];
	ULONG ulNumChars;
	
	do {
		pStream->Read(szBuf,BUF_SIZE*sizeof(TCHAR),&ulNumChars);
		ulNumChars/=sizeof(TCHAR);
		szBuf[min(BUF_SIZE,ulNumChars)]=0;
		m_strBody+=szBuf;
	} while(ulNumChars>=BUF_SIZE);

	RELEASE(pStream);
}

void CMAPIMessage::FillHasAttachments()
{
	m_bAttachments=FALSE;

	LPSPropValue prop;
	if(GetProperty(PR_HASATTACH,prop)==S_OK) {
		m_bAttachments=prop->Value.b;
	} 
	MAPIFreeBuffer(prop);
}

void  CMAPIMessage::FillReceivedTime()
{
	LPSPropValue prop;
	if(GetProperty(PR_MESSAGE_DELIVERY_TIME,prop)==S_OK) {
		m_tmReceived=CTime(prop->Value.ft);
	}
	MAPIFreeBuffer(prop);
}

LPCTSTR CMAPIMessage::GetReceivedTime(LPCTSTR szFormat)
{
	static CString strTime;
	CString strFormat=szFormat ? szFormat : _T("%m/%d/%Y %I:%M %p");
	strTime=m_tmReceived.Format(strFormat);
	return strTime;
}

BOOL CMAPIMessage::SaveAttachment(LPATTACH pAttachment,LPCTSTR szPath)
{
	CFile file;
	if(!file.Open(szPath,CFile::modeCreate | CFile::modeWrite)) return FALSE;

	IStream* pStream;
	if(pAttachment->OpenProperty(PR_ATTACH_DATA_BIN,&IID_IStream,STGM_READ,NULL,(LPUNKNOWN*)&pStream)!=S_OK) {
		file.Close();
		return FALSE;
	}

	const int BUF_SIZE=4096;
	BYTE b[BUF_SIZE];
	ULONG ulRead;

	do {
		pStream->Read(&b,BUF_SIZE,&ulRead);
		if(ulRead) file.Write(b,ulRead);
	} while(ulRead>=BUF_SIZE);

	file.Close();
	RELEASE(pStream);
	return TRUE;
}

// use nIndex of -1 to save all attachments to szFolder
BOOL CMAPIMessage::SaveAttachments(LPCTSTR szFolder,int nIndex)
{
	LPMAPITABLE pAttachTable=NULL;
	if(m_pMessage->GetAttachmentTable(0,&pAttachTable)!=S_OK) return NULL;

	CString strPath;
	BOOL bResult=FALSE;
	enum { PROP_ATTACH_NUM, PROP_ATTACH_FILENAME, ATTACH_COLS };
	static SizedSPropTagArray(ATTACH_COLS,Columns)={ATTACH_COLS, PR_ATTACH_NUM, PR_ATTACH_LONG_FILENAME };
	if(pAttachTable->SetColumns((LPSPropTagArray)&Columns,0)==S_OK) {
		int i=0;
		LPSRowSet pRows=NULL;
		while(TRUE) {
			if(pAttachTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
			else if(!pRows->cRows) FreeProws(pRows);
			else if(i<nIndex) {
				i++;
				continue;
			} else {
				LPATTACH pAttachment;
				if(m_pMessage->OpenAttach(pRows->aRow[0].lpProps[PROP_ATTACH_NUM].Value.bin.cb,NULL,0,&pAttachment)==S_OK) {
					strPath.Format(_T("%s\\%s"),szFolder,pRows->aRow[0].lpProps[PROP_ATTACH_FILENAME].Value.LPSZ);
					if(!SaveAttachment(pAttachment,strPath)) {
						pAttachment->Release();
						FreeProws(pRows);
						RELEASE(pAttachTable);
						return FALSE;
					}
					bResult=TRUE;
					pAttachment->Release();
				}

				FreeProws(pRows);
				if(nIndex==-1) continue;
			}
			break;
		}
	}
	RELEASE(pAttachTable);
	return bResult;
}

BOOL CMAPIMessage::Create(CMAPIEx* pMAPI,int nPriority)
{
	if(!pMAPI) return FALSE;
	LPMAPIFOLDER pOutbox=pMAPI->GetFolder();
	if(!pOutbox) pOutbox=pMAPI->OpenOutbox();
	if(!pOutbox || pOutbox->CreateMessage(NULL,0,&m_pMessage)!=S_OK) return FALSE;
	m_pMAPI=pMAPI;

    SPropValue prop;
	prop.ulPropTag=PR_MESSAGE_FLAGS;
	prop.Value.l=MSGFLAG_UNSENT | MSGFLAG_FROMME;
	m_pMessage->SetProps(1,&prop,NULL);

	LPSPropValue props=NULL;
	ULONG cValues=0;
	ULONG rgTags[]={ 1, PR_IPM_SENTMAIL_ENTRYID };

	if(m_pMAPI->GetMessageStore()->GetProps((LPSPropTagArray) rgTags, CMAPIEx::cm_nMAPICode, &cValues, &props)==S_OK) {
		prop.ulPropTag=PR_SENTMAIL_ENTRYID;
		prop.Value.bin.cb=props[0].Value.bin.cb;
		prop.Value.bin.lpb=props[0].Value.bin.lpb;
		m_pMessage->SetProps(1,&prop,NULL);
	}

	if(nPriority!=IMPORTANCE_NORMAL) {
		prop.ulPropTag=PR_IMPORTANCE;
		prop.Value.l=nPriority;
		m_pMessage->SetProps(1,&prop,NULL);
	}

#ifdef _WIN32_WCE
	prop.ulPropTag=PR_MSG_STATUS;
	prop.Value.ul=MSGSTATUS_RECTYPE_SMTP;
	m_pMessage->SetProps(1,&prop,NULL);
#endif

	MAPIFreeBuffer(props);
	return TRUE;
}

// used only by WinCE, pass in MSGSTATUS_RECTYPE_SMS to send an SMS 
// (default is set to MSGSTATUS_RECTYPE_SMTP in Create(...) above)
void CMAPIMessage::SetMessageStatus(int nMessageStatus)
{
	SPropValue prop;
	prop.ulPropTag=PR_MSG_STATUS;
	prop.Value.ul=nMessageStatus;
	m_pMessage->SetProps(1,&prop,NULL);
}

void CMAPIMessage::MarkAsRead(BOOL bRead)
{
	ULONG ulFlags=bRead ? 0 : CLEAR_READ_FLAG;
	if(m_pMessage) m_pMessage->SetReadFlag(ulFlags);
}

BOOL CMAPIMessage::AddRecipient(LPCTSTR szEmail,int nType)
{
	if(!m_pMessage || !m_pMAPI) return FALSE;

#ifndef _WIN32_WCE
	LPADRBOOK pAddressBook;
	if(m_pMAPI->GetSession()->OpenAddressBook(0, NULL, AB_NO_DIALOG, &pAddressBook)!=S_OK) return FALSE;
#endif

	int nBufSize=CbNewADRLIST(1);
	LPADRLIST pAddressList=NULL;
	MAPIAllocateBuffer(nBufSize,(LPVOID FAR*)&pAddressList);
	memset(pAddressList,0,nBufSize);

	const int nProperties=2;
	pAddressList->cEntries=1;

	pAddressList->aEntries[0].ulReserved1=0;
	pAddressList->aEntries[0].cValues=nProperties;

	MAPIAllocateBuffer(sizeof(SPropValue)*nProperties,(LPVOID FAR*)&pAddressList->aEntries[0].rgPropVals);
	memset(pAddressList->aEntries[0].rgPropVals, 0, sizeof(SPropValue)*nProperties);

	pAddressList->aEntries[0].rgPropVals[0].ulPropTag=PR_RECIPIENT_TYPE;
	pAddressList->aEntries[0].rgPropVals[0].Value.ul=nType;

#ifdef _WIN32_WCE
	pAddressList->aEntries[0].rgPropVals[1].ulPropTag=PR_EMAIL_ADDRESS;
	pAddressList->aEntries[0].rgPropVals[1].Value.LPSZ=(TCHAR*)szEmail;
#else
	pAddressList->aEntries[0].rgPropVals[1].ulPropTag=PR_DISPLAY_NAME;
	pAddressList->aEntries[0].rgPropVals[1].Value.LPSZ=(TCHAR*)szEmail;
#endif

#ifdef _WIN32_WCE
	HRESULT hr=m_pMessage->ModifyRecipients(MODRECIP_ADD, pAddressList);
#else
	HRESULT hr=E_INVALIDARG;
	if(pAddressBook->ResolveName(0, CMAPIEx::cm_nMAPICode, NULL, pAddressList)==S_OK) hr=m_pMessage->ModifyRecipients(MODRECIP_ADD, pAddressList);
#endif
	MAPIFreeBuffer(pAddressList->aEntries[0].rgPropVals);
	MAPIFreeBuffer(pAddressList);
#ifndef _WIN32_WCE
	RELEASE(pAddressBook);
#endif
	return (hr==S_OK);
}

void CMAPIMessage::SetSubject(LPCTSTR szSubject)
{
	m_strSubject=szSubject;
	if(m_strSubject.GetLength() && m_pMessage) {
		SPropValue prop;
		prop.ulPropTag=PR_SUBJECT;
		prop.Value.LPSZ=(TCHAR*)szSubject;
		m_pMessage->SetProps(1,&prop,NULL);
	}
}

void CMAPIMessage::SetSenderName(LPCTSTR szSenderName)
{
	m_strSenderName=szSenderName;
	if(m_strSenderName.GetLength() && m_pMessage) {
		SPropValue prop;
		prop.ulPropTag=PR_SENDER_NAME;
		prop.Value.LPSZ=(TCHAR*)szSenderName;
		m_pMessage->SetProps(1,&prop,NULL);
	}
}

void CMAPIMessage::SetSenderEmail(LPCTSTR szSenderEmail)
{
	m_strSenderEmail=szSenderEmail;
	if(m_strSenderEmail.GetLength() && m_pMessage) {
		SPropValue prop;
		prop.ulPropTag=PR_SENDER_EMAIL_ADDRESS;
		prop.Value.LPSZ=(TCHAR*)szSenderEmail;
		m_pMessage->SetProps(1,&prop,NULL);
	}
}
void CMAPIMessage::SetBody(LPCTSTR szBody)
{
	m_strBody=szBody;
	if(m_strBody.GetLength() && m_pMessage) {
		LPSTREAM pStream=NULL;
        if(m_pMessage->OpenProperty(PR_BODY, &IID_IStream, 0, MAPI_MODIFY | MAPI_CREATE, (LPUNKNOWN*)&pStream)==S_OK) {
	        pStream->Write(szBody,(ULONG)(_tcslen(szBody)+1)*sizeof(TCHAR),NULL);
        }
        RELEASE(pStream);
	}
}

BOOL CMAPIMessage::AddAttachment(LPCTSTR szPath,LPCTSTR szName)
{
	if(!m_pMessage) return FALSE;

	IAttach* pAttachment=NULL;
	ULONG ulAttachmentNum=0;

	CFile file;
	if(!file.Open(szPath,CFile::modeRead)) return FALSE;

	LPTSTR szFileName=(LPTSTR)szName;
	if(!szFileName) {
		szFileName=(LPTSTR)szPath;
		for(int i=(int)_tcsclen(szPath)-1;i>=0;i--) if(szPath[i]=='\\' || szPath[i]=='/') {
			szFileName=(LPTSTR)&szPath[i+1];
			break;
		}
	}

	if(m_pMessage->CreateAttach(NULL,0,&ulAttachmentNum,&pAttachment)!=S_OK) {
		file.Close();
		return FALSE;
	}

	const int nProperties=3;
	SPropValue prop[nProperties];
	memset(prop,0,sizeof(SPropValue)*nProperties);
	prop[0].ulPropTag=PR_ATTACH_METHOD;
	prop[0].Value.ul=ATTACH_BY_VALUE;
	prop[1].ulPropTag=PR_ATTACH_PATHNAME;
	prop[1].Value.LPSZ=(TCHAR*)szPath;
	prop[1].ulPropTag=PR_ATTACH_FILENAME;
	prop[1].Value.LPSZ=(TCHAR*)szFileName;
	prop[2].ulPropTag=PR_RENDERING_POSITION;
	prop[2].Value.l=-1;
	if(pAttachment->SetProps(nProperties,prop,NULL)==S_OK) {
		LPSTREAM pStream=NULL;
		if(pAttachment->OpenProperty(PR_ATTACH_DATA_BIN, &IID_IStream, 0, MAPI_MODIFY | MAPI_CREATE, (LPUNKNOWN*)&pStream)==S_OK) {
			const int BUF_SIZE=4096;
			BYTE pData[BUF_SIZE];
			ULONG ulSize=0,ulRead,ulWritten;

			ulRead=file.Read(pData,BUF_SIZE);
			while(ulRead) {
				pStream->Write(pData,ulRead,&ulWritten);
				ulSize+=ulRead;
				ulRead=file.Read(pData,BUF_SIZE);
			}

			pStream->Commit(STGC_DEFAULT);
			RELEASE(pStream);
			file.Close();

			prop[0].ulPropTag=PR_ATTACH_SIZE;
			prop[0].Value.ul=ulSize;
			pAttachment->SetProps(1, prop, NULL);

			pAttachment->SaveChanges(KEEP_OPEN_READONLY);
			RELEASE(pAttachment);
			m_bAttachments=TRUE;
			return TRUE;
		}
	}

	file.Close();
	RELEASE(pAttachment);
	return FALSE;
}

// request a Read Receipt sent to szReceiverEmail 
void CMAPIMessage::SetReadReceipt(BOOL bSet,LPCTSTR szReceiverEmail)
{
	if(!m_pMessage) return;

	SPropValue prop;
	prop.ulPropTag=PR_READ_RECEIPT_REQUESTED;
	prop.Value.b=bSet;
	m_pMessage->SetProps(1,&prop,NULL);

	if(bSet && szReceiverEmail && _tcslen(szReceiverEmail)>0) {
		prop.ulPropTag=PR_READ_RECEIPT_SEARCH_KEY;
		prop.Value.LPSZ=(TCHAR*)szReceiverEmail;
		m_pMessage->SetProps(1,&prop,NULL);
	}
}

BOOL CMAPIMessage::Send()
{
	if(m_pMessage && m_pMessage->SubmitMessage(0)==S_OK) {
		Close();
		return TRUE;
	}
	return FALSE;
}

// shallow copy only, no message pointer
CMAPIMessage& CMAPIMessage::operator=(CMAPIMessage& message)
{
	m_strSenderName=message.m_strSenderName;
	m_strSenderEmail=message.m_strSenderEmail;
	m_strSubject=message.m_strSubject;
	m_strBody=message.m_strBody;

	m_dwMessageFlags=message.m_dwMessageFlags;
	m_tmReceived=message.m_tmReceived;
	SetEntryID(message.GetEntryID());
	return *this;
}

BOOL CMAPIMessage::operator==(CMAPIMessage& message)
{
	if(!m_entry.cb || !message.m_entry.cb || m_entry.cb!=message.m_entry.cb) return FALSE;
	if(memcmp(&m_entry.lpb,&message.m_entry.lpb,m_entry.cb)) return FALSE;
	return (!m_strSubject.Compare(message.m_strSubject));
}
