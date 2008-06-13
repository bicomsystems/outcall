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

// MCListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "MCListCtrl.h"

#include <string>

#include "CPPSQLite3.h"


// CMCListCtrl

IMPLEMENT_DYNAMIC(CMCListCtrl, CListCtrl)
CMCListCtrl::CMCListCtrl()
{	
	query = "select * from MissedCalls";
	updateQuery = "update MissedCalls set NewCall=0";

	sort_asc = false;
	sort_column = 2;
    	
}

CMCListCtrl::~CMCListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMCListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHdnItemclick)
END_MESSAGE_MAP()


// CMCListCtrl message handlers
// OnCustomDraw
void CMCListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.

	if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		// This is the notification message for an item.  We'll request
		// notifications before each subitem's prepaint stage.

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		// This is the prepaint stage for a subitem. Here's where we set the
		// item's text and background colors. Our return value will tell
		// Windows to draw the subitem itself, but it will use the new colors
		// we set here.

		int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
		int nSubItem = pLVCD->iSubItem;
		
		if (GetItemData(nItem)!=0) { //new call
			pLVCD->clrText = RGB(255, 0, 0);
			//pLVCD->clrTextBk = crBkgnd;
		}
	}
}


void CMCListCtrl::Init() {
	RECT rect;
	GetClientRect(&rect);

	InsertColumn(0, _("From"), LVCFMT_LEFT, rect.right*0.35);
	InsertColumn(1, _("To"), LVCFMT_LEFT, rect.right*0.25);
	InsertColumn(2, _("Date"), LVCFMT_LEFT, rect.right*0.40);
	
	LONG style = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
	style &= ~LVS_SORTDESCENDING;
	style &= ~LVS_SORTASCENDING;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, style);
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);	
}


void CMCListCtrl::UpdateList() {	
	CppSQLite3DB db;	    
	db.open(CStringA(OUTCALL_DB));

	DeleteAllItems();
	
	ListContents = 0;
	CStringA query = this->query;

	if (sort_column==0) {
		if (sort_asc)
			query += " order by CallerID asc";
		else
            query += " order by CallerID desc";
	} else if (sort_column==1) {
		if (sort_asc)
			query += " order by Callee asc";
		else
            query += " order by Callee desc";
	} else if (sort_column==2) {
		if (sort_asc)
			query += " order by Sec asc";
		else
            query += " order by Sec desc";
	}
	sort_asc = !sort_asc;

	try {
		CppSQLite3Query q = db.execQuery(query.GetBuffer());

		std::string callerID, callee, sTime, sDate;
		int nItem;
		long newCall;
		
		while (!q.eof())
		{				
			newCall = q.getIntField(0);
			sDate = q.fieldValue(1);
			sTime = q.fieldValue(2);
			callee = q.fieldValue(3);
			callerID = q.fieldValue(4);		

			nItem = InsertItem(GetItemCount(), CString(callerID.c_str()));
			if ((newCall==1) || (newCall==2))
				SetItemData(nItem, newCall);
			else
				SetItemData(nItem, 0);
			
			SetItem(nItem, 1, LVIF_TEXT, CString(callee.c_str()), 0, 0, 0, 0);
			SetItem(nItem, 2, LVIF_TEXT, CString(sDate.c_str()) + ", " + CString(sTime.c_str()), 0, 0, 0, 0);		
			q.nextRow();
		}
	} catch (CppSQLite3Exception& e) { }
}

void CMCListCtrl::UpdatePlacedRecivedList() {
	CppSQLite3DB db;	    
	db.open(CStringA(OUTCALL_DB));

	DeleteAllItems();

	ListContents = 1;
	CStringA query = this->query;

	if (sort_column==0) {
		if (sort_asc)
			query += " order by CallerID asc";
		else
            query += " order by CallerID desc";
	} else if (sort_column==1) {
		if (sort_asc)
			query += " order by Callee asc";
		else
            query += " order by Callee desc";
	} else if (sort_column==2) {
		if (sort_asc)
			query += " order by Sec asc";
		else
            query += " order by Sec desc";
	}
	sort_asc = !sort_asc;

	try {
		CppSQLite3Query q = db.execQuery(query.GetBuffer());
		
		std::string callerID, callee, sTime, sDate;
		int nItem;	
		
		while (!q.eof())
		{				
			sDate = q.fieldValue(0);
			sTime = q.fieldValue(1);
			callee = q.fieldValue(2);
			callerID = q.fieldValue(3);		

			nItem = InsertItem(GetItemCount(), CString(callerID.c_str()));
			SetItemData(nItem, 0);
			
			SetItem(nItem, 1, LVIF_TEXT, CString(callee.c_str()), 0, 0, 0, 0);
			SetItem(nItem, 2, LVIF_TEXT, CString(sDate.c_str()) + ", " + CString(sTime.c_str()), 0, 0, 0, 0);		
			q.nextRow();
		}
	} catch (CppSQLite3Exception& e) { }
}


void CMCListCtrl::OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here

	if (phdr->iItem!=sort_column)
		sort_asc=false;
		
	sort_column = phdr->iItem;

	if (ListContents==0)
		UpdateList();
	else
		UpdatePlacedRecivedList();	

	*pResult = 0;
}
