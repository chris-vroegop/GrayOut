#include "RegistryConf.h"

#include <Windows.h>
#include <windowsx.h>

#include "Utils.h"



RegistryConf::RegistryConf()
{
	W32_ABORT_IF_NOT_EQ(RegCreateKeyW(HKEY_CURRENT_USER, m_startupKeyPath, &m_startupKey), ERROR_SUCCESS);
	W32_ABORT_IF_NOT_EQ(RegCreateKeyW(HKEY_CURRENT_USER, m_configKeyPath, &m_configKey), ERROR_SUCCESS);
}


RegistryConf::~RegistryConf()
{
	RegCloseKey(m_configKey);
	RegCloseKey(m_startupKey);
}


bool RegistryConf::IsRunningAtStartupWithCorrectPath()
{
	DWORD bufferSize = 1024;
	WCHAR data[1024];
	LSTATUS regValueStatus = RegQueryValueExW(m_startupKey, m_startupValName, NULL, NULL, reinterpret_cast<LPBYTE>(&data), &bufferSize);

	if (regValueStatus == ERROR_SUCCESS)
	{
		LPCWSTR arg = L" --minimized";
		WCHAR path[MAX_PATH + 12];
		W32_ABORT_IF_EQ(GetModuleFileNameW(NULL, path, MAX_PATH), MAX_PATH);
		wcscat_s(path, MAX_PATH + 12, arg);
		if (wcscmp(data, path) == 0)
		{
			return true;
		}
	}
	return false;
}


void RegistryConf::ToggleRunAtStartup(bool runAtStartup)
{
	DWORD bufferSize = 1024;
	WCHAR data[1024];
	LSTATUS regValueStatus = RegQueryValueExW(m_startupKey, m_startupValName, NULL, NULL, reinterpret_cast<LPBYTE>(&data), &bufferSize);

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
			LPCWSTR arg = L" --minimized";
			WCHAR path[MAX_PATH + 12];
			W32_ABORT_IF_EQ(GetModuleFileNameW(NULL, path, MAX_PATH), MAX_PATH);
			wcscat_s(path, MAX_PATH + 12, arg);
			W32_ABORT_IF_NOT_EQ(RegSetValueExW(m_startupKey, m_startupValName, 0, REG_SZ, reinterpret_cast<LPBYTE>(path), static_cast<DWORD>(wcslen(path) + 1) * 2), ERROR_SUCCESS);
		}
		else
		{
			RegDeleteValueW(m_startupKey, m_startupValName);
		}
		break;
	default:
		Utils::LogAndAbort(L"RegQueryValueExW(m_startupKey...)");
	}
}


GrayOutConfig RegistryConf::RetrieveConfig()
{
	GrayOutConfig config = { {},{0} };

	// Mode data
	DWORD modeData;
	DWORD bufferSize = sizeof(modeData);
	LSTATUS regValueStatus = RegQueryValueExW(m_configKey, m_modeValName, NULL, NULL, reinterpret_cast<LPBYTE>(&modeData), &bufferSize);
	switch (regValueStatus)
	{
	case ERROR_FILE_NOT_FOUND:
		// Set defaults:
		config.basicConf.mode = GrayOutMode::GRAYOUT_DISABLED;
		break;

	case ERROR_SUCCESS:
		config.basicConf.mode = static_cast<GrayOutMode>(modeData);
		break;

	default:
		Utils::LogAndAbort(L"RegQueryValueExW(m_configKey, m_modeValName...)");
	}

	// Schedule data
	QWORD schedData;
	bufferSize = sizeof(schedData);
	regValueStatus = RegQueryValueExW(m_configKey, m_scheduleValName, NULL, NULL, reinterpret_cast<LPBYTE>(&schedData), &bufferSize);
	switch (regValueStatus)
	{
	case ERROR_FILE_NOT_FOUND:
		// Set defaults:
		config.schedConf.startHour = 22;
		config.schedConf.startMin = 0;
		config.schedConf.startSec = 0;
		config.schedConf.endHour = 7;
		config.schedConf.endMin = 0;
		config.schedConf.endSec = 0;
		break;

	case ERROR_SUCCESS:
		config.schedConf.startHour = static_cast<WORD>((schedData & 0xff0000000000LL) >> 40);
		config.schedConf.startMin = static_cast<WORD>((schedData & 0x00ff00000000LL) >> 32);
		config.schedConf.startSec = static_cast<WORD>((schedData & 0x0000ff000000LL) >> 24);
		config.schedConf.endHour = static_cast<WORD>((schedData & 0x000000ff0000LL) >> 16);
		config.schedConf.endMin = static_cast<WORD>((schedData & 0x00000000ff00LL) >> 8);
		config.schedConf.endSec = static_cast<WORD>(schedData & 0x0000000000ffLL);
		break;

	default:
		Utils::LogAndAbort(L"RegQueryValueExW(m_configKey, m_scheduleValName...)");
	}

	return config;
}


void RegistryConf::SaveBasicConfig(GrayOutBasicConfig basicConf)
{
	HKEY newKey = nullptr;
	W32_ABORT_IF_NOT_EQ(RegCreateKeyW(HKEY_CURRENT_USER, m_configKeyPath, &newKey), ERROR_SUCCESS);
	W32_ABORT_IF_NOT_EQ(RegSetValueExW(newKey, m_modeValName, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&basicConf.mode), sizeof(DWORD)), ERROR_SUCCESS);
}


void RegistryConf::SaveSchedConfig(GrayOutSchedConfig schedConf)
{
	QWORD schedule = 0;
	sizeof(QWORD);
	schedule = static_cast<QWORD>(static_cast<__int8>(schedConf.endSec)
		| (static_cast<QWORD>(schedConf.endMin) << 8)
		| (static_cast<QWORD>(schedConf.endHour) << 16)
		| (static_cast<QWORD>(schedConf.startSec) << 24)
		| (static_cast<QWORD>(schedConf.startMin) << 32)
		| (static_cast<QWORD>(schedConf.startHour) << 40));
	
	W32_ABORT_IF_NOT_EQ(RegSetValueExW(m_configKey, m_scheduleValName, 0, REG_QWORD, reinterpret_cast<LPBYTE>(&schedule), sizeof(QWORD)), ERROR_SUCCESS);
}
