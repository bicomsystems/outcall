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

// DebugDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "DebugDlg.h"
#include ".\debugdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg dialog


CDebugDlg::CDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDebugDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
		DDX_Control(pDX, IDC_EDIT_DEBUG_INFO, m_edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDebugDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugDlg)
		ON_COMMAND(IDC_BUTTON_CLEAR, OnCmdClear)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg message handlers

void CDebugDlg::OnCmdClear() {
	m_edit.SetWindowText(_T(""));
}

BOOL CDebugDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	RECT rect;
	this->GetClientRect(&rect);
	int w = rect.right+8;
	int h = rect.bottom+30;
	int left = (GetSystemMetrics(SM_CXSCREEN)-w)/2;	
	int top = (GetSystemMetrics(SM_CYSCREEN)-h)/2;	
	SetWindowPos(&this->wndNoTopMost,left,top,w,h,SWP_HIDEWINDOW); //SWP_NOMOVE|SWP_NOSIZE);

	InitLocaleGUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDebugDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);	

	m_edit.LineScroll(m_edit.GetLineCount());

	// TODO: Add your message handler code here
}


void CDebugDlg::InitLocaleGUI() {
	SetWindowText(_("Debug info"));
	GetDlgItem(IDC_STATIC_DEBUG_INFO)->SetWindowText(CString(_("Debug info")) + ":");
    GetDlgItem(IDC_BUTTON_CLEAR)->SetWindowText(_("&Clear"));
	GetDlgItem(IDOK)->SetWindowText(_("E&xit"));	
}
