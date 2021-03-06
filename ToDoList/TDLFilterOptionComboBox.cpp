// FilterOptionComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TDLFilterOptionComboBox.h"
#include "tdcstatic.h"

#include "..\shared\misc.h"
#include "..\Shared\enstring.h"
#include "..\Shared\localizer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterOptionComboBox

CTDLFilterOptionComboBox::CTDLFilterOptionComboBox() : CCheckComboBox(), m_dwOptions((DWORD)-1)
{
}

CTDLFilterOptionComboBox::~CTDLFilterOptionComboBox()
{
}


BEGIN_MESSAGE_MAP(CTDLFilterOptionComboBox, CCheckComboBox)
	//{{AFX_MSG_MAP(CFilterOptionComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterOptionComboBox message handlers

void CTDLFilterOptionComboBox::Initialize(DWORD dwFlags, DWORD dwOptions)
{
	// translation done via CEnString
	CLocalizer::EnableTranslation(*this, FALSE);

	ResetContent();
	m_dwOptions = (DWORD)-1;

	CEnString sFlag;

	for (int nItem = 0; nItem < NUM_FILTEROPT; nItem++)
	{
		UINT nFlag = FILTER_OPTIONS[nItem][1];

		if (Misc::HasFlag(dwFlags, nFlag))
		{
			sFlag.LoadString(FILTER_OPTIONS[nItem][0]);
			
			int nIndex = AddString(sFlag);
			
			if (nIndex != CB_ERR)
			{
				SetItemData(nIndex, nFlag);

				// is the item checked?
				BOOL bChecked = Misc::HasFlag(dwOptions, nFlag);
				SetCheck(nIndex, (bChecked ? CCBC_CHECKED : CCBC_UNCHECKED));
			}
		}
	}
}

void CTDLFilterOptionComboBox::Initialize(const TDCFILTER& filter, FTC_VIEW /*nView*/, BOOL bWantHideParents)
{
	// translation done via CEnString
	CLocalizer::EnableTranslation(GetSafeHwnd(), FALSE);

	ResetContent();
	m_dwOptions = (DWORD)-1;

	// add flags to droplist depending on the filter being used
	CEnString sFlag;

	for (int nItem = 0; nItem < NUM_FILTEROPT; nItem++)
	{
		UINT nFlag = FILTER_OPTIONS[nItem][1];
		BOOL bAddFlag = FALSE;

		switch (nFlag)
		{
		case FO_ANYCATEGORY:
		case FO_ANYALLOCTO:
		case FO_ANYTAG:
			bAddFlag = ((filter.nShow != FS_ADVANCED) && (filter.nShow != FS_SELECTED));
			break;

		case FO_SHOWALLSUB:
		case FO_HIDECOLLAPSED:
			bAddFlag = TRUE;
			break;

		case FO_HIDEPARENTS:
			bAddFlag = !bWantHideParents;
			break;

		case FO_HIDEOVERDUE:
			bAddFlag = (filter.nDueBy == FD_TODAY) ||
						(filter.nDueBy == FD_YESTERDAY) ||
						(filter.nDueBy == FD_TOMORROW) ||
						(filter.nDueBy == FD_ENDTHISWEEK) || 
						(filter.nDueBy == FD_ENDNEXTWEEK) || 
						(filter.nDueBy == FD_ENDTHISMONTH) ||
						(filter.nDueBy == FD_ENDNEXTMONTH) ||
						(filter.nDueBy == FD_ENDTHISYEAR) ||
						(filter.nDueBy == FD_NEXTNDAYS) ||
						(filter.nDueBy == FD_ENDNEXTYEAR);
			break;

		case FO_HIDEDONE:
			bAddFlag = ((filter.nShow != FS_NOTDONE) && (filter.nShow != FS_DONE));
			break;

		default:
			ASSERT(0);
			break;		
		}	

		if (bAddFlag)
		{
			sFlag.LoadString(FILTER_OPTIONS[nItem][0]);
			
			int nIndex = AddString(sFlag);
			
			if (nIndex != CB_ERR)
				SetItemData(nIndex, nFlag);
		}
	}

	// set states
	SetSelectedOptions(filter.dwFlags);
}

void CTDLFilterOptionComboBox::OnCheckChange(int /*nIndex*/)
{
	// update m_dwOptions
	m_dwOptions = 0;

	for (int nItem = 0; nItem < GetCount(); nItem++)
	{
		// aggregate checked items
		if (GetCheck(nItem))
			m_dwOptions |= GetItemData(nItem);
	}
}

void CTDLFilterOptionComboBox::SetSelectedOptions(DWORD dwOptions)
{
	ASSERT(GetSafeHwnd());

	if (dwOptions == m_dwOptions)
		return;

	for (int nItem = 0; nItem < GetCount(); nItem++)
	{
		// get flag for item
		UINT nFlag = GetItemData(nItem);

		// set state
		BOOL bChecked = Misc::HasFlag(dwOptions, nFlag);
		SetCheck(nItem, (bChecked ? CCBC_CHECKED : CCBC_UNCHECKED));
	}

	m_dwOptions = dwOptions;
}

DWORD CTDLFilterOptionComboBox::GetSelectedOptions() const 
{
	return m_dwOptions;
}

void CTDLFilterOptionComboBox::DrawItemText(CDC& dc, const CRect& rect, int nItem, UINT nItemState,
	DWORD dwItemData, const CString& sItem, BOOL bList, COLORREF crText)
{
	CString sItemText(sItem);

	if (sItemText.IsEmpty() && m_sText.IsEmpty())
		sItemText = CEnString(IDS_TDC_NONE);

	CCheckComboBox::DrawItemText(dc, rect, nItem, nItemState, dwItemData, sItemText, bList, crText);
}
