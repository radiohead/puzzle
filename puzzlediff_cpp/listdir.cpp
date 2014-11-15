#include "listdir.hpp"

using namespace std;

#ifdef WIN32 || WIN64
	#include <windows.h>

	void listDir(const char* dirName, vector<string>& fnVec)
	{
		size_t origsize = strlen(dirName) + 1;
		size_t convertedChars = 0;
		wchar_t wdirName[MAX_PATH];

		mbstowcs_s(&convertedChars, wdirName, origsize, dirName, _TRUNCATE);
		wcscat_s(wdirName, L"\\*.*");

		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(wdirName, &fd);
		fnVec.clear();

		// Convert to a char*
		origsize = wcslen(fd.cFileName) + 1;
		convertedChars = 0;
		char nstring[MAX_PATH];
		wcstombs_s(&convertedChars, nstring, origsize, fd.cFileName, _TRUNCATE);

		fnVec.push_back(nstring);

		while (FindNextFile(h, &fd))
		{
			// Convert to a char*
			size_t origsize = wcslen(fd.cFileName) + 1;
			size_t convertedChars = 0;
			char nstring[MAX_PATH];
			wcstombs_s(&convertedChars, nstring, origsize, fd.cFileName, _TRUNCATE);
			char filedirstring[MAX_PATH];
			strcpy(filedirstring, dirName);
			strcat(filedirstring, "\\");
			strcat(filedirstring, nstring);
			fnVec.push_back(filedirstring);
		}

		fnVec.erase(fnVec.begin(), fnVec.begin() + 2);
	}
#elif defined __linux__ || __APPLE__
	#include <dirent.h>
	#include <sys/types.h>
	#include <errno.h>
	#include <string.h>

	void listDir(const char* dirName, vector<string> &files)
	{
		DIR *dp;
		struct dirent *dirp;
		if ((dp  = opendir(dirName)) == NULL) {
			cout << "Error(" << errno << ") opening " << dirName << endl;
		}

		while ((dirp = readdir(dp)) != NULL) {
			if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
				files.push_back(dirName+string(dirp->d_name));
			}
		}
		closedir(dp);
	}
#else
	void listDir(const char* dirName, vector<string> &files)
	{
	}
#endif
