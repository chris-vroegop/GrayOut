#include "MainDialog.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>

#include <string>
#include <sstream>

#include "Utils.h"
#include "ChangeColors.h"
#include "Main.h"
#include "Startup.h"
#include "resource.h"



static HMENU s_contextMenu = 0;
static UINT_PTR s_currentTimer = NULL;
static int s_enablePickerSecs = 0;
static int s_disablePickerSecs = 0;


static void ToggleScheduledEnabled(HWND hDlg, BOOL enabled)
{
	HWND enableLab, enable, disableLab, disable;
	W32_ABORT_IF_EQ(enableLab = GetDlgItem(hDlg, IDC_ENABLE_AT_LABEL), NULL);
	W32_ABORT_IF_EQ(enable = GetDlgItem(hDlg, IDC_ENABLE_AT_PICKER), NULL);
	W32_ABORT_IF_EQ(disableLab = GetDlgItem(hDlg, IDC_DISABLE_AT_LABEL), NULL);
	W32_ABORT_IF_EQ(disable = GetDlgItem(hDlg, IDC_DISABLE_AT_PICKER), NULL);
	EnableWindow(enableLab, enabled);
	EnableWindow(enable, enabled);
	EnableWindow(disableLab, enabled);
	EnableWindow(disable, enabled);
	
	if (!enabled && (s_currentTimer != NULL))
	{
		W32_ABORT_IF_EQ(KillTimer(hDlg, s_currentTimer), FALSE);
		s_currentTimer = NULL;
	}
}


static void ShowContextMenu(HWND hDlg)
{
	W32_ABORT_IF_EQ(SetForegroundWindow(hDlg), FALSE);
	W32_ABORT_IF_EQ(s_contextMenu = CreatePopupMenu(), NULL);
	W32_ABORT_IF_EQ(InsertMenuW(s_contextMenu, -1, MF_BYPOSITION | MF_STRING, WM_USER_CONTEXT_OPEN, L"Open"), FALSE);
	W32_ABORT_IF_EQ(InsertMenuW(s_contextMenu, -1, MF_BYPOSITION | MF_STRING, WM_USER_CONTEXT_EXIT, L"Exit"), FALSE);

	POINT pos;
	W32_ABORT_IF_EQ(GetCursorPos(&pos), FALSE);
	W32_ABORT_IF_EQ(TrackPopupMenu(s_contextMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pos.x, pos.y, 0, hDlg, NULL), FALSE);
}


static void InitDialog(HWND hDlg)
{
	// Set Disabled as default radio button
	SendMessageW(GetDlgItem(hDlg, IDC_STATUS_DISABLED), BM_SETCHECK, BST_CHECKED, 0);

	// Set schedule times to default
	// TODO: Read from registry
	SYSTEMTIME pickerTime;
	pickerTime.wYear = 2000;
	pickerTime.wMonth = 1;
	pickerTime.wDayOfWeek = 6;
	pickerTime.wDay = 1;
	pickerTime.wHour = 22;
	pickerTime.wMinute = 0;
	pickerTime.wSecond = 0;
	pickerTime.wMilliseconds = 0;


	HWND enablePicker, disablePicker;
	W32_ABORT_IF_EQ(enablePicker = GetDlgItem(hDlg, IDC_ENABLE_AT_PICKER), NULL);
	W32_ABORT_IF_EQ(disablePicker = GetDlgItem(hDlg, IDC_DISABLE_AT_PICKER), NULL);
	DateTime_SetSystemtime(enablePicker, GDT_VALID, &pickerTime);
	pickerTime.wHour = 7;
	DateTime_SetSystemtime(disablePicker, GDT_VALID, &pickerTime);

	HWND runAtStartupCheckbox;
	W32_ABORT_IF_EQ(runAtStartupCheckbox = GetDlgItem(hDlg, IDC_RUN_AT_STARTUP), NULL);
	Button_SetCheck(runAtStartupCheckbox, Startup::IsRunningAtStartupWithCorrectPath() ? BST_CHECKED : BST_UNCHECKED);
}


static int inline NumSecondsIntoDay(const SYSTEMTIME& sysTime)
{
	return sysTime.wHour * 3600 + sysTime.wMinute * 60 + sysTime.wSecond;
}


