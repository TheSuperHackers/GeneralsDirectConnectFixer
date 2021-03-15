
#include "stdafx.h"
#include "strlcpy.h"
#include <windows.h>
#include <Shlobj.h>
#include <ctime>

//////////////////////////////////////////////////////////////

void InitRandom()
{
	::srand((unsigned int)::time(0));
}

//////////////////////////////////////////////////////////////

template <size_t Size>
void GenerateRandomChars(wchar_t(&str)[Size])
{
    const wchar_t alphanum[] = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	const size_t alphanumLen = (sizeof(alphanum)/sizeof(wchar_t)) - 1;
    for (size_t i = 0; i < Size-1; ++i)
		str[i] = alphanum[::rand() % alphanumLen];
	str[Size-1] = wchar_t(0);
}

//////////////////////////////////////////////////////////////

template <size_t Size>
bool GetRegValueData(const wchar_t* regKey, const wchar_t* regValue, wchar_t(&regData)[Size])
{
	bool success = false;
	HKEY hKey;
	if (::RegOpenKeyExW(HKEY_LOCAL_MACHINE, regKey, NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		BYTE* data = (BYTE*)regData;
		DWORD size = Size;
		if(::RegQueryValueExW(hKey, regValue, NULL, NULL, data, &size) == ERROR_SUCCESS)
		{
			success = true;
		}
		::RegCloseKey(hKey);
	}
	return success;
}

//////////////////////////////////////////////////////////////

bool SetRegValueData(const wchar_t* regKey, const wchar_t* regValue, const wchar_t* regData)
{
	bool success = false;
	HKEY hKey;
	if (::RegOpenKeyExW(HKEY_LOCAL_MACHINE, regKey, NULL, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
	{
		BYTE* data = (BYTE*)regData;
		DWORD size = (::wcslen(regData) + 1) * 2;
		if (::RegSetValueExW(hKey, regValue, 0, REG_SZ, data, size) == ERROR_SUCCESS)
		{
			success = true;
		}
		::RegCloseKey(hKey);
	}
	return success;
}

//////////////////////////////////////////////////////////////

bool IsDirectory(const wchar_t* path, DWORD* pFileAttributes = 0)
{
	
	const DWORD fileAttributes = ::GetFileAttributesW(path);
	const bool isDirectory = (fileAttributes != INVALID_FILE_ATTRIBUTES) && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (pFileAttributes)
		*pFileAttributes = fileAttributes;
	return isDirectory;
}

//////////////////////////////////////////////////////////////

void Welcome()
{
	::printf("GameRanger Direct Connect Fixer v1.3 by xezon\n");
	::printf("---------------------------------------------\n\n");
};

//////////////////////////////////////////////////////////////////////////////////////

void Shutdown()
{
	::printf("\n");
	::printf("Press any key to continue . . .\n");
	::FlushConsoleInputBuffer(::GetStdHandle(STD_INPUT_HANDLE));
	::getchar();
};

//////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	InitRandom();
	Welcome();

	int error = 0;
	bool revertRegistryChange = true;
	const wchar_t* regKey = L"SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
	const wchar_t* regValue = L"UserDataLeafName";
	const wchar_t* regData = L"Command and Conquer Generals Zero Hour Data";

	wchar_t myDocuments[MAX_PATH];
	if (::SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments) != S_OK)
	{
		::wprintf(L"ERROR: Could not find MyDocuments folder.\n");
		Shutdown();
		return 1;
	}

#ifdef _DEBUG
	// Set german registry for testing
	wchar_t* regDataGerman = L"Command & Conquer Generäle Stunde Null Data";
	SetRegValueData(regKey, regValue, regDataGerman);
#endif _DEBUG

	wchar_t userData[MAX_PATH];
	if (!GetRegValueData(regKey, regValue, userData))
	{
		::wprintf(L"ERROR: Could not find '%s' in registry.\n", regValue);
		error = 1;
	}
	else
	{
		wchar_t curUserDataDir[MAX_PATH];
		::wcslcpy_t(curUserDataDir, myDocuments);
		::wcslcat_t(curUserDataDir, L"\\");
		::wcslcat_t(curUserDataDir, userData);

		wchar_t newUserDataDir[MAX_PATH];
		::wcslcpy_t(newUserDataDir, myDocuments);
		::wcslcat_t(newUserDataDir, L"\\");
		::wcslcat_t(newUserDataDir, regData);

		if (::wcscmp(userData, regData) == 0)
		{
			::wprintf(L"Game user data is compatible with GameRanger.\n");
		}
		else
		{
			::wprintf(L"Game user data is not compatible with GameRanger.\n");
			::wprintf(L"This program will update registry data and move game user files.\n");
			::wprintf(L"Close game and press any key to continue . . .\n");

			::getchar();

			::wprintf(L"Modify registry value data\n");
			::wprintf(L"from %s\\%s = %s\n", regKey, regValue, userData);
			::wprintf(L"to   %s\\%s = %s\n", regKey, regValue, regData);

			if (!SetRegValueData(regKey, regValue, regData))
			{
				::wprintf(L"Modification failed.\n");
				error = 1;
			}
			else
			{
				::wprintf(L"Modification succeeded.\n");

				DWORD dwFileAttributes;
				if (!IsDirectory(curUserDataDir, &dwFileAttributes))
				{
					::wprintf(L"ERROR: Directory '%s' does not exist. File attributes: 0x%.08X\n", curUserDataDir, dwFileAttributes);
				}
				else
				{
					const DWORD dwflags = MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH;

					// Backup eventual existing folder
					if (IsDirectory(newUserDataDir))
					{
						::wprintf(L"Backup %s\n", newUserDataDir);
						wchar_t randomNumbers[9];
						wchar_t backupUserDataDir[MAX_PATH];
						GenerateRandomChars(randomNumbers);
						::wcslcpy_t(backupUserDataDir, newUserDataDir);
						::wcslcat_t(backupUserDataDir, L"_");
						::wcslcat_t(backupUserDataDir, randomNumbers);
						if (::MoveFileExW(newUserDataDir, backupUserDataDir, dwflags) == FALSE)
						{
							DWORD errorCode = ::GetLastError();
							::wprintf(L"Backup failed. Error %u.\n", errorCode);
						}
						else
						{
							::wprintf(L"Backup succeeded.\n");
						}
					}

					// Copy current user data folder to new data folder
					::wprintf(L"Copy user data directory\n");
					::wprintf(L"from %s\n", curUserDataDir);
					::wprintf(L"to   %s\n", newUserDataDir);
					if (::MoveFileExW(curUserDataDir, newUserDataDir, dwflags) == FALSE)
					{
						const DWORD dwError = ::GetLastError();
						::wprintf(L"Copy failed. Error %u.\n", dwError);
						error = (int)dwError;
					}
					else
					{
						::wprintf(L"Copy succeeded.\n");
						revertRegistryChange = false;
					}
				}
			}
		}
	}

	if (revertRegistryChange)
	{
		::SetRegValueData(regKey, regValue, userData);
	}

	Shutdown();
	return error;
}

//////////////////////////////////////////////////////////////