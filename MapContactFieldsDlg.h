#pragma once
#include "afxcmn.h"


// CMapContactFieldsDlg dialog

#include <vector>

class CMapContactFieldsDlg : public CDialog
{
	DECLARE_DYNAMIC(CMapContactFieldsDlg)

public:
	CMapContactFieldsDlg(CWnd* pParent, std::vector<int> *field_map, std::vector<CString> *fields);   // standard constructor
	virtual ~CMapContactFieldsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAP_CONTACT_FIELDS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	std::vector<int> *field_map;
	std::vector<CString> *fields;

public:
	virtual BOOL OnInitDialog();
	CListCtrl m_list1;
	CListCtrl m_list2;
	afx_msg void OnBnClickedButtonMapFields();
	afx_msg void OnBnClickedOk();
};
