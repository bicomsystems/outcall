// AddLanguageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "AddLanguageDlg.h"
#include ".\addlanguagedlg.h"


// CAddLanguageDlg dialog

IMPLEMENT_DYNAMIC(CAddLanguageDlg, CDialog)
CAddLanguageDlg::CAddLanguageDlg(CWnd* pParent /*=NULL*/, vector<CString>& languages, CString *SelectedLanguage)
	: CDialog(CAddLanguageDlg::IDD, pParent)
{	
	this->languages = &languages;
	this->SelectedLanguage = SelectedLanguage;
}

CAddLanguageDlg::~CAddLanguageDlg()
{
}

void CAddLanguageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LANGUAGES, m_list);
}


BEGIN_MESSAGE_MAP(CAddLanguageDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CAddLanguageDlg message handlers

BOOL CAddLanguageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i=0; i<languages->size(); i++) {
		m_list.AddString((*languages)[i]);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAddLanguageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (m_list.GetCurSel()==-1) {
        MessageBox(_("Please select some language in the list."), APP_NAME, MB_OK | MB_ICONINFORMATION);
		return;
	}

	m_list.GetText(m_list.GetCurSel(), *SelectedLanguage);
	
	InitLocaleGUI();

	OnOK();
}


void CAddLanguageDlg::InitLocaleGUI() {
	SetWindowText(_("Add Language"));
	GetDlgItem(IDC_STATIC_SELECT_LANGUAGE)->SetWindowText(_("Select language") + CString(":"));
    GetDlgItem(IDOK)->SetWindowText(_("&Add language"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));	
}
