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

// MyAddin.cpp: implementation of the CMyAddin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutlookAddin.h"
#include "MyAddin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyAddin::CMyAddin()
{
	m_ulRefCount = 0;
	m_ulContext  = 0;
	m_pMyButton  = NULL;
	m_pMyMenuItem = NULL;
	m_pMyContextMenuItem = NULL;
	interfaceInstalled=FALSE;

	added=FALSE;
}

CMyAddin::~CMyAddin()
{
}


STDMETHODIMP CMyAddin::QueryInterface(REFIID					riid,
									   void**					ppvObj)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*ppvObj=NULL;
	IUnknown* punk=NULL;
	if (riid == IID_IUnknown)
	{
		punk = (IExchExt*)this;
	}
	else if (riid == IID_IExchExt)
	{
		punk = (IExchExt*)this;
	}
	/*else if (riid == IID_IExchExtSessionEvents)
	{
		punk = (IExchExtSessionEvents*)this;
	}*/
	else if (riid == IID_IExchExtCommands)
	{
		punk = (IExchExtCommands*)this;
	}
	else
	{
		return E_NOINTERFACE;
	}
	if (NULL!=punk)
	{
		*ppvObj = punk;
		AddRef();
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) CMyAddin::AddRef()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_ulRefCount++;
	return m_ulRefCount;
}
STDMETHODIMP_(ULONG) CMyAddin::Release()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	

	ULONG ulCount = m_ulRefCount--;
	if (!ulCount)
	{
		delete this;
	}
	return ulCount;
}


STDMETHODIMP CMyAddin::Install(IExchExtCallback	*lpExchangeCallback,
			     ULONG		mcontext,
 			     ULONG		ulFlags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hRet = S_FALSE;
	try
	{
		m_ulContext = mcontext;
		GetOutlookApp(lpExchangeCallback, m_OLAppPtr);
		switch(m_ulContext)
		{
			case EECONTEXT_TASK:
				{
					hRet = S_OK;
					//InstallInterface(lpExchangeCallback);
				}
				break;
			case EECONTEXT_SESSION:
				{
					hRet = S_OK;
					//InstallInterface(lpExchangeCallback);
				}
				break;
			case EECONTEXT_VIEWER:
				{
					hRet = S_OK;
					InstallInterface(lpExchangeCallback);
					/*if (!interfaceInstalled)
						InstallInterface(lpExchangeCallback);					
					interfaceInstalled=TRUE;*/
				}
				break;
			case EECONTEXT_REMOTEVIEWER:
			case EECONTEXT_SENDNOTEMESSAGE:
			case EECONTEXT_SENDPOSTMESSAGE:
			case EECONTEXT_SEARCHVIEWER:
			case EECONTEXT_ADDRBOOK:		
			case EECONTEXT_READNOTEMESSAGE:		
			case EECONTEXT_READPOSTMESSAGE:
			case EECONTEXT_READREPORTMESSAGE:
			case EECONTEXT_SENDRESENDMESSAGE:
			case EECONTEXT_PROPERTYSHEETS:
			case EECONTEXT_ADVANCEDCRITERIA:
			default:
				{
					hRet = S_FALSE;
				}
				break;

		};
	}
	catch(...)
	{
	}
	return hRet;
}

HRESULT CMyAddin::GetOutlookApp(IExchExtCallback 		*pmecb,
			     Outlook::_ApplicationPtr 	&rOLAppPtr)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   try
   {
	  IOutlookExtCallback *pOutlook = NULL;
      HRESULT hRes = pmecb->QueryInterface(IID_IOutlookExtCallback,(void **) &pOutlook);
      if (pOutlook)
      {
         IUnknown *pUnk = NULL;
         pOutlook->GetObject(&pUnk);
         LPDISPATCH lpMyDispatch;
         if (pUnk != NULL)
         {
            hRes = pUnk->QueryInterface(IID_IDispatch, (void **) &lpMyDispatch);
			pUnk->Release();
         }
         if (lpMyDispatch)
         {
            OLECHAR * szApplication = L"Application";
			DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
			DISPID dspid;
			VARIANT vtResult;
			lpMyDispatch->GetIDsOfNames(IID_NULL, &szApplication, 1, LOCALE_SYSTEM_DEFAULT, &dspid);
			lpMyDispatch->Invoke(dspid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparamsNoArgs, &vtResult, NULL, NULL);
			lpMyDispatch->Release();

			rOLAppPtr= vtResult.pdispVal;
            return S_OK;
         }
      }
   }
   catch(...)
   {
   }
   return S_FALSE;
}


