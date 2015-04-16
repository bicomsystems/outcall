/*
 * Copyright (c) 2003-2010, Bicom Systems Ltd.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *	   this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *   - Neither the name of the Bicom Systems Ltd nor the names of its
 *	   contributors may be used to endorse or promote products derived from this
 *	   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Denis Komadaric,
 * Bicom Systems Ltd.
 */

#include "stdafx.h"
#include "MyAddin.h"

//#include "MAPIEx.h"

#include <vector>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


COutlookButton::COutlookButton(CComPtr <Office::CommandBarControl> pButton)
{
	m_cRef		= 0;
	m_dwCookie	= 0;
	m_pButton	= NULL;
	m_pCP		= NULL;
	try
	{
		if (SetupConnection(pButton))
		{
			m_pButton = pButton;
		}
	}
	catch(...)
	{
	}
}


COutlookButton::~COutlookButton()
{
	ShutDown();
}

STDMETHODIMP_(ULONG) COutlookButton::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) COutlookButton::Release(void)
{
	if( m_cRef > 0 )
	{
    	if (0 != --m_cRef)
    		return m_cRef;
	    delete this;
	}
    return 0;
}

STDMETHODIMP COutlookButton::QueryInterface(REFIID riid, void** ppv)
{
	if (NULL == ppv) 
		return E_POINTER;
	*ppv = NULL;
	
	HRESULT hr = S_OK;

	if ((__uuidof(Office::_CommandBarButtonEvents) == riid) ||
        (IID_IUnknown == riid) || 
		(IID_IDispatch == riid))
		*ppv = static_cast<IDispatch*>(this);
	else
		hr = E_NOINTERFACE;

	if (NULL != *ppv)
		reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return hr;
}

STDMETHODIMP COutlookButton::GetTypeInfoCount(UINT* pctinfo)
{
    if (NULL == pctinfo) 
		return E_POINTER;
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP COutlookButton::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    if (NULL == ppTInfo) 
		return E_POINTER;
    *ppTInfo = NULL;
    return E_FAIL;
}

STDMETHODIMP COutlookButton::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
    if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;

    if (NULL == rgDispId) 
		return E_POINTER;
    if (NULL == rgszNames) 
		return E_POINTER;
    if (cNames != 1) 
		return E_INVALIDARG;
    *rgDispId = 0x00000000;
    return S_OK;
}


IDispatchPtr COutlookButton::FindContact(_bstr_t query) {
	std::vector<MAPIFolderPtr> loaded_folders;
	_ContactItemPtr pContact = NULL;
			
	try {
				
		_FoldersPtr folders = m_OLAppPtr->GetNamespace(_bstr_t("MAPI"))->GetFolders();
		MAPIFolderPtr pFolder = folders->GetFirst(); //Personal folders (root folder)		
		bool bFound;
	
		while (pFolder!=NULL) {
			bFound = false;
			for (int i=0; i<loaded_folders.size(); i++) {
				if (loaded_folders[i]==pFolder) {                        
					bFound=true;
					break;
				}
			}
			if (bFound) {
				pFolder = folders->GetNext();
			} else {					
				pContact = FindContact(pFolder, query);
				if (pContact)
					return pContact;
				pFolder = folders->GetNext();
				loaded_folders.push_back(pFolder);
			}
		}				 		
	}
	catch(_com_error &e)
	{
		MessageBox(NULL, (char *)e.Description(), APP_NAME, MB_ICONERROR);
	}
	/*finally
	{
		MessageBox(NULL, "error", APP_NAME, MB_ICONERROR);
	}*/

	return pContact;
}


IDispatchPtr COutlookButton::FindContact(MAPIFolderPtr pFolder, _bstr_t query) {	
	_ItemsPtr pItems;	
	_ContactItemPtr pContact;
	_FoldersPtr folders;
		
	if (pFolder->GetDefaultItemType()==olContactItem) {

		pItems=pFolder->GetItems();
		if (pItems) {
			pContact = pItems->Find(query);
			if (pContact) {
				return pContact;
			}
		}
	}

			
	folders = pFolder->GetFolders();
	pFolder = folders->GetFirst();
	while (pFolder!=NULL) {
		pContact = FindContact(pFolder, query);
		if (pContact)
			return pContact;
		pFolder = folders->GetNext();
	}

	return NULL;
}


