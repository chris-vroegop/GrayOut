#pragma once

#include <Windows.h>


#define WM_USER_SHELLICON WM_USER + 1
#define WM_USER_CONTEXT_OPEN WM_USER + 2
#define WM_USER_CONTEXT_EXIT WM_USER + 3
#define WM_USER_EXIT WM_USER + 4

#define IDT_TIMER1 WM_USER + 20


namespace MainDialog
{
	INT_PTR Dlgproc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void EnableNotificationIcon(HWND hwnd);
	void DisableNotificationIcon(HWND hwnd);
}
