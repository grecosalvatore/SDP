/*	SKELETON OF WINDOWS VERSION OF MULTIPLE PRODUCE AND CONSUMER WITH SEMAPHORES
*/


#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif // !UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

typedef struct _ThreadParameter {
	int id;
	LPTSTR filename;
	HANDLE semFull, semEmpty, semMEp, semMEc;
} ThreadParameter;


void produce() {
	Sleep(3000);
	_tprintf(_T("message produced\n"));
	return;
}

void consume() {
	Sleep(2000);
	return;
}

void enqueue(LPTSTR s) {
	_tprintf(_T("Enqueue done\n"));
	return;
}

LPSTR dequeue() {
	return _T("dequeue done\n");
}

DWORD WINAPI producer(PVOID pThParam) {
	ThreadParameter *p;

	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("Producer %d created\n"),p->id);

	while (1) {
		produce();
		WaitForSingleObject(p->semEmpty, INFINITE);
		WaitForSingleObject(p->semMEp, INFINITE);
		enqueue("prova");
		ReleaseSemaphore(p->semMEp,1,NULL);
		ReleaseSemaphore(p->semFull, 1, NULL);
	}

	ExitThread(0);
}

DWORD WINAPI consumer(PVOID pThParam) {
	ThreadParameter *p;

	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("Consumer %d created\n"), p->id);

	while (1) {
		produce();
		WaitForSingleObject(p->semFull, INFINITE);
		WaitForSingleObject(p->semMEc, INFINITE);
		LPSTR m = dequeue();
		_tprintf(_T("%s\n"), m);
		ReleaseSemaphore(p->semMEc, 1, NULL);
		ReleaseSemaphore(p->semEmpty, 1, NULL);
	}

	ExitThread(0);
}



INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE *p, *c;
	HANDLE semFull, semEmpty, semMEp, semMEc;
	DWORD n;
	DWORD *thIdP, *thIdC;
	ThreadParameter *thParP, *thParC;
	if (argc != 2) {
		_tprintf(_T("Error in the number of parameters %s <nPC>\n"), argv[0]);
		return -1;
	}
	n = _ttoi(argv[1]);

	//allocation of memory
	c = (HANDLE *)malloc(n * sizeof(HANDLE));
	p = (HANDLE *)malloc(n * sizeof(HANDLE));
	thIdC = (DWORD *)malloc(n * sizeof(DWORD));
	thIdP = (DWORD *)malloc(n * sizeof(DWORD));
	thParP = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));
	thParC = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));

	//creation of semaphores
	semFull = CreateSemaphore(NULL,0,n,NULL);
	semEmpty = CreateSemaphore(NULL, n, n, NULL);
	semMEp = CreateSemaphore(NULL, 1, 1, NULL);
	semMEc = CreateSemaphore(NULL, 1, 1, NULL);

	//create n producers
	for (int i = 0; i < n; i++) {
		thIdP[i] = i;
		thParP[i].id = i;
		thParP[i].semEmpty = semEmpty;
		thParP[i].semFull = semFull;
		thParP[i].semMEc = semMEc;
		thParP[i].semMEp = semMEp;
		p[i] = CreateThread(NULL,0,producer, &thParP[i],NULL,&thIdP[i]);
		if (p[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of producer \n"));
			return -1;
		}
	}

	//create n consumers
	for (int i = 0; i < n; i++) {
		thIdC[i] = i;
		thParC[i].id = i;
		c[i] = CreateThread(NULL, 0, consumer, &thParC[i], NULL, &thIdC[i]);
		if (c[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of consumer \n"));
			return -1;
		}
	}

	/* Wait for the producers to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS),&p[i], TRUE, INFINITE);

	/* Wait for the consumers to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &c[i], TRUE, INFINITE);

	return 0;
}