HRESULT CMyAddin::InstallInterface(IExchExtCallback		*lpExchangeCallback)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	try
	{
		CString csTitle = APP_NAME; // This will be the display-name of the toolbar
		HBITMAP hBmp;

		// First we need to get the active Explorer, you could see this as the active window (it's more to it but let's keep it at that for now)
		Outlook::_ExplorerPtr spExplorer = m_OLAppPtr->ActiveExplorer();
		if (spExplorer == NULL)
		{
			return S_FALSE;
		}

		// Then we need to have a pointer to the commandbars, that is the toolbars in the UI
		Office::_CommandBarsPtr spCmdBars = spExplorer->CommandBars;
		if (spCmdBars == NULL)
		{
			return S_FALSE;
		}

		try 
		{ 
			CComVariant vName(csTitle); // This is the caption of the toolbar
			CComVariant vTemp(TRUE);	
			CComPtr <Office::CommandBar> spToolbar;	
			
			CComVariant vPos(1); 
			//CComVariant vTemp(VARIANT_TRUE); 		
			CComVariant vEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);

			/*spToolbar = spCmdBars->Add(vName,		// Caption
									   vtMissing,   // <ignore this for now>
									   vtMissing,   // <ignore this for now>
									   vTemp);      // The toolbar should be temporary, 
													// that way we have to add it every time we*/
			//spToolbar = spCmdBars->Add(vName,		// Caption
			//						   vPos,   // <ignore this for now>
			//						   vEmpty,   // <ignore this for now>
			//						   vTemp);      // The toolbar should be temporary, 
													// that way we have to add it every time we

			CComPtr <Office::CommandBarControl>  pButton	  = NULL; 
			CComPtr <Office::CommandBarControls> pBarControls = NULL; 

			//pBarControls = spToolbar->GetControls();

			/*m_pToolBar = (Office::CommandBarControlPtr)(pBarControls->Add((long)Office::msoBarFloating, //msoControlButton, 
																	   vtMissing, 
																	   vtMissing, 
																	   vtMissing, 
																	   vTemp));*/			
			
			// get the outlok Context Menu. for this enumerate all the command bars for "Context Menu" command bar			
			CComPtr<Office::CommandBar> pTempBar;
			for(long i = 0; i < spCmdBars->Count ; i++)
			{			
				CComVariant vItem(i+1);		 //zero based index
				pTempBar=NULL;
				spCmdBars->get_Item(vItem,&pTempBar);				
				if((pTempBar) && (!wcscmp(CString(APP_NAME).AllocSysString(),pTempBar->Name)))
				{
					return S_FALSE;
				}		
			}



			m_pToolBar = (Office::CommandBarPtr)(spCmdBars->Add(APP_NAME, vPos, /*Office::msoBarFloating,*/ vtMissing, vTemp));
			pButton = (Office::CommandBarControlPtr)m_pToolBar->Controls->Add(Office::msoControlButton,
																	  vtMissing, vtMissing, vtMissing, vTemp);

			pButton->Caption  = APP_NAME;
			pButton->OnAction = "MyButtonsAction";

			BOOL bSetImage = TRUE;
			// I've set this to FALSE, but if you want an image to appear on the button, 
			// you will need to have an image in the resources.
			// The only drawback of this is that there is only one way of 
			// putting an image on the button, and that is by going through the 
			// clipboard. And by doing so, the contents of the clipboard will be
			// lost, unless you implement some logic to store the clipboard-data
			// before doing this and then restoring the data once the image is on 
			// the button.
			if (bSetImage)
			{
				UINT uiBitmapId = 0;
				CComQIPtr <Office::_CommandBarButton> spCmdButton(pButton);

				// to set a bitmap to a button, load a 32x32 bitmap
				// and copy it to clipboard. Call CommandBarButton's PasteFace()
				// to copy the bitmap to the button face. to use
				// Outlook's set of predefined bitmap, set button's FaceId to 	
				// the button whose bitmap you want to use
				
				hBmp =(HBITMAP)::LoadImage(AfxGetInstanceHandle(),
												   MAKEINTRESOURCE(IDB_BITMAP_PHONE),
												   IMAGE_BITMAP,
												   0,
												   0,
												   LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);

				if (hBmp)
				{					
					// put bitmap into Clipboard
					::OpenClipboard(NULL);
					::EmptyClipboard();
					::SetClipboardData(CF_BITMAP, (HANDLE)hBmp);
					::CloseClipboard();
					::DeleteObject(hBmp);	

					// set style before setting bitmap
					spCmdButton->PutStyle(Office::msoButtonIconAndCaption);
					if (FAILED(spCmdButton->PasteFace()))
					{
						TRACE("Failed to paste face to button\n");
					}

					
				}/* else {
					char serr[50];
					DWORD err = GetLastError();
					ltoa(err, serr, 10);
					MessageBox(NULL, serr, "error", MB_OK);
				}*/
			}
			// With the button physically added to the toolbar, we just need to 
			// create an object of COutlookButton to sink it's events.
			m_pMyButton = new COutlookButton(pButton);
			m_pMyButton->m_OLAppPtr = m_OLAppPtr;

			// The only thing left to do is to make the toolbar visible.
			//spToolbar->PutVisible(VARIANT_TRUE); 
			LoadToolbarSettings();
		} 
		catch(...)
		{
		}


		// Add menu item

		_bstr_t bstrNewMenuText(CString(APP_NAME).AllocSysString());
		CComPtr < Office::CommandBarControls> spCmdCtrls;
		CComPtr < Office::CommandBarControls> spCmdBarCtrls; 
		CComPtr < Office::CommandBarPopup> spCmdPopup;
		CComPtr < Office::CommandBarControl> spCmdCtrl;
		CComPtr < Office::CommandBar> spCmdBar;

		// get CommandBar that is Outlook's main menu
		HRESULT hr = spCmdBars->get_ActiveMenuBar(&spCmdBar); 
		if (FAILED(hr))
			return S_FALSE; //hr;
		// get menu as CommandBarControls 
		spCmdCtrls = spCmdBar->GetControls(); 
		ATLASSERT(spCmdCtrls);

		// we want to add a menu entry to Outlook's 6th(Tools) menu     //item
		CComVariant vItem(5);
		spCmdCtrl= spCmdCtrls->GetItem(vItem);
		ATLASSERT(spCmdCtrl);
        
		IDispatchPtr spDisp;
		spDisp = spCmdCtrl->GetControl(); 
        
		// a CommandBarPopup interface is the actual menu item
		CComQIPtr < Office::CommandBarPopup> ppCmdPopup(spDisp);  
		ATLASSERT(ppCmdPopup);
            
		spCmdBarCtrls = ppCmdPopup->GetControls();
		ATLASSERT(spCmdBarCtrls);
        
		CComVariant vMenuType(1); // type of control - menu
		CComVariant vMenuPos(6);  
		CComVariant vMenuEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComVariant vMenuShow(VARIANT_TRUE); // menu should be visible
		CComVariant vMenuTemp(VARIANT_TRUE); // menu is temporary        
    
        
		CComPtr < Office::CommandBarControl> spNewMenu;
		// now create the actual menu item and add it
		spNewMenu = spCmdBarCtrls->Add(vMenuType, vMenuEmpty, vMenuEmpty, 
										   vMenuEmpty, vMenuTemp); 
		ATLASSERT(spNewMenu);
            
		spNewMenu->PutCaption(bstrNewMenuText);
		spNewMenu->PutEnabled(VARIANT_TRUE);
		spNewMenu->PutVisible(VARIANT_TRUE); 
		spNewMenu->OnAction = "MyButtonsAction";
    
		//we'd like our new menu item to look cool and display
		// an icon. Get menu item  as a CommandBarButton
		CComQIPtr < Office::_CommandBarButton> spCmdMenuButton(spNewMenu);
		ATLASSERT(spCmdMenuButton);
		spCmdMenuButton->PutStyle(Office::msoButtonIconAndCaption);
    
		// we want to use the same toolbar bitmap for menuitem too.
		// we grab the CommandBarButton interface so we can add
		// a bitmap to it through PasteFace().
		spCmdMenuButton->PasteFace(); 


		// Empty whatever we put in the clipboard
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::CloseClipboard();

		// With the button physically added to the toolbar, we just need to 
		// create an object of COutlookButton to sink it's events.
		m_pMyMenuItem = new COutlookButton(spNewMenu);
		m_pMyMenuItem->m_OLAppPtr = m_OLAppPtr;

		// show the menu        
		spNewMenu->PutVisible(VARIANT_TRUE); 
   }
   catch(...)
   {
   }

   return S_FALSE;
}


