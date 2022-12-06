#include "Utils.h"

#include <Windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include <string>
#include <sstream>

#include "resource.h"



const std::wstring Utils::GetErrStr(LPCWSTR failedComponent)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	// LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process
	std::wostringstream oss;
	oss << failedComponent << " failed with error " << dw << ": " << (LPCWSTR)lpMsgBuf << std::endl;

	LocalFree(lpMsgBuf);
	return oss.str();
}


void Utils::LogAndAbort(LPCWSTR failedComponent)
{
	const std::wstring errMsg = GetErrStr(failedComponent);
	MessageBoxW(NULL, errMsg.c_str(), L"Error in GrayOut", MB_OK | MB_ICONERROR);
	exit(-1);
}


void Utils::InitializeCommonControls()
{
	INITCOMMONCONTROLSEX icex = { 0 };
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_STANDARD_CLASSES | ICC_DATE_CLASSES | ICC_UPDOWN_CLASS;

	InitCommonControlsEx(&icex);
}
