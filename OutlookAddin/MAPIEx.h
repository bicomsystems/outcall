////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// File: MAPIEx.h
// Description: Windows Extended MAPI class 
//				Works in Windows CE and Windows XP with Outlook 2000 and 2003, maybe others but untested
//
// Copyright (C) 2005, Noel Dillabough
//
// This source code is free to use and modify provided this notice remains intact and that any enhancements
// or bug fixes are posted to the CodeProject page hosting this class for all to benefit.
//
// Usage: see the Codeproject article at http://www.codeproject.com/internet/CMapiEx.asp
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __MAPIEX_H__
#define __MAPIEX_H__

#ifdef _WIN32_WCE
#include <cemapi.h>
#include <objidl.h>
#else
#include <mapix.h>
#include <objbase.h>
#endif

class CMAPIMessage;

#define RELEASE(s) if(s!=NULL) { s->Release();s=NULL; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMAPIEx

class AFX_EXT_CLASS CMAPIEx 
{
public:
	CMAPIEx();
	~CMAPIEx();

	enum { PROP_MESSAGE_FLAGS, PROP_ENTRYID, MESSAGE_COLS };

// Attributes
public:
	static int cm_nMAPICode;

protected:
	IMAPISession* m_pSession;
	LPMDB m_pMsgStore;
	LPMAPIFOLDER m_pFolder;
	LPMAPITABLE m_pHierarchy;
	LPMAPITABLE m_pContents;

// Operations
public:
	static BOOL Init();
	static void Term();

	IMAPISession* GetSession() { return m_pSession; }
	LPMDB GetMessageStore() { return m_pMsgStore; }
	LPMAPIFOLDER GetFolder() { return m_pFolder; }

	BOOL Login(LPCTSTR szProfileName=NULL);
	void Logout();

	LPCTSTR GetProfileName();
	BOOL OpenMessageStore(LPCTSTR szStore=NULL);
	LPMAPIFOLDER OpenInbox();
	LPMAPIFOLDER OpenOutbox();

	LPMAPITABLE GetHierarchy(LPMAPIFOLDER pFolder=NULL);
	LPMAPIFOLDER GetNextSubFolder(CString& strFolder,LPMAPIFOLDER pFolder=NULL);
	LPMAPIFOLDER OpenSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder=NULL);
	LPMAPIFOLDER CreateSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder=NULL);
	BOOL DeleteSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder=NULL);
	BOOL DeleteSubFolder(LPMAPIFOLDER pSubFolder,LPMAPIFOLDER pFolder=NULL);

	BOOL GetContents(LPMAPIFOLDER pFolder=NULL);
	BOOL GetNextMessage(CMAPIMessage& message,BOOL bUnreadOnly=FALSE);

	BOOL DeleteMessage(CMAPIMessage& message,LPMAPIFOLDER pFolder=NULL);
	BOOL CopyMessage(CMAPIMessage& message,LPMAPIFOLDER pFolderDest,LPMAPIFOLDER pFolderSrc=NULL);
	BOOL MoveMessage(CMAPIMessage& message,LPMAPIFOLDER pFolderDest,LPMAPIFOLDER pFolderSrc=NULL);

	BOOL SendMessage(CMAPIMessage& message);
};

/////////////////////////////////////////////////////////////
// CMAPIMessage

#define MSGFLAG_TAGGED ((ULONG)0x00000400)

class AFX_EXT_CLASS CMAPIMessage : public CObject
{
public:
	CMAPIMessage();
	CMAPIMessage(CMAPIMessage& message);
	~CMAPIMessage();

// Attributes
protected:
	CString m_strSenderName;
	CString m_strSenderEmail;
	CString m_strSubject;
	CString m_strBody;
	BOOL m_bAttachments;
	CTime m_tmReceived;

	CMAPIEx* m_pMAPI;
	LPMESSAGE m_pMessage;
	DWORD m_dwMessageFlags;
	SBinary m_entry;

// Operations
public:
	void Tag(BOOL bSet=TRUE) { m_dwMessageFlags=bSet ? m_dwMessageFlags|MSGFLAG_TAGGED : m_dwMessageFlags&~MSGFLAG_TAGGED; }
	BOOL IsTagged() { return (m_dwMessageFlags&MSGFLAG_TAGGED); }
	void SetEntryID(SBinary* pEntry=NULL);
	SBinary* GetEntryID() { return &m_entry; }

	BOOL IsUnread() { return !(m_dwMessageFlags&MSGFLAG_READ); }
	void MarkAsRead(BOOL bRead=TRUE);
	BOOL Open(IMAPISession* pSession,SBinary entry,DWORD dwMessageFlags);
	void Close();
	BOOL Create(CMAPIEx* pMAPI,int nPriority=IMPORTANCE_NORMAL); // IMPORTANCE_HIGH or IMPORTANCE_LOW also valid

	LPCTSTR GetSenderName() { return m_strSenderName; }
	LPCTSTR GetSenderEmail() { return m_strSenderEmail; }
	LPCTSTR GetSubject() { return m_strSubject; }
	LPCTSTR GetBody() { return m_strBody; }
	BOOL HasAttachments() { return m_bAttachments; }
	BOOL SaveAttachments(LPCTSTR szFolder,int nIndex=-1);
	CTime& GetReceivedTime() { return m_tmReceived; }
	LPCTSTR GetReceivedTime(LPCTSTR szFormat=NULL);

	void SetMessageStatus(int nMessageStatus); // used only by WinCE, pass in MSGSTATUS_RECTYPE_SMS to send an SMS
	BOOL AddRecipient(LPCTSTR szEmail,int nType=MAPI_TO);  // MAPI_CC and MAPI_BCC also valid
	void SetSubject(LPCTSTR szSubject);
	void SetSenderName(LPCTSTR szSenderName);
	void SetSenderEmail(LPCTSTR szSenderEmail);
	void SetBody(LPCTSTR szBody);
	BOOL AddAttachment(LPCTSTR szPath,LPCTSTR szName=NULL);
	void SetReadReceipt(BOOL bSet=TRUE,LPCTSTR szReceiverEmail=NULL);
	BOOL Send();

	// does not copy/reference m_pMessage, shallow copy only
	CMAPIMessage& operator=(CMAPIMessage& message);

	// compares entryID only
	BOOL operator==(CMAPIMessage& message);

protected:
	HRESULT GetProperty(ULONG ulProperty, LPSPropValue &prop);

	void FillSenderName();
	void FillSenderEmail();
	void FillSubject();
	void FillBody();
	void FillHasAttachments();
	void FillReceivedTime();

	BOOL SaveAttachment(LPATTACH pAttachment,LPCTSTR szPath);
};

#endif
