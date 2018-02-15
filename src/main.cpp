#include <bstring.h>
#include <butils.h>
#include <bexcept.h>
#include <bprocess.h>
#include <wconsole.h>
#include <scanfile.h>

using b::String;
//---------------------------------------------------------------------------
// Defines
// #define CANDELETE

//---------------------------------------------------------------------------
ConsoleOut cout;
ConsoleErr cerr;

String getLastErrorMessage();

class TFindOldFiles: public ScanFile {
	LARGE_INTEGER Time;
public:
	TFindOldFiles(int DaysOld) {
		Time.QuadPart = 0;
		SYSTEMTIME SystemTime;
		GetSystemTime(&SystemTime);
		FILETIME SystemFileTime;
		if (SystemTimeToFileTime(&SystemTime, &SystemFileTime)) {
			// one sec is 10000000 * 100 nsec.
			// one day is 24 * 60 * 60 => 86400 sec
			// one day is 864000000000 ticks
			Time.LowPart = SystemFileTime.dwLowDateTime;
			Time.HighPart = SystemFileTime.dwHighDateTime;
			Time.QuadPart -= 864000000000LL * DaysOld;
		}
	}
	bool IsOlder() {
		bool bReturn = false;
		if (Time.QuadPart != 0) {
			LARGE_INTEGER FileTime;
			FileTime.LowPart = FindData->ftCreationTime.dwLowDateTime;
			FileTime.HighPart = FindData->ftCreationTime.dwHighDateTime;

			if (FileTime.QuadPart <= Time.QuadPart)
				bReturn = true;
		}
		return bReturn;
	}
#ifdef CANDELETE
	bool static CanDelete(const TCHAR* file)
	{
		HANDLE hFile = CreateFile(file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return true;
		}
		else
		return false;
	}
#endif
};

int main(int argc, char** argv) {
#ifdef UNICODE
	argv = CommandLineToArgvW(GetCommandLine(), &argc);
#endif
	if (argc < 2 || argc > 3) {
		cerr << TEXT("Usage: delete pattern [days old]\n");
		return 1;
	}
	try {
		String Mask(TEXT("*"));
		String Dir(argv[1]);
		int pos = Dir.last_index(TCHAR('\\'));
		if (pos != b::npos) {
			CONST TCHAR* ptr = &Dir.c_str()[pos + 1];
			if (*ptr)
				Mask = ptr;
			Dir.resize((pos) ? pos : 1);
		} else {
			Mask = Dir;
			Dir = TEXT(".");
		}
		cout << TEXT("Directory: ") << Dir << endl;
		cout << TEXT("File mask: ") << Mask << endl;
		int DaysOld = 0;
		if (argc == 3) {
			DaysOld = b::atoi(argv[2]);
			cout << TEXT("Days old : ") << argv[2] << endl;
		}
		b::TStack<String> DirStack;
		String Path;
		TFindOldFiles Scan(DaysOld);
		TFindOldFiles::TScanResult Result = Scan.Init(Dir, true);
		while (Result != ScanFile::Invalid) {
			switch (Result) {
			case TFindOldFiles::IsFile:
				if (b::CompareMask(Scan.GetFileName(), Mask.c_str())) {
					if (Scan.IsOlder()) {
						Path = Scan.GetFile();
#ifdef CANDELETE
						if(TFindOldFiles::CanDelete(Path) && DeleteFile(Path)) {
#else
						if (DeleteFile(Path)) {
#endif
							cout << TEXT("Deleted  : ") << Path << endl;
						} else {
							cerr << TEXT("Cannot delete: ") << Path << endl;
							cerr << TEXT("Reason       : ")
									<< getLastErrorMessage() << endl;
						}
					}
				}
				break;
			case TFindOldFiles::IsDirectory:
				DirStack.Push(Scan.GetFile());
				break;
			default:
				break;
			}
			Result = Scan.Next();
		}
		if (!DirStack.isEmpty()) {
			cout << TEXT("Deleting empty directories...\n");
			while (DirStack.Pop(Path)) {
				if (RemoveDirectory(Path)) {
					cout << TEXT("Deleted  : ") << Path << endl;
				}
			}
		}
		cout << TEXT("Ready\n");
	} catch (std::exception &e) {
		cerr << TEXT("Exception: ") << e.what() << endl;
		return -1;
	}
#ifdef _DEBUG
	asm int 3;
#endif
	return 0;
}

String getLastErrorMessage() {
	String msg;
	TCHAR* MsgBuf = 0;
	if (FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL, GetLastError(), 0, // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &MsgBuf, 0,
			NULL)) {
		msg = MsgBuf;
	} else {
		msg.clear();
	}
	LocalFree(MsgBuf);
	return msg;
}
