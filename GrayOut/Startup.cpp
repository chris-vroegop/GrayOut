#include "Startup.h"

#include <Windows.h>
#include <windowsx.h>

#include "Utils.h"



LPCWSTR g_regKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
LPCWSTR g_regKeyName = L"GrayOut";


bool Startup::IsRunningAtStartupWithCorrectPath()
{
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, g_regKeyPath, 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		WCHAR path[MAX_PATH];
		GetModuleFileNameW(NULL, path, MAX_PATH);

		DWORD bufferSize = 1024;
		WCHAR data[1024];
		LSTATUS regValueStatus = RegQueryValueExW(key, g_regKeyName, NULL, NULL, reinterpret_cast<LPBYTE>(&data), &bufferSize);

		if ((regValueStatus == ERROR_SUCCESS) && (wcscmp(data, path) == 0))
		{
			return true;
		}
	}
	return false;
}


void Startup::ToggleRunAtStartup(bool runAtStartup)
{
	HKEY subKey = nullptr;
	W32_ABORT_IF_NOT_EQ(RegCreateKeyW(HKEY_CURRENT_USER, g_regKeyPath, &subKey), ERROR_SUCCESS);
	DWORD bufferSize = 1024;
	WCHAR data[1024];
	LSTATUS regValueStatus = RegQueryValueExW(subKey, g_regKeyName, NULL, NULL, reinterpret_cast<LPBYTE>(&data), &bufferSize);

	switch (regValueStatus)
	{
	case ERROR_FILE_NOT_FOUND:
		if (!runAtStartup)
		{
			return;
		}
		// fallthrough
	case ERROR_SUCCESS:
		if (runAtStartup)
		{
			// Location may have changed, so re-write key.
			WCHAR path[MAX_PATH];
			GetModuleFileNameW(NULL, path, MAX_PATH);
			W32_ABORT_IF_NOT_EQ(RegSetValueExW(subKey, g_regKeyName, 0, REG_SZ, reinterpret_cast<LPBYTE>(path), static_cast<DWORD>(wcslen(path) + 1) * 2), ERROR_SUCCESS);
		}
		else
		{
			RegDeleteValueW(subKey, g_regKeyName);
		}
		break;
	default:
		Utils::LogAndAbort(L"RegQueryValueExW");
	}
}
