#include "utils.h"


int main()
{ 
	DWORD my_pid;
	HWND hWnd = FindWindowA(0, ("Overwatch"));
	GetWindowThreadProcessId(hWnd, &my_pid);

	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, my_pid);
	if (proc == NULL)
		printf("Process Open Failed\n");

	threadHijacking(proc, L"C:\\Users\\Kwangil\\source\\repos\\wannaBePro\\x64\\Release\\wannaBePro.dll");
	WaitForSingleObject(proc, INFINITE); 
	return 0; 
}