/*	SKELETON OF WINDOWS VERSION OF MULTIPLE PRODUCE AND CONSUMER WITH SEMAPHORES AND
ONE CIRCULAR BUFFER
*/


#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif // !UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

//global variabls;
INT n,counter = 0;


typedef struct _ThreadParameter {
	int id;
	//HANDLE s1;
	//HANDLE s2;
	//CRITICAL_SECTION *cs;
} ThreadParameter;


typedef struct _ThreadParameterB {
	int id;
	//HANDLE *s1;
	//HANDLE s2;
} thParB;


HANDLE *s1, s2;
CRITICAL_SECTION cs;


DWORD WINAPI thAWorker(PVOID pThParam) {
	ThreadParameter *p;

	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("thA %d created\n"), p->id);

	while (1) {
		WaitForSingleObject(s1[p->id], INFINITE);
		Sleep(1000);
		_tprintf(_T("thA %d HAVE PRODUCED\n"), p->id);
		EnterCriticalSection(&cs);
		counter++;
		if (counter == n) {
			ReleaseSemaphore(s2,1,NULL);
		}
		LeaveCriticalSection(&cs);
	}

	ExitThread(0);
}

DWORD WINAPI thBWorker(PVOID pThParam) {
	thParB *p;
	BOOLEAN termination;
	//cast of parameter
	p = (thParB *)pThParam;
	_tprintf(_T("Consumer %d created\n"), p->id);

	while (1) {
		WaitForSingleObject(s2, INFINITE);
		_tprintf(_T("Barrier activated %d \n"), p->id);
		Sleep(3000);
		EnterCriticalSection(&cs);
		counter = 0;
		LeaveCriticalSection(&cs);
		for (int i = 0; i < n; i++) {
			ReleaseSemaphore(s1[i], 1, NULL);
		}
	}

	ExitThread(0);
}



INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE *thA, thB;
	DWORD *thId, thIdC;
	ThreadParameter *thParA ;
	thParB parB;
	//HANDLE *s1,s2;
	//CRITICAL_SECTION cs;
	if (argc != 2) {
		_tprintf(_T("Error in the number of parameters %s <n>\n"), argv[0]);
		return -1;
	}
	
	n = _ttoi(argv[1]);

	//allocation of memory
	thA = (HANDLE *)malloc(n * sizeof(HANDLE));
	thId = (DWORD *)malloc(n * sizeof(DWORD));
	thParA = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));
	s1 = (HANDLE *)malloc(n * sizeof(HANDLE));

	//creation of semaphores
	for (int i = 0; i < n; i++) {
		s1[i] = CreateSemaphore(NULL, 1, 1, NULL);
	}

	s2 = CreateSemaphore(NULL,0 , 1, NULL);
	InitializeCriticalSection(&cs);
	
	

	//create n thA
	for (int i = 0; i < n; i++) {
		thId[i] = i;
		thParA[i].id = i;
		//thParA[i].s1 = s1[i];
		//thParA[i].s2 = s2;
		
		thA[i] = CreateThread(NULL, 0, thAWorker, &thParA[i], NULL, &thId[i]);
		if (thA[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of thA \n"));
			return -1;
		}
	}

	parB.id = n;
	/*parB.s1 = (HANDLE *)malloc(n * sizeof(HANDLE));
	for (int i = 0; i < n; i++) {
		parB.s1[i] = s1[i];
	}
	parB.s2 = s2;*/


	//create 1 thB
	thB = CreateThread(NULL, 0, thBWorker, &parB, NULL, NULL);
	if (thB == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error in the creation of thA \n"));
		return -1;
	}

	/* Wait for the producers to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &thA[i], TRUE, INFINITE);

	WaitForSingleObject(thB, INFINITE);

	

	return 0;
}