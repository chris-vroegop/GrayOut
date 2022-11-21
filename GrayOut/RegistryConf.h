#pragma once

#include <Windows.h>


typedef unsigned __int64 QWORD;

enum class GrayOutMode : DWORD { GRAYOUT_DISABLED, GRAYOUT_ALWAYS_ON, GRAYOUT_SCHEDULED };

typedef struct
{
	GrayOutMode mode;

} GrayOutBasicConfig;

typedef struct
{
	WORD startHour;
	WORD startMin;
	WORD startSec;
	WORD endHour;
	WORD endMin;
	WORD endSec;

} GrayOutSchedConfig;

typedef struct
{
	GrayOutBasicConfig basicConf;
	GrayOutSchedConfig schedConf;
} GrayOutConfig;


class RegistryConf
{
public:
	RegistryConf();
	~RegistryConf();

	bool IsRunningAtStartupWithCorrectPath();
	void ToggleRunAtStartup(bool runAtStartup);
	GrayOutConfig RetrieveConfig();
	void SaveBasicConfig(GrayOutBasicConfig basicConf);
	void SaveSchedConfig(GrayOutSchedConfig schedConf);

private:
	HKEY m_startupKey = nullptr;
	LPCWSTR m_startupKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	LPCWSTR m_startupValName = L"GrayOut";

	HKEY m_configKey = nullptr;
	LPCWSTR m_configKeyPath = L"Software\\GrayOut";
	LPCWSTR m_scheduleValName = L"GrayOutSchedule";
	LPCWSTR m_modeValName = L"GrayOutMode";
};
