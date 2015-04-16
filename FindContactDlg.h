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

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CFindContactDlg dialog

class CFindContactDlg : public CDialog
{
	DECLARE_DYNAMIC(CFindContactDlg)

public:
	CFindContactDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFindContactDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FIND_CONTACT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:	
	CListCtrl m_list;
	afx_msg void OnBnClickedDial();	
	afx_msg void OnBnClickedCancel();
	CEdit m_editQuery;
	virtual BOOL OnInitDialog();
	CComboBox m_DialFrom;

private:
	void LoadExtensions();
	void Search();

	LRESULT OnEnableDialButton(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnHdnItemdblclickListResults(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListResults(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditSearch();
	CEdit m_phoneNumber;
	afx_msg void OnNMClickListResults(NMHDR *pNMHDR, LRESULT *pResult);

	void InitLocaleGUI();
	CButton m_btnDial;
};
