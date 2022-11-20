#pragma once

#include <Windows.h>

#include <string>


#define W32_ABORT_IF_EQ(x, y) do { if (y == (x)) { Utils::LogAndAbort(L ## #x); }} while (0)
#define W32_ABORT_IF_NOT_EQ(x, y) do { if (y != (x)) { Utils::LogAndAbort(L ## #x); }} while (0)


namespace Utils
{
	const std::wstring GetErrStr(LPCWSTR failedComponent);
	void LogAndAbort(LPCWSTR failedComponent);
	void InitializeCommonControls();
};