static void TimerProc(HWND hDlg, UINT, UINT_PTR, DWORD)
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	int systemSecs = NumSecondsIntoDay(systemTime);
	int secsToTimer;
	if (s_enablePickerSecs < s_disablePickerSecs)
	{
		if (s_enablePickerSecs < systemSecs && s_disablePickerSecs > systemSecs)
		{
			ChangeColors::EnableGrayscale();
			secsToTimer = s_disablePickerSecs - systemSecs;
		}
		else
		{
			ChangeColors::DisableGrayscale();
			secsToTimer = s_enablePickerSecs - systemSecs;
		}
	}
	else
	{
		if (s_enablePickerSecs > systemSecs && s_disablePickerSecs < systemSecs)
		{
			ChangeColors::DisableGrayscale();
			secsToTimer = s_enablePickerSecs - systemSecs;
		}
		else
		{
			ChangeColors::EnableGrayscale();
			secsToTimer = s_disablePickerSecs - systemSecs;
		}
	}
	W32_ABORT_IF_EQ(s_currentTimer = SetTimer(hDlg, IDT_TIMER1, secsToTimer * 1000 + 100, TimerProc), 0);
}


static void SetupSchedule(HWND hDlg)
{
	if (s_currentTimer != NULL)
	{
		W32_ABORT_IF_EQ(KillTimer(hDlg, s_currentTimer), FALSE);
		s_currentTimer = NULL;
	}

	HWND enablePicker, disablePicker;
	W32_ABORT_IF_EQ(enablePicker = GetDlgItem(hDlg, IDC_ENABLE_AT_PICKER), NULL);
	W32_ABORT_IF_EQ(disablePicker = GetDlgItem(hDlg, IDC_DISABLE_AT_PICKER), NULL);
	SYSTEMTIME enablePickerTime, disablePickerTime;
	DateTime_GetSystemtime(enablePicker, &enablePickerTime);
	DateTime_GetSystemtime(disablePicker, &disablePickerTime);
	s_enablePickerSecs = NumSecondsIntoDay(enablePickerTime);
	s_disablePickerSecs = NumSecondsIntoDay(disablePickerTime);

	if (abs(s_enablePickerSecs - s_disablePickerSecs) < 5)
	{
		return;
	}

	TimerProc(hDlg, NULL, NULL, NULL);

}


INT_PTR MainDialog::Dlgproc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		InitDialog(hDlg);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {

			switch (LOWORD(wParam)) {
			case IDOK:
				ShowWindow(hDlg, SW_HIDE);
				return TRUE;

			// Status toggles
			case IDC_STATUS_DISABLED:
				ToggleScheduledEnabled(hDlg, FALSE);
				ChangeColors::DisableGrayscale();
				break;
			case IDC_STATUS_ALWAYS_ON:
				ToggleScheduledEnabled(hDlg, FALSE);
				ChangeColors::EnableGrayscale();
				break;
			case IDC_STATUS_SCHEDULE:
				ToggleScheduledEnabled(hDlg, TRUE);
				SetupSchedule(hDlg);
				break;
			
			// "Run at startup" checkbox
			case IDC_RUN_AT_STARTUP:
			{
				UINT checkboxState = IsDlgButtonChecked(hDlg, IDC_RUN_AT_STARTUP);
				Startup::ToggleRunAtStartup(checkboxState == BST_CHECKED);
				break;
			}

			// System tray menu
			case WM_USER_CONTEXT_OPEN:
				ShowWindow(hDlg, SW_RESTORE);
				break;
			case WM_USER_CONTEXT_EXIT:
				SendMessageW(hDlg, WM_USER_EXIT, 0, 0);
				break;
			}
		}
		break;
	case WM_USER_SHELLICON:
		switch (lParam)
		{
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
			ShowWindow(hDlg, SW_RESTORE);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hDlg);
		}
		break;
	case WM_NOTIFY: // Common controls
		switch (((LPNMHDR)lParam)->code)
		{
		case DTN_DATETIMECHANGE:
			SetupSchedule(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_HIDE);
		return TRUE;
	case WM_USER_EXIT:
		DestroyWindow(hDlg);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}


void MainDialog::EnableNotificationIcon(HWND hwnd)
{
	HICON hMainIcon;
	W32_ABORT_IF_EQ(hMainIcon = static_cast<HICON>(LoadImageW(g_hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR)), NULL);

	SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMainIcon);
	NOTIFYICONDATAW notify = { 0 };
	notify.cbSize = sizeof(NOTIFYICONDATA);
	notify.hWnd = hwnd;
	notify.uID = IDI_ICON1;
	notify.uFlags = NIF_MESSAGE | NIF_ICON | NIF_INFO;
	notify.hIcon = hMainIcon;
	notify.uCallbackMessage = WM_USER_SHELLICON;
	notify.uVersion = NOTIFYICON_VERSION_4;
	W32_ABORT_IF_EQ(Shell_NotifyIconW(NIM_ADD, &notify), FALSE);
}


void MainDialog::DisableNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATAW notify = { 0 };
	notify.cbSize = sizeof(NOTIFYICONDATA);
	notify.hWnd = hwnd;
	notify.uID = IDI_ICON1;
	W32_ABORT_IF_EQ(Shell_NotifyIconW(NIM_DELETE, &notify), FALSE);

}
