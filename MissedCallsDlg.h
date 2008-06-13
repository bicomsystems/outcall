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

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "MCListCtrl.h"


// CMissedCallsDlg dialog

class CMissedCallsDlg : public CDialog
{
	DECLARE_DYNAMIC(CMissedCallsDlg)

public:
	CMissedCallsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMissedCallsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MISSED_CALLS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cboPhone;
	CComboBox m_cboDate;
	CMCListCtrl m_list;
	virtual BOOL OnInitDialog();

private:
	void LoadExtensions();
	void UpdateList();
	LRESULT OnMsgRefresh(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnLvnOdstatechangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeComboPhone();
	afx_msg void OnCbnSelchangeComboDate();
	afx_msg void OnBnClickedButtonCall();	
	afx_msg void OnBnClickedButtonRemove();
protected:
	virtual void OnCancel();

public:
	afx_msg void OnBnClickedOk();
	CComboBox m_cboShow;
	CStatic m_frame;
	afx_msg void OnCbnSelchangeComboShow();
	CButton m_btnAddContact;
	afx_msg void OnBnClickedButtonAddContact();

	void InitLocaleGUI();
};
