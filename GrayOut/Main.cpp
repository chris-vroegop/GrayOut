#pragma comment(linker, \
"\"/manifestdependency:type='Win32' "\
"name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' "\
"processorArchitecture='*' "\
"publicKeyToken='6595b64144ccf1df' "\
"language='*'\"")


#include <Windows.h>

#include "MainDialog.h"
#include "Utils.h"
#include "ChangeColors.h"
#include "resource.h"



HINSTANCE g_hInstance;


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,	_In_ int nShowCmd)
{
	g_hInstance = hInstance;

	HWND hDlg;
	MSG msg;
	BOOL ret;

	Utils::InitializeCommonControls();
	ChangeColors::Init();

	W32_ABORT_IF_EQ(hDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, MainDialog::Dlgproc, 0), NULL);

	MainDialog::EnableNotificationIcon(hDlg);
	
	ShowWindow(hDlg, nShowCmd);

	while ((ret = GetMessageW(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			Utils::LogAndAbort(L"GetMessageW");

		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	MainDialog::DisableNotificationIcon(hDlg);


	return 0;
}
