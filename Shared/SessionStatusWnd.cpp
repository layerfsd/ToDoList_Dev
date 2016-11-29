// SessionStatusWnd.cpp : implementation file
//
///////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionStatusWnd.h"
#include "misc.h"

/////////////////////////////////////////////////////////////////////////////

#ifndef WM_WTSSESSION_CHANGE
#	define WM_WTSSESSION_CHANGE	0x02B1
#	define WTS_SESSION_LOCK		0x7
#	define WTS_SESSION_UNLOCK	0x8
#endif

/////////////////////////////////////////////////////////////////////////////

enum 
{
	TIMER_LOCK = 1,
	TIMER_SCREENSAVE,
};
///////////////////////////////////////////////////////////
// CSessionStatusWnd

IMPLEMENT_DYNAMIC(CSessionStatusWnd, CFrameWnd)

CSessionStatusWnd::CSessionStatusWnd()	
	:
	m_hwndNotify(NULL),
	m_bHibernated(FALSE),
	m_bLocked(FALSE),
	m_bScreenSaver(FALSE)
{

}

CSessionStatusWnd::~CSessionStatusWnd()
{
}

///////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSessionStatusWnd, CFrameWnd)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnSessionChange)
	ON_WM_SYSCOMMAND()
	ON_WM_TIMER()
	ON_WM_CREATE()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////

BOOL CSessionStatusWnd::Initialize(HWND hwndNotify)
{
	ASSERT(::IsWindow(hwndNotify));

	if (Create(NULL, NULL/*, WS_OVERLAPPED | WS_POPUP, CRect(0, 0, 0, 0), NULL, NULL*/))
	{
		m_hwndNotify = hwndNotify;
		return TRUE;
	}

	return FALSE;
}

LRESULT CSessionStatusWnd::OnPowerBroadcast(WPARAM wp, LPARAM /*lp*/)
{
	switch (wp)
	{
	case PBT_APMSUSPEND:
	case PBT_APMSTANDBY:
	case PBT_APMQUERYSUSPEND:
	case PBT_APMQUERYSTANDBY:
		m_bHibernated = TRUE;
		Notify(SESSIONSTATUS_HIBERNATE, TRUE);
		break;

	case PBT_APMQUERYSUSPENDFAILED:
	case PBT_APMQUERYSTANDBYFAILED:
	case PBT_APMRESUMECRITICAL:
	case PBT_APMRESUMESUSPEND: 
	case PBT_APMRESUMESTANDBY:
		m_bHibernated = FALSE;
		Notify(SESSIONSTATUS_HIBERNATE, FALSE);
		break;
	}

	return 0L;
}

LRESULT CSessionStatusWnd::OnSessionChange(WPARAM wp, LPARAM /*lp*/)
{
	switch (wp)
	{
	case WTS_SESSION_LOCK:
		m_bLocked = TRUE;
		Notify(SESSIONSTATUS_LOCK, TRUE);
		break;

	case WTS_SESSION_UNLOCK:
		m_bLocked = FALSE;
		Notify(SESSIONSTATUS_LOCK, FALSE);
		break;
	}

	return 0L;
}

void CSessionStatusWnd::Notify(SESSIONSTATUS nStatus, BOOL bOn) const
{
	::SendMessage(m_hwndNotify, WM_SESSIONSTATUS_CHANGE, nStatus, bOn);
}

BOOL CSessionStatusWnd::RegisterForSessionNotification()
{
	typedef BOOL (WINAPI *PFNWTSREGISTERSESSIONNOTIFICATION)(HWND, DWORD);

	// load dll once only
	static HMODULE hDll = LoadLibrary(_T("Wtsapi32.dll"));

	if (hDll)
	{
		static PFNWTSREGISTERSESSIONNOTIFICATION fnRegister = 
			(PFNWTSREGISTERSESSIONNOTIFICATION)GetProcAddress(hDll, "WTSRegisterSessionNotification");

		if (fnRegister)
			return fnRegister(GetSafeHwnd(), 0/*NOTIFY_FOR_THIS_SESSION*/);
	}

	// else
	return FALSE;
}

void CSessionStatusWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID)
	{
	case SC_SCREENSAVE:
		m_bScreenSaver = TRUE;
		SetTimer(TIMER_SCREENSAVE, 30000, NULL);
		Notify(SESSIONSTATUS_SCREENSAVER, FALSE);
		break;
	}

	CFrameWnd::OnSysCommand(nID, lParam);
}

void CSessionStatusWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_SCREENSAVE:
		ASSERT(m_bScreenSaver);
		if (!Misc::IsScreenSaverActive())
		{
			m_bScreenSaver = FALSE;
			KillTimer(TIMER_SCREENSAVE);
			Notify(SESSIONSTATUS_SCREENSAVER, FALSE);
		}
		break;

	case TIMER_LOCK:
		if (!m_bLocked && Misc::IsWorkStationLocked())
		{
			m_bLocked = TRUE;
			Notify(SESSIONSTATUS_LOCK, TRUE);
		}
		else if (m_bLocked && !Misc::IsWorkStationLocked())
		{
			m_bLocked = FALSE;
			Notify(SESSIONSTATUS_LOCK, FALSE);
		}
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}


int CSessionStatusWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!RegisterForSessionNotification())
	{
		// Use a timer for lock/unlock state
		SetTimer(TIMER_LOCK, 30000, NULL);
	}

	return 0;
}
