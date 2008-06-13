// MapContactFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "MapContactFieldsDlg.h"
#include ".\mapcontactfieldsdlg.h"


using namespace std;
// CMapContactFieldsDlg dialog

IMPLEMENT_DYNAMIC(CMapContactFieldsDlg, CDialog)
CMapContactFieldsDlg::CMapContactFieldsDlg(CWnd* pParent, vector<int> *field_map, vector<CString> *fields)
	: CDialog(CMapContactFieldsDlg::IDD, pParent)
{
	this->field_map = field_map;
	this->fields = fields;
}

CMapContactFieldsDlg::~CMapContactFieldsDlg()
{
}

void CMapContactFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list1);
	DDX_Control(pDX, IDC_LIST_OUTLOOK_FIELDS, m_list2);
}


BEGIN_MESSAGE_MAP(CMapContactFieldsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MAP_FIELDS, OnBnClickedButtonMapFields)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CMapContactFieldsDlg message handlers

BOOL CMapContactFieldsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rect;
	m_list1.GetClientRect(&rect);

	m_list1.InsertColumn(0, _("Field"), LVCFMT_LEFT, rect.right*0.45);
	m_list1.InsertColumn(1, _("Mapped from"), LVCFMT_LEFT, rect.right*0.45);	

    m_list2.InsertColumn(1, _("Value"), LVCFMT_LEFT, rect.right*0.95);
	
	m_list1.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list2.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	field_map->push_back(0);
	field_map->push_back(1);
	field_map->push_back(2);
	field_map->push_back(3);
	field_map->push_back(4);
	field_map->push_back(5);

	field_map->push_back(29);
	field_map->push_back(31);
	field_map->push_back(32);
	field_map->push_back(30);

	field_map->push_back(33);
	field_map->push_back(34);
	field_map->push_back(35);

	field_map->push_back(37);
	field_map->push_back(38);
	field_map->push_back(36);

	field_map->push_back(39);
	field_map->push_back(40);
	field_map->push_back(42);
	field_map->push_back(41);

	field_map->push_back(43);
	field_map->push_back(44);
	field_map->push_back(45);
	field_map->push_back(47);
	field_map->push_back(46);    

	CString temp;
	int nItem;

	for (int num=0; num<field_map->size(); num++) {
		if (num==0)
			temp = "Title";
		else if (num==1)
			temp = "First Name";
		else if (num==2)
			temp = "Middle Name";
		else if (num==3)
			temp = "Last Name";
		else if (num==4)
			temp = "Suffix";
		else if (num==5)
			temp = "Company";
		else if (num==6)
			temp = "Assistant";
		else if (num==7)
			temp = "Business";
		else if (num==8)
			temp = "Business 2";
		else if (num==9)
			temp = "Business Fax";
		else if (num==10)
			temp = "Callback";
		else if (num==11)
			temp = "Car";
		else if (num==12)
			temp = "Company";
		else if (num==13)
			temp = "Home";
		else if (num==14)
			temp = "Home 2";
		else if (num==15)
			temp = "Home Fax";
		else if (num==16)
			temp = "ISDN";
		else if (num==17)
			temp = "Mobile";
		else if (num==18)
			temp = "Other";
		else if (num==19)
			temp = "Other Fax";
		else if (num==20)
			temp = "Pager";
		else if (num==21)
			temp = "Primary";
		else if (num==22)
			temp = "Radio";
		else if (num==23)
			temp = "Telex";
		else if (num==24)
			temp = "TTY/TDD";
        
		nItem = m_list1.InsertItem(num, temp);
		if ((*field_map)[num]>(fields->size()-1)) {
            (*field_map)[num] = -1;
			m_list1.SetItem(nItem, 1, LVIF_TEXT, _T(""), 0, 0, 0, 0);
		} else {			
			m_list1.SetItem(nItem, 1, LVIF_TEXT, (*fields)[(*field_map)[num]], 0, 0, 0, 0);			
		}
		m_list1.SetItemData(nItem, (DWORD_PTR)(*field_map)[num]);
	}


	for (int i=0; i<fields->size(); i++) {
		m_list2.InsertItem(i, (*fields)[i]);        
	}

	SetWindowText(_("Map custom fields"));
	GetDlgItem(IDC_STATIC_OUTCALL_FIELDS)->SetWindowText(_("Mapped fields:"));
	GetDlgItem(IDC_STATIC_OUTLOOK_FIELDS)->SetWindowText(_("Outlook fields:"));
	GetDlgItem(IDC_BUTTON_MAP_FIELDS)->SetWindowText(_("&Map selected fields"));

	GetDlgItem(IDOK)->SetWindowText(_("&OK"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMapContactFieldsDlg::OnBnClickedButtonMapFields()
{
	int nItem1 = m_list1.GetNextItem(-1, LVNI_SELECTED);
	int nItem2 = m_list2.GetNextItem(-1, LVNI_SELECTED);

	if (nItem1==-1 || nItem2==-1) {
		MessageBox(_("Please select items on both left and right list."), APP_NAME, MB_ICONINFORMATION);
		return;
	}

    m_list1.SetItemData(nItem1, nItem2);    
	m_list1.SetItem(nItem1, 1, LVIF_TEXT, (*fields)[nItem2], 0, 0, 0, 0);
}

void CMapContactFieldsDlg::OnBnClickedOk()
{
	for (int i=0; i<field_map->size(); i++) {
		(*field_map)[i] = (int)m_list1.GetItemData(i);
	}

	OnOK();
}