STDMETHODIMP COutlookButton::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;

	// The only dispID supported is 1, which triggers when the "button" is clicked
    if (dispIdMember != 0x00000001)
        return DISP_E_MEMBERNOTFOUND;

    if (NULL == pDispParams) 
		return E_POINTER;
    if (pDispParams->cArgs != 2) 
		return DISP_E_BADPARAMCOUNT;

	try {
		if( pDispParams->rgvarg[1].vt == VT_DISPATCH ) 
		{
			Outlook::_ExplorerPtr spExplorer = m_OLAppPtr->ActiveExplorer();
            _ContactItemPtr pContactItem = NULL;
			CString fullName="";

			if (spExplorer!=NULL) {			
				Outlook::_NameSpacePtr nameSpace = m_OLAppPtr->GetNamespace("MAPI");
				Outlook::MAPIFolderPtr currFolder = spExplorer->CurrentFolder;
				HRESULT hr;
				
				enum Outlook::OlItemType olt;
				hr = currFolder->get_DefaultItemType(&olt);
				if (!FAILED(hr)) {
					if (olt==Outlook::olContactItem) {
						IDispatchPtr pContact = GetSelectedItem(spExplorer);
						if (pContact!=NULL) {
							pContactItem = (Outlook::_ContactItemPtr)pContact;
							fullName = (LPCTSTR)pContactItem->GetFullName();							
						}					
					} else if (olt==(Outlook::OlItemType)0) {
						Outlook::MAPIFolderPtr pParentFolder = (Outlook::MAPIFolderPtr)currFolder->GetParent();

						if (pParentFolder!=NULL) {							
							IDispatchPtr pSelectedItem = GetSelectedItem(spExplorer);
							Outlook::_MailItemPtr pMailItem;
							//Outlook::_ContactItemPtr pContactItem;												
							Outlook::MAPIFolderPtr pTemp, pTemp2;
							BOOL itemType = 0; // 0-other, 1-mail, 2-contact

							if (pSelectedItem!=NULL) {
								
								pMailItem = (Outlook::_MailItemPtr)pSelectedItem;
								itemType = 1;
								
								/*HRESULT hr = pSelectedItem->QueryInterface(&pMailItem);

								if (FAILED(hr)) {
									hr = pSelectedItem->QueryInterface(&pContactItem);
									if (!FAILED(hr))
										itemType = 2;							
								} else {
									itemType = 1;
								}*/

								if (itemType==1) { //mail item
									if ((pTemp=pParentFolder->GetParent())==NULL) {						
										pParentFolder = currFolder;
									} else {										
										while (1) {
											pTemp=(Outlook::MAPIFolderPtr)pParentFolder->GetParent();
											
											if ((pTemp2=pTemp->GetParent())!=NULL)
												pParentFolder=pTemp;
											else
												break;
										}
									}

									if ((pParentFolder->GetName()==nameSpace->GetDefaultFolder(Outlook::olFolderOutbox)->GetName()) ||
										(pParentFolder->GetName()==nameSpace->GetDefaultFolder(Outlook::olFolderSentMail)->GetName()) ||
										(pParentFolder->GetName()==nameSpace->GetDefaultFolder(Outlook::olFolderDrafts)->GetName())) {
											fullName = (LPCTSTR)pMailItem->GetTo();
									} else if (pParentFolder->GetName()==nameSpace->GetDefaultFolder(Outlook::olFolderInbox)->GetName()) {										
										fullName = (LPCTSTR)pMailItem->GetSenderName();
									} else {
										fullName = (LPCTSTR)pMailItem->GetSenderName();										
										/*fullName += "#####";
										fullName += (LPCTSTR)pMailItem->GetTo();*/
									}
																		
									pContactItem = FindContact(CString(CString("[FullName] = '") + fullName + "'").AllocSysString());
									
								} else if (itemType==2) { //contact item
									fullName = (LPCTSTR)pContactItem->GetFullName();
								}
							}							
						}
					}				
				}
			}

			CString numbers;			
			CString strTemp;
			if (pContactItem) {
                strTemp = (LPCTSTR)pContactItem->GetAssistantTelephoneNumber();
				if (strTemp!="")
					numbers += "Assistant:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetBusinessTelephoneNumber();				
				if (strTemp!="")
					numbers += "Business:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetBusiness2TelephoneNumber();					
				if (strTemp!="")
					numbers += "Business2:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetBusinessFaxNumber();					
				if (strTemp!="")
					numbers += "Business Fax:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetCallbackTelephoneNumber();					
				if (strTemp!="")
					numbers += "Callback:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetCarTelephoneNumber();					
				if (strTemp!="")
					numbers += "Car:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetCompanyMainTelephoneNumber();
				if (strTemp!="")
					numbers += "Company Main:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetHomeTelephoneNumber();					
				if (strTemp!="")
					numbers += "Home:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetHome2TelephoneNumber();					
				if (strTemp!="")
					numbers += "Home2:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetHomeFaxNumber();
				if (strTemp!="")
					numbers += "Home Fax:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetISDNNumber();
				if (strTemp!="")
					numbers += "ISDN:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetMobileTelephoneNumber();					
				if (strTemp!="")
					numbers += "Mobile:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetOtherTelephoneNumber();					
				if (strTemp!="")
					numbers += "Other:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetOtherFaxNumber();
				if (strTemp!="")
					numbers += "Other Fax:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetPagerNumber();
				if (strTemp!="")
					numbers += "Pager:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetPrimaryTelephoneNumber();					
				if (strTemp!="")
					numbers += "Primary:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetRadioTelephoneNumber();
				if (strTemp!="")
					numbers += "Radio:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetTelexNumber();
				if (strTemp!="")
					numbers += "Telex:" + strTemp + " | ";

				strTemp = (LPCTSTR)pContactItem->GetTTYTDDTelephoneNumber();
				if (strTemp!="")
					numbers += "TTY/TDD:" + strTemp + " | ";
			}

			// We can now get at the "button" which triggered this event.
			LPDISPATCH pButton = pDispParams->rgvarg[1].pdispVal;
			
			CString strWindowTitle = "BSOC Main Application Window";
			CString strDataToSend;
			
			CString ContactName;
			if (pContactItem) {
				fullName = (LPCTSTR)pContactItem->GetFullName();
				if (fullName!="") {
					ContactName = CString((LPCTSTR)pContactItem->GetFirstName()) + " | " + 
					CString((LPCTSTR)pContactItem->GetMiddleName()) + " | " +
					CString((LPCTSTR)pContactItem->GetLastName());
				} else {
					ContactName = (LPCTSTR)pContactItem->GetCompanyName();
				}
			} else {
                ContactName = fullName;
			}
			strDataToSend = _T("Dial#####" + ContactName + "#####" + numbers);
			

	    
			LRESULT copyDataResult;			
			CWnd *pOtherWnd = CWnd::FindWindow(NULL, strWindowTitle);

			if (pOtherWnd)
			{
				COPYDATASTRUCT cpd;
				cpd.dwData = 0;
				cpd.cbData = strDataToSend.GetLength();
				cpd.lpData = (void*)strDataToSend.GetBuffer(cpd.cbData);
				copyDataResult = pOtherWnd->SendMessage(WM_COPYDATA,
														(WPARAM)NULL,
														(LPARAM)&cpd);				
				// copyDataResult has value returned by other app				
			} 
			else 
			{
				AfxMessageBox(CString(APP_NAME) + " is not running.");
			}	

		}
	} catch (...) { }

	/*mapi.Logout();
	CMAPIEx::Term();*/

    return S_OK;
}