void CMyAddin::LoadToolbarSettings() {
	CRegKey regKey;
	DWORD top, left, visible, position, rowIndex;

	if (regKey.Open(HKEY_CURRENT_USER, APP_REG_KEY)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		return;
	}

	if (regKey.QueryDWORDValue("Top", top)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		goto end;
	}

	if (regKey.QueryDWORDValue("Left", left)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		goto end;
	}

	if (regKey.QueryDWORDValue("Visible", visible)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		goto end;
	}

	if (regKey.QueryDWORDValue("Position", position)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		goto end;
	}

	if (regKey.QueryDWORDValue("RowIndex", rowIndex)!=0) {
		m_pToolBar->PutVisible(VARIANT_TRUE);
		goto end;
	}

	m_pToolBar->Position = (Office::MsoBarPosition)position;
	m_pToolBar->RowIndex = (int)rowIndex;
	m_pToolBar->Top = (int)top;
	m_pToolBar->Left = (int)left;
	m_pToolBar->Visible = (visible!=0)?VARIANT_TRUE:VARIANT_FALSE;

end:

	regKey.Close();	
}

void CMyAddin::SaveToolbarSettings() {
	CRegKey regKey;
	if (regKey.Create(HKEY_CURRENT_USER, APP_REG_KEY)!=0)
		return;

	regKey.SetDWORDValue("Top", m_pToolBar->Top);	
	regKey.SetDWORDValue("Left", m_pToolBar->Left);
	regKey.SetDWORDValue("Visible", (DWORD)m_pToolBar->Visible);
	regKey.SetDWORDValue("Position", (int)m_pToolBar->Position);
	regKey.SetDWORDValue("RowIndex", m_pToolBar->RowIndex);
	regKey.Close();
}


