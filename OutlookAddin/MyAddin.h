/*
 * Copyright (c) 2003-2007, Bicom Systems Ltd.
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

// MyAddin.h: interface for the CMyAddin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYADDIN_H__232F4A79_4123_4F32_B2FD_8460CB64B629__INCLUDED_)
#define AFX_MYADDIN_H__232F4A79_4123_4F32_B2FD_8460CB64B629__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OutlookInterface.h"

using namespace Outlook;

class COutlookButton : public IDispatch 
{
public:
	COutlookButton(CComPtr <Office::CommandBarControl> pButton);
	virtual ~COutlookButton();

	// IUnknown Implementation
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void ** ppv);
    virtual ULONG   __stdcall AddRef(void);
    virtual ULONG   __stdcall Release(void);

    // Methods:

	// IDispatch Implementation
    virtual HRESULT __stdcall GetTypeInfoCount(UINT* pctinfo);
    virtual HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    virtual HRESULT __stdcall GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId);
    virtual HRESULT __stdcall Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);


	BOOL						SetupConnection(IDispatch *pDisp);
	void						ShutDown();

	DWORD								m_dwCookie;
	CComPtr <Office::CommandBarControl> m_pButton;
	IConnectionPoint					*m_pCP;
	ULONG								m_cRef;



	Outlook::_ApplicationPtr 	m_OLAppPtr;
	IDispatchPtr GetSelectedItem(Outlook::_ExplorerPtr spExplorer);	

private:
	IDispatchPtr COutlookButton::FindContact(_bstr_t query);
    IDispatchPtr COutlookButton::FindContact(MAPIFolderPtr pFolder, _bstr_t query);

};


//#import "C:\Program Files\Common Files\designer\MSADDNDR.dll" raw_interfaces_only , raw_native_types, no_namespace, named_guids

class CMyAddin : public IExchExt, IExchExtCommands //, IDTExtensibility2 //, IExchExtSessionEvents
{
public:
   CMyAddin();
   ~CMyAddin();


   STDMETHODIMP_(ULONG)	AddRef();
   STDMETHODIMP_(ULONG)	Release(); 
   STDMETHODIMP		QueryInterface(REFIID	riid,
				      void**	ppvObj);
   STDMETHODIMP		Install(IExchExtCallback	*lpExchangeCallback,
			        ULONG		context,
			        ULONG		ulFlags);


   STDMETHODIMP_(VOID) InitMenu(LPEXCHEXTCALLBACK lpeecb);

   STDMETHODIMP InstallCommands(LPEXCHEXTCALLBACK lpeecb, HWND hwnd, HMENU hmenu, UINT FAR * lpcmdidBase, LPTBENTRY lptbeArray, UINT ctbe, ULONG ulFlags) {
		return S_OK;
   }
   STDMETHODIMP DoCommand(LPEXCHEXTCALLBACK lpeecb, UINT cmdid) {
	   if (cmdid == SC_CLOSE) {
			SaveToolbarSettings();
	   }	  	   
	   
	   return S_FALSE;
   }
   STDMETHODIMP Help(LPEXCHEXTCALLBACK lpeecb, UINT cmdid) {
       return S_FALSE;
   }
   STDMETHODIMP QueryHelpText(UINT cmdid,ULONG ulFlags, LPTSTR lpsz,UINT cch)  {
       return S_FALSE;
   }
   STDMETHODIMP QueryButtonInfo(ULONG tbid,UINT itbb,  LPTBBUTTON ptbb,  LPTSTR lpsz,  UINT cch,  ULONG ulFlags) {
	   return S_FALSE;
   }
   STDMETHODIMP ResetToolbar(ULONG tbid, ULONG ulFlags) {
		
	   return S_OK;
   }


   
   //STDMETHODIMP			OnDelivery(IExchExtCallback				*lpExchangeCallback); 
	
   HRESULT GetOutlookApp(IExchExtCallback 		*pmecb,
		       Outlook::_ApplicationPtr 	&rOLAppPtr);
   HRESULT   InstallInterface(IExchExtCallback   *lpExchangeCallback);
   Outlook::_ApplicationPtr 	m_OLAppPtr;

   COutlookButton	*m_pMyButton;
   COutlookButton	*m_pMyMenuItem;
   COutlookButton	*m_pMyContextMenuItem;
   CComPtr <Office::CommandBar> m_pToolBar;

   IDispatchPtr GetSelectedItem(Outlook::_ExplorerPtr spExplorer);

   CComPtr<Office::CommandBarControl> m_pButton; // Button
   BOOL added;

   ULONG			m_ulRefCount;	// Basic COM stuff, needed to know when it's safe to delete ourselves
   ULONG			m_ulContext;	// Member to store the current context in.
   BOOL interfaceInstalled;

private:
	void LoadToolbarSettings();
	void SaveToolbarSettings();
};

#endif // !defined(AFX_MYADDIN_H__232F4A79_4123_4F32_B2FD_8460CB64B629__INCLUDED_)
