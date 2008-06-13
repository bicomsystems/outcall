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

// MyLabel.cpp : implementation file
//

#include "stdafx.h"
#include ".\mylabel.h"

BEGIN_MESSAGE_MAP(CMyLabel, CStatic)
	//{{AFX_MSG_MAP(CMyLabel)	
	ON_WM_SETCURSOR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PAINT()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMyLabel::CMyLabel(void)
{
	m_textColor = GetSysColor(COLOR_WINDOWTEXT);	
	m_brush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	m_hCursor =	NULL;
}

CMyLabel::~CMyLabel(void)
{	
	m_font.DeleteObject();
	::DeleteObject(m_brush);
}

void CMyLabel::SetTextColor(COLORREF crText) {
	m_textColor = crText;	
	UpdateLabel();
}

void CMyLabel::SetCursor(HCURSOR hCursor) {
	m_hCursor = hCursor;
}

void CMyLabel::SetFont(LOGFONT lf) {
    CopyMemory(&m_logFont, &lf, sizeof(m_logFont));
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_logFont);	
	UpdateLabel();
}

void CMyLabel::UpdateLabel() {
	CRect (rc);
	GetWindowRect(rc);
	RedrawWindow();

	GetParent()->ScreenToClient(rc);
	GetParent()->InvalidateRect(rc,TRUE);
	GetParent()->UpdateWindow();
}


void CMyLabel::PreSubclassWindow() {
	CStatic::PreSubclassWindow();

	CFont* cf = GetFont();
	if(cf !=NULL) {
		cf->GetObject(sizeof(m_logFont),&m_logFont);
	} else {
		GetObject(GetStockObject(SYSTEM_FONT),sizeof(m_logFont), &m_logFont);
	}

	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_logFont);
}


void CMyLabel::OnPaint() {
	CPaintDC dc(this); // device context for painting

	DWORD dwFlags = DT_LEFT | DT_WORDBREAK;

	CRect rc;
	GetClientRect(rc);
	CString strText;
	GetWindowText(strText);
	CBitmap bmp;
	
	CDC* pDCMem;
	CBitmap*	pOldBitmap = NULL;

	pDCMem = new CDC;
	pDCMem->CreateCompatibleDC(&dc);
	bmp.CreateCompatibleBitmap(&dc,rc.Width(),rc.Height());
	pOldBitmap = pDCMem->SelectObject(&bmp);
	
	UINT nMode = pDCMem->SetBkMode(TRANSPARENT);

	COLORREF crText;
	if (IsWindowEnabled()) {
		crText = pDCMem->SetTextColor(m_textColor);
	} else {
		crText = pDCMem->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}

	CFont *pOldFont = pDCMem->SelectObject(&m_font);

	CBrush br;			
	br.Attach(m_brush);			
	pDCMem->FillRect(rc,&br);

	br.Detach();		

	pDCMem->DrawText(strText,rc,dwFlags);
		
	// Restore DC's State
	pDCMem->SetBkMode(nMode);
	pDCMem->SelectObject(pOldFont);
	pDCMem->SetTextColor(crText);
	
	dc.BitBlt(0,0,rc.Width(),rc.Height(),pDCMem,0,0,SRCCOPY);
	// continue DC restore 
	pDCMem->SelectObject ( pOldBitmap ) ;
	delete pDCMem;	
}


BOOL CMyLabel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
	if (m_hCursor) {
		::SetCursor(m_hCursor);
		return TRUE;
	}
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CMyLabel::OnSysColorChange() {
	if (m_brush)
		::DeleteObject(m_brush);

	m_brush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	
	UpdateLabel();		
}