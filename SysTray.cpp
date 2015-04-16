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
#include "SysTray.h"
#include "MainFrm.h"
#include "PBXClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSysTray::CSysTray()
{
    memset(&m_notifyIconData, 0, sizeof(m_notifyIconData));        
    m_lastBaloonDisplay = 0;
}


void CSysTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR tooltip, HICON hIcon, UINT uID) {
    m_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    m_notifyIconData.hWnd   = pParent->GetSafeHwnd();
    m_notifyIconData.uID    = uID;
    m_notifyIconData.hIcon  = hIcon;
    m_notifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;	

    m_notifyIconData.uCallbackMessage = uCallbackMessage;
    _tcscpy(m_notifyIconData.szTip, tooltip);

	this->pParent = pParent;

	Shell_NotifyIcon(NIM_ADD, &m_notifyIconData);
}

CSysTray::~CSysTray()
{
    m_notifyIconData.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &m_notifyIconData);
}

BOOL CSysTray::SetIcon(UINT nID)
{
    HICON hIcon = AfxGetApp()->LoadIcon(nID);
    
    m_notifyIconData.uFlags = NIF_ICON;
    m_notifyIconData.hIcon = hIcon;

	iconResourceID = nID;

	::theApp.m_bSignedIn = (nID==IDI_ICON_CONNECTED);

    return Shell_NotifyIcon(NIM_MODIFY, &m_notifyIconData);
}

BOOL CSysTray::SetTooltipText(CString tooltipText)
{
    m_notifyIconData.uFlags = NIF_TIP;
	_tcscpy(m_notifyIconData.szTip, tooltipText.GetBuffer());

    return Shell_NotifyIcon(NIM_MODIFY, &m_notifyIconData);
}

void CSysTray::ShowBaloon() {
	time_t now;
	time(&now);
	if ((now-m_lastBaloonDisplay)>=10800) {
		CString tooltip = m_notifyIconData.szTip;
		m_notifyIconData.uFlags = NIF_INFO;
		_tcscpy(m_notifyIconData.szInfo, _T("New updates are available"));
		m_notifyIconData.uTimeout = 5000;
		_tcscpy(m_notifyIconData.szInfoTitle, _T("Updates"));
		m_notifyIconData.dwInfoFlags = NIIF_INFO;
		Shell_NotifyIcon(NIM_MODIFY, &m_notifyIconData);
		m_notifyIconData.uFlags = NIF_TIP;	
		Shell_NotifyIcon(NIM_MODIFY, &m_notifyIconData);
		m_lastBaloonDisplay = now;
	}
}

LRESULT CSysTray::OnSysTrayNotification(WPARAM wParam, LPARAM lParam) 
{
    if (wParam != m_notifyIconData.uID)
        return 0;

    CMenu menu, *pSubMenu;

    if (LOWORD(lParam) == WM_RBUTTONUP)
    {    
        if (!menu.LoadMenu(m_notifyIconData.uID)) 
			return 0;

		pSubMenu = menu.GetSubMenu(0);
        if (!pSubMenu) 
			return 0;

		::theApp.m_pMainWnd->ActivateTopParent();

		::theApp.SetMenuItemText(pSubMenu->GetSubMenu(0), ID_FILE_ONLINEHTML, _("Online HTML"));
		::theApp.SetMenuItemText(pSubMenu->GetSubMenu(0), ID_HELP_ONLINEPDF, _("Online PDF"));
		::theApp.SetMenuItemText(pSubMenu->GetSubMenu(0), ID_HELP_CONTACTSUPPORT, _("Contact Support"));

		::theApp.SetMenuItemText(pSubMenu, 0, _("Help"), true);
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_ABOUT, _("About"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_SETTINGS, _("Settings"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_ASTERISKDEBUGINFO, _("Debug info"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_CALL, _("Place a Call"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_ACTIVECALLS, _("Active Calls"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_MISSEDCALLS, _("Call History"));
				
		::theApp.SetMenuItemText(pSubMenu, 9, _("Import Contacts"), true);
		::theApp.SetMenuItemText(pSubMenu->GetSubMenu(9), ID_REFRESHCONTACTS_IMPORTFROMOUTLOOK, _("From Outlook"));
		::theApp.SetMenuItemText(pSubMenu->GetSubMenu(9), ID_REFRESHCONTACTS_IMPORTFROMCSVFILE, _("From CSV file"));

		::theApp.SetMenuItemText(pSubMenu, ID_FILE_ADDCONTACT, _("Add Contact"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_FIND, _("Find"));
		::theApp.SetMenuItemText(pSubMenu, ID_APP_EXIT, _("Exit"));
		::theApp.SetMenuItemText(pSubMenu, ID_FILE_MISSEDCALLS, _("Call History"));	


		pSubMenu->SetDefaultItem(ID_FILE_CALL);
		
		if (::theApp.m_bDeveloperVersion==0)
			pSubMenu->DeleteMenu(ID_FILE_ASTERISKDEBUGINFO, MF_BYCOMMAND);

        CPoint pos;
        GetCursorPos(&pos);        
        ::TrackPopupMenu(pSubMenu->m_hMenu, 0, pos.x, pos.y, 0, ::theApp.m_pMainWnd->GetSafeHwnd() , NULL);
        menu.DestroyMenu();
    } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {		
		((CMainFrame*)pParent)->OnTrayIconDblClick();
	}

    return 1;
}