BOOL COutlookButton::SetupConnection(IDispatch *pDisp)
{
	IConnectionPointContainer *pCPC = 0;
	HRESULT hr = S_OK;

	try
	{
		// Set up the connection and call Advise.
		if (FAILED((hr=pDisp->QueryInterface(IID_IConnectionPointContainer,(void **)&pCPC)))) 
			return FALSE;

		if (FAILED((hr=pCPC->FindConnectionPoint(__uuidof(Office::_CommandBarButtonEvents),&m_pCP)))) 
		{
			pCPC->Release();
			return FALSE;
		}
		if (FAILED((hr=m_pCP->Advise(static_cast<IDispatch*>(this), &m_dwCookie)))) 
		{
			pCPC->Release();
			return FALSE;
		}

		pCPC->Release();
	}
	catch(...)
	{
	}
	return TRUE;
}

void COutlookButton::ShutDown()
{
	try
	{
		
		m_pCP->Unadvise(m_dwCookie);
		m_pCP->Release();		
	}
	catch(...)
	{
	}
}



IDispatchPtr COutlookButton::GetSelectedItem(Outlook::_ExplorerPtr spExplorer)
{
	//HRESULT hr;
	IDispatchPtr olSelectedItem = NULL;	// Since you don't know what type to expect, just use an IDispatch*

	COleVariant covOptional((long) DISP_E_PARAMNOTFOUND, VT_ERROR);
 
	//CComPtr<Outlook::_Explorer> spExplorer; 
	CComPtr<Outlook::Selection> olSel;
	

	//Outlook::_ExplorerPtr spExplorer = m_OLAppPtr->ActiveExplorer();
	//m_OLAppPtr->ActiveExplorer(&spExplorer);
	// Get the Selection object
	olSel = spExplorer->GetSelection();
	//hr = spExplorer->get_Selection(&olSel);
	if (olSel==NULL)
		return NULL;

	/*if (FAILED(hr))
		return NULL;*/

	// Since more than one object can be selected at any one time, you loop through them
	// one at a time
	long count;
	olSel->get_Count(&count);

	if (count<=0)
		return NULL;

	COleVariant covIndex;
	covIndex.vt = VT_I4;
	covIndex.lVal = 1;

	/*_variant_t vv;
	vv.lVal=1;
	vv.vt=VT_I4;*/

	olSelectedItem = olSel->Item(covIndex);

	/*for (long i = 1; i <= count; ++i){
		
		//olSel->Item(covIndex,&olSelectedItem);	// Get the selected item
	}*/

	return olSelectedItem;
}
