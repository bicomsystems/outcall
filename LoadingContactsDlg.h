#pragma once


// CLoadingContactsDlg dialog

class CLoadingContactsDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoadingContactsDlg)

public:
	CLoadingContactsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoadingContactsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOADING_CONTACTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
};
