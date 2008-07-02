////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// File: MAPIEx.cpp
// Description: Windows Extended MAPI class 
//
// Copyright (C) 2005-2006, Noel Dillabough
//
// This source code is free to use and modify provided this notice remains intact and that any enhancements
// or bug fixes are posted to the CodeProject page hosting this class for the community to benefit.
//
// Usage: see the Codeproject article at http://www.codeproject.com/internet/CMapiEx.asp
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MAPIEx.h"
#include <MapiUtil.h>
#include "..\PBXClient.h"

#include "CPPSqlite3.h"

#include <edk.h>

using namespace std;

#ifdef UNICODE
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
}

CMAPIEx::~CMAPIEx()
{
	Logout();
}

BOOL CMAPIEx::Init(BOOL bMultiThreadedNotifcations)
{
	if(MAPIInitialize(NULL)!=S_OK) {
		return FALSE;
	} else {
		return TRUE;
    }
}

void CMAPIEx::Term()
{
	MAPIUninitialize();
#ifdef _WIN32_WCE
	CoUninitialize();
#endif
}

BOOL CMAPIEx::Login()
{
	CString profile = ::theApp.GetProfileString("Settings", "MailProfile", "");
	HRESULT hRes;	

	/*vector<CString> vProfiles;
	int nDefault = -1;
	LoadProfileDetails(vProfiles, nDefault, false);

	if (profile=="") {
		if (vProfiles.size()>0) {
			if (nDefault!=-1) {
				profile = vProfiles[nDefault];
			} else {
				profile = vProfiles[0];
			}
		}
	} else {
		bool bFound = false;
		for (int i=0; i < vProfiles.size(); i++) {
			if (profile==vProfiles[i]) {
				bFound = true;
				break;
			}
		}
		if (bFound==false) {
			profile = "";
		}
	}*/

	if (profile=="") {
		DBG_LOG("Using system default Mail profile");
	} else {
		CStringA msg = "Using Mail profile: " + profile;
		DBG_LOG(msg);
	}

	//// logon to MAPI for extracting the address book. This can also be used to open the individual mail boxes.
	if (profile!="") {
        TCHAR prof[100];
        _tcscpy(prof, profile);
#ifdef _UNICODE
		hRes = MAPILogonEx(NULL, prof, NULL, MAPI_EXTENDED |
		MAPI_NEW_SESSION | MAPI_LOGON_UI | MAPI_EXPLICIT_PROFILE | MAPI_UNICODE,&m_pSession);
#else
        hRes = MAPILogonEx(NULL, prof, NULL, MAPI_EXTENDED |
		MAPI_NEW_SESSION | MAPI_LOGON_UI | MAPI_EXPLICIT_PROFILE,&m_pSession);
#endif
	} else {
		hRes = MAPILogonEx (NULL, NULL, NULL, MAPI_EXTENDED | MAPI_NEW_SESSION |
		MAPI_USE_DEFAULT ,&m_pSession);
	}

	return (hRes==S_OK);
}

void CMAPIEx::Logout()
{
	RELEASE(m_pContents);
	RELEASE(m_pFolder);
	RELEASE(m_pMsgStore);
	RELEASE(m_pSession);
}

