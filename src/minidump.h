#ifndef MINI_DUMP_H_
#define MINI_DUMP_H_
/**程序崩溃产生dumpfile文件
Release模式
如果是lib修改4处地方：

1. 工程 à properties à C/C++ àGeneral à Debug Information Format       选择 “Program Database for Edit & Continue (/ZI)”可使release下可调式

2. 工程 à properties à C/C++ àOptimization àOptimization                选择 “Disabled (/Od)”

3. 工程 à properties à C/C++ àOptimization àWhole Program Optimization 选择 “No”（1，2，3与生成DUMP文件有关）

4. 工程 à properties à C/C++ àOutput Files àAssembler Output            选择 “Assembly, Machine Code and Source (/FAcs)”此处生成cod文件（包含汇编的所有code）

如果是dll除了修改上面4处，还要修改Linker处

5. 工程 à properties à Linker àDebugging àGenerate Map File            选择 “Yes (/MAP)”此处生成map文件（所有函数的入口内存地址 当崩溃时会有崩溃地址可以据此查询）


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

	// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效   
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
		//注册异常处理函数   
		SetUnhandledExceptionFilter(UnhandledExceptionFilter);  

		//使SetUnhandledExceptionFilter   
		DisableSetUnhandledExceptionFilter();  
	}  

};


#endif // !MINI_DUMP_H_
