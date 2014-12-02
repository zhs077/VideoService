#ifndef MINI_DUMP_H_
#define MINI_DUMP_H_
/**�����������dumpfile�ļ�
Releaseģʽ
�����lib�޸�4���ط���

1. ���� �� properties �� C/C++ ��General �� Debug Information Format       ѡ�� ��Program Database for Edit & Continue (/ZI)����ʹrelease�¿ɵ�ʽ

2. ���� �� properties �� C/C++ ��Optimization ��Optimization                ѡ�� ��Disabled (/Od)��

3. ���� �� properties �� C/C++ ��Optimization ��Whole Program Optimization ѡ�� ��No����1��2��3������DUMP�ļ��йأ�

4. ���� �� properties �� C/C++ ��Output Files ��Assembler Output            ѡ�� ��Assembly, Machine Code and Source (/FAcs)���˴�����cod�ļ���������������code��

�����dll�����޸�����4������Ҫ�޸�Linker��

5. ���� �� properties �� Linker ��Debugging ��Generate Map File            ѡ�� ��Yes (/MAP)���˴�����map�ļ������к���������ڴ��ַ ������ʱ���б�����ַ���Ծݴ˲�ѯ��


*/

#include <windows.h>
#pragma comment(lib, "Dbghelp.lib")
#include <time.h>
#include <tchar.h>
#include <Dbghelp.h>

namespace MiniDump
{
#ifndef _M_IX86   
#error "The following code only works for x86!"   
#endif   

	inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)  
	{  
		if(pModuleName == 0)  
		{  
			return FALSE;  
		}  

		WCHAR szFileName[_MAX_FNAME] = L"";  
		_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);  

		if(wcsicmp(szFileName, L"ntdll") == 0)  
			return TRUE;  

		return FALSE;  
	}  

	inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,  
		const PMINIDUMP_CALLBACK_INPUT   pInput,  
		PMINIDUMP_CALLBACK_OUTPUT        pOutput)  
	{  
		if(pInput == 0 || pOutput == 0)  
			return FALSE;  

		switch(pInput->CallbackType)  
		{  
		case ModuleCallback:  
			if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)  
				if(!IsDataSectionNeeded(pInput->Module.FullPath))  
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);  
		case IncludeModuleCallback:  
		case IncludeThreadCallback:  
		case ThreadCallback:  
		case ThreadExCallback:  
			return TRUE;  
		default:;  
		}  

		return FALSE;  
	}  

	inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)  
	{  
		HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,  
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

		if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))  
		{  
			MINIDUMP_EXCEPTION_INFORMATION mdei;  
			mdei.ThreadId           = GetCurrentThreadId();  
			mdei.ExceptionPointers  = pep;  
			mdei.ClientPointers     = NULL;  

			MINIDUMP_CALLBACK_INFORMATION mci;  
			mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
			mci.CallbackParam       = 0;  

			::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);  

			CloseHandle(hFile);  
		}  
	}  

	LONG __stdcall UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)  
	{  
		TCHAR directory [MAX_PATH+1] = {0};
		GetModuleFileName(NULL, directory, MAX_PATH);
		time_t t = time(0);
		TCHAR tmp[64];
		memset(tmp,0,sizeof(tmp));
		_tcsftime( tmp, sizeof(tmp), _T("(%Y%m%d_%H%M%S).dmp"),localtime(&t));
		_tcscat(directory,tmp);
		CreateMiniDump(pExceptionInfo, directory);  
		FatalAppExit(-1,  _T("Fatal Error"));

		return EXCEPTION_CONTINUE_SEARCH;  
	}  

	// �˺���һ���ɹ����ã�֮��� SetUnhandledExceptionFilter �ĵ��ý���Ч   
	void DisableSetUnhandledExceptionFilter()  
	{  
		void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"),  
			"SetUnhandledExceptionFilter");  

		if (addr)  
		{  
			unsigned char code[16];  
			int size = 0;  

			code[size++] = 0x33;  
			code[size++] = 0xC0;  
			code[size++] = 0xC2;  
			code[size++] = 0x04;  
			code[size++] = 0x00;  

			DWORD dwOldFlag, dwTempFlag;  
			VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);  
			WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);  
			VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);  
		}  
	}  

	void InitMinDump()  
	{  
		//ע���쳣������   
		SetUnhandledExceptionFilter(UnhandledExceptionFilter);  

		//ʹSetUnhandledExceptionFilter   
		DisableSetUnhandledExceptionFilter();  
	}  

};


#endif // !MINI_DUMP_H_
