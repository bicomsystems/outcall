// SettingsDialingRules.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsDialingRules.h"


// CSettingsDialingRules dialog

IMPLEMENT_DYNAMIC(CSettingsDialingRules, CDialog)
CSettingsDialingRules::CSettingsDialingRules(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDialingRules::IDD, pParent)
{
}

CSettingsDialingRules::~CSettingsDialingRules()
{
}

void CSettingsDialingRules::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_OUTGOING_CONTEXT, m_editContext);
	DDX_Control(pDX, IDC_EDIT_OUTGOING_PREFIX, m_editOutgoingPrefix);
	DDX_Control(pDX, IDC_EDIT_STRIP_OPTIONS, m_editStripOptions);
}


BEGIN_MESSAGE_MAP(CSettingsDialingRules, CDialog)
END_MESSAGE_MAP()


// CSettingsDialingRules message handlers


void CSettingsDialingRules::LoadSettings() {
	m_editContext.SetWindowText(::theApp.GetProfileString("Settings", "OutgoingContext", "default"));
	m_editOutgoingPrefix.SetWindowText(::theApp.GetProfileString("Settings", "OutgoingPrefix", ""));
	m_editStripOptions.SetWindowText(::theApp.GetProfileString("Settings", "StripOptions", ""));
}


void CSettingsDialingRules::SaveSettings() {
	CString value;
    m_editContext.GetWindowText(value);
	if (value=="")
		value="default";
	::theApp.WriteProfileString("Settings", "OutgoingContext", value);

	m_editOutgoingPrefix.GetWindowText(value);		
	::theApp.WriteProfileString("Settings", "OutgoingPrefix", value);

	m_editStripOptions.GetWindowText(value);		
	::theApp.WriteProfileString("Settings", "StripOptions", value);
}


void CSettingsDialingRules::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_OUT_CONTEXT)->SetWindowText(_("Outgoing context") + CString(":"));
	GetDlgItem(IDC_STATIC_OUTGOING_PREFIX)->SetWindowText(_("Outgoing prefix") + CString(":"));
	GetDlgItem(IDC_STATIC_STRIP)->SetWindowText(_("Strip following prefixes from incoming calls' caller id (comma separated for multiple prefixes)") + CString(":"));
	GetDlgItem(IDC_STATIC_GROUP)->SetWindowText(_("Dialing rules"));
}

