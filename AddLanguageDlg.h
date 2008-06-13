#pragma once


// CAddLanguageDlg dialog
#include <vector>
#include "afxwin.h"

class CAddLanguageDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddLanguageDlg)

public:
	CAddLanguageDlg(CWnd* pParent, vector<CString>& languages, CString *SelectedLanguage);   // standard constructor
	virtual ~CAddLanguageDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_LANGUAGE };

private:
	vector<CString> *languages;
	CString *SelectedLanguage;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


public:
	virtual BOOL OnInitDialog();
	CListBox m_list;
	afx_msg void OnBnClickedOk();

	void InitLocaleGUI();
};