BOOL CMAPIEx::OpenMessageStore(LPCTSTR szStore,ULONG ulFlags)
{
	if(!m_pSession) return FALSE;

	m_ulMDBFlags=ulFlags;

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
						CString strStore=GetValidString(pRows->aRow[0].lpProps[0]);
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

LPMAPIFOLDER CMAPIEx::OpenFolder(unsigned long ulFolderID,BOOL bInternal)
{
	if(!m_pMsgStore) return NULL;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	DWORD dwObjType;
	ULONG rgTags[]={ 1, ulFolderID };
	LPMAPIFOLDER pFolder;

	if(m_pMsgStore->GetProps((LPSPropTagArray) rgTags, cm_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, m_ulMDBFlags, &dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(props);

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenSpecialFolder(unsigned long ulFolderID,BOOL bInternal)
{
	LPMAPIFOLDER pInbox=OpenInbox(FALSE);
	if(!pInbox || !m_pMsgStore) return FALSE;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	DWORD dwObjType;
	ULONG rgTags[]={ 1, ulFolderID };
	LPMAPIFOLDER pFolder;

	if(pInbox->GetProps((LPSPropTagArray) rgTags, cm_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, m_ulMDBFlags, &dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(props);
	RELEASE(pInbox);

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenInbox(BOOL bInternal)
{
	if(!m_pMsgStore) return NULL;

#ifdef _WIN32_WCE
	return OpenFolder(PR_CE_IPM_INBOX_ENTRYID);
#else
	ULONG cbEntryID;
	LPENTRYID pEntryID;
	DWORD dwObjType;
	LPMAPIFOLDER pFolder;

	if(m_pMsgStore->GetReceiveFolder(NULL,0,&cbEntryID,&pEntryID,NULL)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(cbEntryID,pEntryID, NULL, m_ulMDBFlags,&dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(pEntryID);
#endif

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenContacts(BOOL bInternal)
{
	return OpenSpecialFolder(PR_IPM_CONTACT_ENTRYID,bInternal);
}

BOOL CMAPIEx::GetContents(LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}
	RELEASE(m_pContents);
	if(pFolder->GetContentsTable(CMAPIEx::cm_nMAPICode,&m_pContents)!=S_OK) return FALSE;

	const int nProperties=MESSAGE_COLS;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_MESSAGE_FLAGS, PR_ENTRYID }};
	return (m_pContents->SetColumns((LPSPropTagArray)&Columns,0)==S_OK);
}

int CMAPIEx::GetRowCount()
{
	ULONG ulCount;
	if(!m_pContents || m_pContents->GetRowCount(0,&ulCount)!=S_OK) return -1;
	return ulCount;
}

BOOL CMAPIEx::SortContents(ULONG ulSortParam,ULONG ulSortField)
{
	if(!m_pContents) return FALSE;

	SizedSSortOrderSet(1, SortColums) = {1, 0, 0, {{ulSortField,ulSortParam}}};
	return (m_pContents->SortTable((LPSSortOrderSet)&SortColums,0)==S_OK);
}

BOOL CMAPIEx::GetNextContact(CMAPIContact& contact)
{
	if(!m_pContents) return FALSE;

	LPSRowSet pRows=NULL;
	BOOL bResult=FALSE;
	while(m_pContents->QueryRows(1,0,&pRows)==S_OK) {
		if(pRows->cRows) bResult=contact.Open(this,pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin);
		FreeProws(pRows);
		break;
	}
	MAPIFreeBuffer(pRows);
	return bResult;
}

// sometimes the string in prop is invalid, causing unexpected crashes
LPCTSTR CMAPIEx::GetValidString(SPropValue& prop)
{
	LPCTSTR s=prop.Value.LPSZ;
	if(s && !::IsBadStringPtr(s,(UINT_PTR)-1)) return s;
	return NULL;
}

// special case of GetValidString to take the narrow string in UNICODE
void CMAPIEx::GetNarrowString(SPropValue& prop,CString& strNarrow)
{
	LPCTSTR s=GetValidString(prop);
	if(!s) strNarrow=_T("");
	else {
#ifdef UNICODE
		// VS2005 can copy directly
		if(_MSC_VER>=1400) {
			strNarrow=(char*)s;
		} else {
			WCHAR wszWide[256];
			MultiByteToWideChar(CP_ACP,0,(LPCSTR)s,-1,wszWide,255);
			strNarrow=wszWide;
		}
#else
		strNarrow=s;
#endif
	}
}


void CMAPIEx::ReplaceNumber(CString &number) {
	CString prefix;
	for (int i=0; i<m_prefixes.size(); i++) {
		prefix = m_prefixes[i];
		if (number.Left(prefix.GetLength())==prefix) {
			number = m_replacements[i] + number.Mid(prefix.GetLength());
			break;
		}
	}
	number.Replace(_T("-"), _T(""));
	number.Replace(_T("("), _T(""));
	number.Replace(_T(")"), _T(""));
	number.Replace(_T(" "), _T(""));
}


HRESULT CMAPIEx::LoadExchangeContacts(CppSQLite3DB *db) {
	LPMAPISESSION 	pSession    = m_pSession;
	LPADRBOOK 	  	m_pAddrBook = NULL;

	ULONG       cbeid    = 0L;
	LPENTRYID   lpeid    = NULL;
	HRESULT     hRes     = S_OK;
	LPSRowSet   pRow;
	ULONG       ulObjType;
	ULONG       cRows    = 0L;

	LPMAPITABLE    	 lpContentsTable = NULL;
	LPMAPICONTAINER  lpGAL = NULL;

	LPSPropValue lpDN = NULL;

	CStringA title, name, middleName, surname, suffix, fullName, company;
	CStringA query;	
	CString strTemp;

	if (pSession==NULL) {
		DBG_LOG("Failed to load Exhcange contacts. MAPI Session is NULL.");		
		return S_FALSE;
	}

	SizedSPropTagArray ( 23, sptCols ) = {
	23,
	PR_ENTRYID,
	PR_DISPLAY_NAME,
	PR_GIVEN_NAME,
	PR_SURNAME,
	PR_COMPANY_NAME,
	PR_ASSISTANT_TELEPHONE_NUMBER,
	PR_BUSINESS_TELEPHONE_NUMBER,
	PR_BUSINESS2_TELEPHONE_NUMBER,
	PR_BUSINESS_FAX_NUMBER,
	PR_CALLBACK_TELEPHONE_NUMBER,
	PR_CAR_TELEPHONE_NUMBER,
	PR_COMPANY_MAIN_PHONE_NUMBER,
	PR_HOME_TELEPHONE_NUMBER,
	PR_HOME2_TELEPHONE_NUMBER,
	PR_HOME_FAX_NUMBER,
	PR_ISDN_NUMBER,
	PR_MOBILE_TELEPHONE_NUMBER,
	PR_OTHER_TELEPHONE_NUMBER,
	PR_PAGER_TELEPHONE_NUMBER,
	PR_PRIMARY_TELEPHONE_NUMBER,
	PR_RADIO_TELEPHONE_NUMBER,
	PR_TELEX_NUMBER,
	PR_TTYTDD_PHONE_NUMBER
	};		

	hRes = pSession->OpenAddressBook(NULL,NULL,AB_NO_DIALOG,&m_pAddrBook);
	if (FAILED (hRes))
	{
		if (m_pAddrBook) {
			m_pAddrBook->Release();
		}
		CStringA msg;
		msg.Format("Failed to open address book. Error: %d", GetLastError());
		DBG_LOG(msg);
		return hRes;
	}

	if ( FAILED ( hRes = HrFindExchangeGlobalAddressList ( m_pAddrBook,
				&cbeid,
				&lpeid ) ) )
	{
		CStringA msg;
		msg.Format("Failed to find Exchange Global Address List. Error: %d", GetLastError());
		DBG_LOG(msg);		
		goto Quit;
	}

	if(FAILED(hRes = m_pAddrBook->OpenEntry((ULONG) cbeid,
					(LPENTRYID) lpeid,
					NULL,
					MAPI_BEST_ACCESS, // | MAPI_NO_CACHE,
					&ulObjType,
					(LPUNKNOWN *)&lpGAL)))
	{
		CStringA msg;
		msg.Format("Failed to open Global Address List. Error: %d", GetLastError());
		DBG_LOG(msg);
		goto Quit;
	}

	if ( ulObjType != MAPI_ABCONT ) {
		DBG_LOG("Failed: ulObjType != MAPI_ABCONT.");
		goto Quit;
	}

	if (FAILED(hRes = lpGAL->GetContentsTable(0L, &lpContentsTable))) {
		CStringA msg;
		msg.Format("Get Contents Table failed. Error: %d", GetLastError());
		DBG_LOG(msg);		
		goto Quit;
	}

	if (FAILED(hRes = lpContentsTable->SetColumns((LPSPropTagArray)&sptCols, TBL_BATCH))) {
		CStringA msg;
		msg.Format("Set Columns failed. Error: %d", GetLastError());
		DBG_LOG(msg);
		goto Quit;
	}

	if ( FAILED ( hRes = lpContentsTable -> GetRowCount ( 0, &cRows ) ) ) {
		CStringA msg;
		msg.Format("Get Row Count failed. Error: %d", GetLastError());
		DBG_LOG(msg);
		goto Quit;
	}

	if ( FAILED ( hRes = lpContentsTable -> QueryRows ( cRows, 0L, &pRow ))) {        
		CStringA msg;
		msg.Format("QueryRows failed. Error: %d", GetLastError());
		DBG_LOG(msg);
		goto Quit;
	}

	try {
		for(int x=0; (x < pRow->cRows) && (x<5) && !m_bStop; x++)
		{		
			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues,
			PR_DISPLAY_NAME);
			lpDN != NULL ? fullName = lpDN->Value.lpszA : fullName="";

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues,
			PR_GIVEN_NAME);
			lpDN != NULL ? name = lpDN->Value.lpszA : name="";

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues,
			PR_SURNAME);
			lpDN != NULL ? surname = lpDN->Value.lpszA : surname="";

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues,
			PR_COMPANY_NAME);
			lpDN != NULL ? company = lpDN->Value.lpszA : company="";

			name.Replace("'", "''");
			surname.Replace("'", "''");
			company.Replace("'", "''");
			fullName.Replace("'", "''");

			if (name=="" && surname=="") {
				name = fullName;
				surname = "";
			}

			query = "INSERT INTO Contacts VALUES ('" + title + "', '" + name + "', '" + middleName + "', '" + surname + "', '" + suffix + "', '" + company + "'";

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_ASSISTANT_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
            ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_BUSINESS_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_BUSINESS2_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_BUSINESS_FAX_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_CALLBACK_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_CAR_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_COMPANY_MAIN_PHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_HOME_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_HOME2_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_HOME_FAX_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_ISDN_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_MOBILE_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_OTHER_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_PRIMARY_FAX_NUMBER); //PR_OTHER_FAX_PHONE_NUMBER
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");			

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_PAGER_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_PRIMARY_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_RADIO_TELEPHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_TELEX_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "'");

			lpDN = PpropFindProp(pRow->aRow[x].lpProps,
			pRow->aRow[x].cValues, PR_TTYTDD_PHONE_NUMBER);
			lpDN != NULL ? strTemp = lpDN->Value.lpszA : strTemp="";
			ReplaceNumber(strTemp);
			query += (", '" + strTemp + "')");

			db->execDML(query.GetBuffer());		
		}
	} catch (CppSQLite3Exception& e) {
		DBG_LOG(e.errorMessage());		
	}


	Quit:
	if ( NULL != lpGAL)
	{
		lpGAL -> Release ( );
		lpGAL = NULL;
	}

	if ( lpContentsTable )
	{
		lpContentsTable -> Release ( );
		lpContentsTable = NULL;
	}

	m_pAddrBook->Release();
	
	return hRes;
}


void CMAPIEx::LoadOutlookContacts() {
	m_bStop = false;

	std::vector<CString> rules;
	m_prefixes.clear();
	m_replacements.clear();	

	SplitString(::theApp.GetProfileString("Settings", "ImportOutlookRules", ""), " || ", rules);
	for (int i=0; i<rules.size(); i++) {
        if ((i%2)==0)
			m_prefixes.push_back(rules[i]);
		else
			m_replacements.push_back(rules[i]);
	}


	DBG_LOG("------------------ LOAD OUTLOOK CONTACTS BEGIN ------------------");

	if (OpenContacts() && GetContents()) {
		// sort by name (stored in PR_SUBJECT)
		SortContents(TABLE_SORT_ASCEND,PR_SUBJECT);

		CMAPIContact contact;

		CppSQLite3DB db;
		db.open(CStringA(OUTCALL_DB));

		CString title, name, middleName, surname, suffix, fullName, company, strTemp;
		CString query;

		try {

			db.execDML("DELETE FROM Contacts");
			db.execDML("begin transaction");

			while (GetNextContact(contact) && !m_bStop) {				

				contact.GetPropertyString(title, PR_TITLE);
				contact.GetPropertyString(name, PR_GIVEN_NAME);
				contact.GetPropertyString(middleName, PR_MIDDLE_NAME);
				contact.GetPropertyString(surname, PR_SURNAME);
				contact.GetPropertyString(company, PR_MIDDLE_NAME);
				contact.GetName(fullName);				

				title.Replace(_T("'"), _T("''"));
				name.Replace(_T("'"), _T("''"));
				middleName.Replace(_T("'"), _T("''"));
				surname.Replace(_T("'"), _T("''"));
				suffix.Replace(_T("'"), _T("''"));
				company.Replace(_T("'"), _T("''"));
				fullName.Replace(_T("'"), _T("''"));

				query = "INSERT INTO Contacts VALUES ('" + title + "', '" + name + "', '" + middleName + "', '" + surname + "', '" + suffix + "', '" + company + "'";
				
				contact.GetPhoneNumber(strTemp, PR_ASSISTANT_TELEPHONE_NUMBER);				
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");				

				contact.GetPhoneNumber(strTemp, PR_BUSINESS_TELEPHONE_NUMBER);				
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");				

				contact.GetPhoneNumber(strTemp, PR_BUSINESS2_TELEPHONE_NUMBER);				
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");				

				contact.GetPhoneNumber(strTemp, PR_BUSINESS_FAX_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");				

				contact.GetPhoneNumber(strTemp, PR_CALLBACK_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_CAR_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_COMPANY_MAIN_PHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_HOME_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_HOME2_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_HOME_FAX_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_ISDN_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_MOBILE_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_OTHER_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_PRIMARY_FAX_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_PAGER_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_PRIMARY_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_RADIO_TELEPHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_TELEX_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "'");
				
				contact.GetPhoneNumber(strTemp, PR_TTYTDD_PHONE_NUMBER);
				ReplaceNumber(strTemp);
				query += (", '" + strTemp + "')");				
				
				db.execDML(query.GetBuffer());
			}

			if (!m_bStop) {
				DBG_LOG("------------------ LOAD EXCHANGE CONTACTS BEGIN ------------------");
                HRESULT hRes = LoadExchangeContacts(&db);
				if (hRes!=S_OK) {
					CStringA msg;
					msg.Format("Failed to load Exchange contacts. Error code: 0x%x", hRes);
                    DBG_LOG(msg);
				}
				DBG_LOG("------------------ LOAD EXCHANGE CONTACTS END ------------------");
			}

			db.execDML("end transaction");

		} catch (CppSQLite3Exception& e) {
			DBG_LOG(e.errorMessage());
			db.execDML("end transaction");
		}

		RELEASE(m_pContents);
		RELEASE(m_pFolder);
	} else {
		DBG_LOG("Failed to open Outlook Contacts folder");
	}	

	DBG_LOG("------------------ LOAD OUTLOOK CONTACTS END ------------------");
}

void CMAPIEx::StopLoadingContacts() {
	m_bStop=true;
}

// if I try to use MAPI_UNICODE when UNICODE is defined I get the MAPI_E_BAD_CHARWIDTH 
// error so I force narrow strings here
LPCTSTR CMAPIEx::GetProfileName(IMAPISession* pSession)
{
	if(!pSession) return NULL;

	static CString strProfileName;
	LPSRowSet pRows=NULL;
	const int nProperties=2;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME_A, PR_RESOURCE_TYPE}};

	IMAPITable*	pStatusTable;
	if(pSession->GetStatusTable(0,&pStatusTable)==S_OK) {
		if(pStatusTable->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) {
			while(TRUE) {
				if(pStatusTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
				else if(pRows->cRows!=1) FreeProws(pRows);
				else if(pRows->aRow[0].lpProps[1].Value.ul==MAPI_SUBSYSTEM) {
					strProfileName=(LPSTR)GetValidString(pRows->aRow[0].lpProps[0]);
					FreeProws(pRows);
					continue;
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




HRESULT CMAPIEx::LoadProfileDetails(vector<CString> &vProfiles/*, int &nDefault, bool bInitializeMAPI*/)
{
	HRESULT       hr;
	
	vProfiles.clear();

	hr = MAPIInitialize(NULL);
	if (FAILED(hr)) {
		DBG_LOG("Failed to initialize MAPI.");
		return hr;
	}

//#ifdef _UNICODE
	IMAPISession* pSession;	
    hr = MAPILogonEx(NULL, NULL, NULL, MAPI_EXTENDED | MAPI_NEW_SESSION | MAPI_USE_DEFAULT ,&pSession);
	if (hr==S_OK) {
		CString ProfileName = GetProfileName(pSession);
		vProfiles.push_back(ProfileName);
		RELEASE(pSession);
	} else {
        DBG_LOG("MAPILogonEx Failed.");		
	}	

	MAPIUninitialize();	

//#endif
//	
//	LPMAPITABLE   pTable = NULL;
//	LPSRowSet     pRows = NULL;
//	bool          found=false;
//	SizedSPropTagArray(2, Columns) = {2, {PR_DISPLAY_NAME, PR_DEFAULT_PROFILE}};
//	LPPROFADMIN pProfAdmin = NULL; // Pointer to IProfAdmin object
//	LPSPropValue lpDN = NULL;
//
//	/*if (true)
//		return S_OK;*/
//
//	nDefault=-1;
//
//	if (bInitializeMAPI) {
//    	hr = MAPIInitialize(NULL);
//		if (FAILED(hr)) {
//			return hr;
//		}
//	}
//
//	CString ProfileName;
//
//	vProfiles.clear();
//
//	hr = MAPIAdminProfiles(0, &pProfAdmin); 
//    if (!FAILED(hr)) 
//    { 
//          // Get the ProfileTable - Contains all profiles 
//          hr = pProfAdmin->GetProfileTable(0, &pTable); 
//          if (FAILED(hr)) 
//		  {
//			   //Error initializing profile table
//			   return hr;
//          } 
//	}
//
//	hr = HrQueryAllRows(pTable,      //pointer to table of pointers
//		 (LPSPropTagArray) &Columns, //list of columns we will get
//		 NULL,                       //filter NULL. We need all rows
//		 NULL,                       //we don’t need any sorting here
//		 0,                          //retrieve all rows that match
//		 &pRows);                    //pointer to resulting table
//
//
//	if (!FAILED(hr) && pRows->cRows>0) //result of HrQueryAllRows
//	{
//		for(int i=0;i<pRows->cRows;i++)
//		{			
//			ProfileName = pRows->aRow[i].lpProps->Value.lpszA;			
//
//			lpDN = PpropFindProp(pRows->aRow[i].lpProps,
//			pRows->aRow[i].cValues, PR_DEFAULT_PROFILE);
//			if (lpDN!=NULL) {
//				if (lpDN->Value.b!=0) {
//					nDefault = i;                    
//				}               
//			}
//			vProfiles.push_back(ProfileName);
//		}
//	} 
//
//	//Cleanup
//	if (pRows) FreeProws(pRows);
//	if (pTable) pTable->Release();
//	if (pProfAdmin) pProfAdmin->Release();
//
//	if (bInitializeMAPI) {
//		MAPIUninitialize();
//	}
//#endif

	return S_OK;
}

