#pragma once
#include "afxwin.h"


// CSettingsDialingRules dialog

class CSettingsDialingRules : public CDialog
{
	DECLARE_DYNAMIC(CSettingsDialingRules)

public:
	CSettingsDialingRules(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsDialingRules();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_DIALING_RULES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_editContext;	
	CEdit m_editOutgoingPrefix;
	CEdit m_editStripOptions;

	void LoadSettings();
	void SaveSettings();

	void InitLocaleGUI();
};