VOID CMyAddin::InitMenu(LPEXCHEXTCALLBACK lpeecb) {
	try {
		HBITMAP hBmp;
		//_bstr_t bstrNewMenuText(OLESTR(APP_NAME));
		//CComPtr < Office::CommandBarControls> spCmdCtrls;
		//CComPtr < Office::CommandBarControls> spCmdBarCtrls; 
		//CComPtr < Office::CommandBarPopup> spCmdPopup;
		CComPtr <Office::CommandBarControl> spCmdCtrl;
		CComPtr < Office::CommandBar> spCmdBar;
		Outlook::_ExplorerPtr spExplorer = m_OLAppPtr->ActiveExplorer();
		Office::_CommandBarsPtr spCmdBars = spExplorer->CommandBars;
		enum Outlook::OlItemType itemType = spExplorer->CurrentFolder->GetDefaultItemType();
		
		
		if ((itemType!=Outlook::olContactItem) && (itemType!=Outlook::olMailItem)) {
            return;			
		}

		// get the outlok Context Menu. for this enumerate all the command bars for "Context Menu" command bar			
		CComPtr<Office::CommandBar> pTempBar;
		BOOL bFound =false;
		for(long i = 0; i < spCmdBars->Count ; i++)
		{			
			CComVariant vItem(i+1);		 //zero based index
			pTempBar=NULL;
			spCmdBars->get_Item(vItem,&pTempBar);	
			if((pTempBar) && (!wcscmp(L"Context Menu",pTempBar->Name)))
			{
				spCmdBar = pTempBar;
				bFound = true;
				break;
			}		
		}

		if (!bFound)
			return;
		
		
		spCmdCtrl = spCmdBar->FindControl(Office::msoControlButton, vtMissing, APP_NAME);

		if (spCmdCtrl!=NULL) {
			
			/*Office::MsoBarProtection oldProtectionLevel = spCmdBar->Protection;
			spCmdBar->Protection = Office::msoBarNoProtection;*/
			spCmdCtrl->Priority = 1;
			/*spCmdCtrl->PutVisible(VARIANT_TRUE);
			spCmdCtrl->PutTag(APP_NAME);
			spCmdCtrl->OnAction = "MyButtonsAction";*/
			//spCmdBar->Protection = oldProtectionLevel;
			//spCmdBar->Reset();
			return;			
		}
		

		CComPtr < Office::CommandBarControl> spNewMenu;
		// change the commandbar protection to zero
		Office::MsoBarProtection oldProtectionLevel = spCmdBar->Protection;
		spCmdBar->Protection = Office::msoBarNoProtection;


		//set the new item type to ControlButton;
		CComVariant vtType(Office::msoControlButton);
		//add a new item to command bar
		spNewMenu = spCmdBar->Controls->Add(vtType);
		//set a unique Tag that u can be used to find your control in commandbar
		spNewMenu->Tag = APP_NAME;
		//a caption
		spNewMenu->Caption = APP_NAME;					
		// priority (makes sure outlook includes this item in everytime)
		spNewMenu->Priority =1 ;
		// store the new button ID into a variable
		//m_vtId = m_pSortButton ->Id;
		// visible the item
		spNewMenu->Visible = true;

		//set the new item type to ControlButton;
		/*CComVariant vtType(Office::msoControlButton);
		CComVariant vMenuType(1); // type of control - menu
		CComVariant vMenuPos(6);  
		CComVariant vMenuEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComVariant vMenuShow(VARIANT_TRUE); // menu should be visible
		CComVariant vMenuTemp(VARIANT_TRUE); // menu is temporary    

		// now create the actual menu item and add it
		spNewMenu = spCmdBar->Controls->Add(vMenuType, vMenuEmpty, vMenuEmpty, 
										   vMenuEmpty, vMenuTemp); 
		ATLASSERT(spNewMenu);

		spNewMenu->PutCaption(bstrNewMenuText);
		spNewMenu->PutEnabled(VARIANT_TRUE);
		spNewMenu->PutVisible(VARIANT_TRUE);
		spNewMenu->PutTag(APP_NAME);
		spNewMenu->OnAction = "MyButtonsAction";*/
    
		//we'd like our new menu item to look cool and display
		// an icon. Get menu item  as a CommandBarButton
		CComQIPtr <Office::_CommandBarButton> spCmdMenuButton(spNewMenu);
		ATLASSERT(spCmdMenuButton);
		//spCmdMenuButton->PutStyle(Office::msoButtonIconAndCaption);

		hBmp = NULL; /*(HBITMAP)::LoadImage(AfxGetInstanceHandle(),
									MAKEINTRESOURCE(IDB_BITMAP_PHONE),
									IMAGE_BITMAP,
									0,
									0,
									LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);*/

		
		if (hBmp) {
			// put bitmap into Clipboard			
			::OpenClipboard(NULL);
			::EmptyClipboard();
			::SetClipboardData(CF_BITMAP, (HANDLE)hBmp);
			::CloseClipboard();
			::DeleteObject(hBmp);	

			// set style before setting bitmap
			spCmdMenuButton->PutStyle(Office::msoButtonIconAndCaption);
			if (FAILED(spCmdMenuButton->PasteFace())) {
				//MessageBox(NULL, "Error", "Error", MB_OK);
			}

			::OpenClipboard(NULL);
			::EmptyClipboard();
			::CloseClipboard();
		}
    
		spCmdBar->Protection = oldProtectionLevel;		
		
		if (m_pMyContextMenuItem==NULL) {
			m_pMyContextMenuItem = new COutlookButton(spNewMenu);
			m_pMyContextMenuItem->m_OLAppPtr = m_OLAppPtr;
		}

		// show the menu        
		spNewMenu->PutVisible(VARIANT_TRUE);

	} catch(...) { 
        //MessageBox(NULL, "Error", "Error", MB_OK);
	}


	
}

/*HRESULT InstallCommands(LPEXCHEXTCALLBACK lpeecb, HWND hwnd, HMENU hmenu, UINT FAR * lpcmdidBase, LPTBENTRY lptbeArray, UINT ctbe, ULONG ulFlags) {

}*/


IDispatchPtr CMyAddin::GetSelectedItem(Outlook::_ExplorerPtr spExplorer)
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



