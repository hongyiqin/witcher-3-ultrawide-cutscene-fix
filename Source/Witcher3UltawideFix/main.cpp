#include "stdafx.h"

#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <limits>

#undef max
#define PATH_LENGTH 32000

using namespace std;


void pause()
{
	system("pause");
}

void stop(int code)
{
	system("pause");
	exit(code);
}

int getResolutionOption()
{
	while (true)
	{
		int input;
		wcout << "Select resolution (1: 3440x1440, 2: 2560x1080, 3: 5120x1440 or 3840x1080 ) : ";
		cin >> input;
		if (input == 1 || input == 2 || input == 3)
			return input;	
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		wcout << "Wrong input, please select 1, 2 or 3" << endl;
	}
}

int getOverwriteOffset(char *exeData, int size)
{
	// Find the right position. We are looking for this 4 byte pattern.
	char pattern[4] = { 0x55, 0x55, 0x15, 0x40 };
	int overwriteOffset = 0;
	int matches = 0;

	for (int offset = 0; offset < size - 4; offset++)
	{
		int matched = 0;
		for (int i = 0; i < 4; i++)
		{
			if (exeData[offset + i] != pattern[i])
			{
				break;
			}
			matched++;
		}

		if (matched == 4)
		{
			matches++;
			overwriteOffset = offset - 4;
		}
	}

	if (matches != 1)
	{
		wcout << "Can't find correct position to modify in witcher3.exe" << endl;
		stop(1);
	}

	return overwriteOffset;
}

int main()
{

	TCHAR currentPath[PATH_LENGTH] = { 0 };
	TCHAR witcherExePath[PATH_LENGTH] = { 0 };
	TCHAR witcherExeBackupPath[PATH_LENGTH] = { 0 };

	if (GetCurrentDirectory(PATH_LENGTH, currentPath) == 0)
	{
		wcout << "Problem encountered, make sure you have sufficient permissions (run as administrator)." << endl;
		stop(1);
	}

	wcscat_s(witcherExePath, currentPath);
	wcscat_s(witcherExePath, L"\\witcher3.exe");

	wcscat_s(witcherExeBackupPath, currentPath);
	wcscat_s(witcherExeBackupPath, L"\\backup_witcher3.exe");

	bool backupSuccess = CopyFile(witcherExePath, witcherExeBackupPath, 0);
	if (!backupSuccess)
	{
		wcout << "Can't access witcher3.exe in current directory (" << currentPath << "). Make sure the tool is in the same directory as witcher3.exe and you have sufficient permissions (run as administrator)." << endl;
		stop(1);
	}

	wcout << "Backed up original exe at " << witcherExeBackupPath << endl;
	
	int resolutionOption = getResolutionOption();

	// The patch is pretty simple, find a specific 4 byte pattern, then replace the 4 preceding bytes.

	fstream witcher3Exe;
	witcher3Exe.open(witcherExePath, ios::in|ios::out|ios::binary|ios::ate);

	if (witcher3Exe.fail())
	{
		wcout << "Problem opening witcher3.exe. Make sure the file is not in use and you have sufficient permissions (run as administrator)." << endl;
		stop(1);
	}

	// Copy the file content to a buffer, so we can search the correct bytes to override.
	int size = witcher3Exe.tellg();
	char* exeData = new char[size];
	witcher3Exe.seekg(0, ios::beg);
	witcher3Exe.read(exeData, size);
	
	int overWriteOffset = getOverwriteOffset(exeData, size);
	delete exeData;
	
	// 3440x1440
	const char resolution1[4] = { 0x8E, 0xE3, 0x18, 0x40 };
	// 2560x1080
	const char resolution2[4] = { 0x24, 0xB4, 0x17, 0x40 };
	// 32:9
	const char resolution3[4] = { 0x39, 0xBE, 0x63, 0x40 };

	// Overwrite with new values.
	witcher3Exe.seekg(overWriteOffset, ios::beg);
	if (resolutionOption == 1)
	{
		witcher3Exe.write(resolution1, 4);
	}
	else if (resolutionOption == 2)
	{
		witcher3Exe.write(resolution2, 4);
	}
	else if (resolutionOption == 3)
	{
		witcher3Exe.write(resolution3, 4);
	}

	witcher3Exe.close();
	
	if (witcher3Exe.fail())
	{
		wcout << "Problem modifying witcher3.exe. Make sure the file is not in use and you have sufficient permissions (run as administrator)." << endl;
		stop(1);
	}

	wcout << "Done" << endl;
	pause();
	return 0;
}