#pragma once
#include "afxwin.h"


// COutlookRuleDlg dialog

class COutlookRuleDlg : public CDialog
{
	DECLARE_DYNAMIC(COutlookRuleDlg)

public:
	COutlookRuleDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COutlookRuleDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_OUTLOOK_RULE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_prefix;
	CEdit m_replacement;

	CListCtrl *list;
	BOOL edit;
	int nSelected;

	void InitLocaleGUI();
	
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
