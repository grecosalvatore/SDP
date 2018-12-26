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
	
} ThreadParameter;


typedef struct _Record {
	//INT n;
	//TCHAR s[25];
	DWORD number;
} record;

typedef struct queue_s {
	record *buffer;
	DWORD in;
	DWORD out;
	DWORD size;
	DWORD nE;
} queue_t;


//GLOBAL VARIABLES
HANDLE *semFullA, *semEmptyA, *semMEpA, *semMEcA, *semFullB, *semEmptyB, *semMEpB, *semMEcB;
CRITICAL_SECTION *csFileIn, *csFileOut;
DWORD n;
queue_t *queueA, *queueB;
HANDLE *filesIn, *filesOut;



void enqueue(queue_t *queue,record r) {
	
	
	queue->buffer[queue->in] = r;
	queue->in = (queue->in + 1) % queue->size;
	queue->nE = queue->nE + 1;
	
	return;
}

record dequeue(BOOLEAN *termination,queue_t *queue) {
	record r;
	r = queue->buffer[queue->out];
	queue->nE = queue->nE - 1;

	queue->out = (queue->out + 1) % queue->size;
	/*if (val == sentinel) {
		*(termination) = TRUE;
		_tprintf(_T("Termination sentinel recived\n"));
	}*/
	return r;
}

DWORD WINAPI thA(PVOID pThParam) {
	ThreadParameter *p;
	int t,indexOut,indexFile;
	record r;
	DWORD nOut;
	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("ThA %d created\n"), p->id);

	while (1) {

		//sleep random time 1-10 seconds
		t = rand(10) + 1;
		t = t * 1000;
		//Sleep(t);

		//select random file
		indexFile = rand()%n;
		indexFile = p->id;

		//Sleep(t);
		//choose an input file
		r.number = p->id;
		EnterCriticalSection(&csFileIn[indexFile]);
		//read the file
		/*ReadFile(filesIn[indexFile], &r, sizeof(record), &nOut, NULL);
		if (nOut != sizeof(record)) {
			_tprintf(_T("Error: impossible read the file\n"));
			return;
		}*/
		_tprintf(_T("THREAD A %d READ : FROM FILE: %d\n"), p->id, indexFile);
		LeaveCriticalSection(&csFileIn[indexFile]);

		//indexOut = rand() % n;
		indexOut = p->id;

		WaitForSingleObject(semEmptyA[indexOut], INFINITE);
		WaitForSingleObject(semMEpA[indexOut], INFINITE);
		
		//enqueue(&queueA[indexOut],r);

		queueA[p->id].buffer[queueA[p->id].in] = r;
		queueA[p->id].in = (queueA[p->id].in + 1) % queueA[p->id].size;
		queueA[p->id].nE = queueA[p->id].nE + 1;

		_tprintf(_T("THREAD A %d PUT IN QUEUE A %d\n"), p->id, indexOut);

		ReleaseSemaphore(semMEpA[indexOut], 1, NULL);
		ReleaseSemaphore(semFullA[indexOut], 1, NULL);
	}

	ExitThread(0);
}

DWORD WINAPI thB(PVOID pThParam) {
	ThreadParameter *p;
	int t, indexOut, indexIn;
	record r;
	BOOLEAN termination;
	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("ThB %d created\n"), p->id);

	while (1) {

		//sleep random time 1-10 seconds
		t = rand()%10 + 1;
		t = t * 1000;
		//Sleep(t);

		//here consume

		//select random queue
		//indexIn = rand()%n;
		indexIn = p->id;

		WaitForSingleObject(semFullA[indexIn], INFINITE);
		WaitForSingleObject(semMEcA[indexIn], INFINITE);
		//r = dequeue(&termination, &queueA[indexIn]);
		_tprintf(_T("THREAD B %d GET IN QUEUE A %d\n"), p->id, indexIn);

		ReleaseSemaphore(semMEcA[indexIn], 1, NULL);
		ReleaseSemaphore(semEmptyA[indexIn], 1, NULL);

		Sleep(t);
		//produce

		//indexOut = rand() % n;
		indexOut = p->id;

		WaitForSingleObject(semEmptyB[indexOut], INFINITE);
		WaitForSingleObject(semMEpB[indexOut], INFINITE);
		_tprintf(_T("THREAD B %d PUT IN QUEUE B %d\n"), p->id, indexOut);
		ReleaseSemaphore(semMEpB[indexOut], 1, NULL);
		ReleaseSemaphore(semFullB[indexOut], 1, NULL);
	}

	ExitThread(0);
}

DWORD WINAPI thC(PVOID pThParam) {
	ThreadParameter *p;
	int t, indexOut, indexIn;
	record r;
	DWORD nOut;
	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("ThC %d created\n"), p->id);

	while (1) {
		//sleep random time 1-10 seconds
		t = rand()%10 + 1;
		t = t * 1000;
		//Sleep(t);

		//select random file
		//indexIn = rand() % n;
		indexIn = p->id;

 		WaitForSingleObject(semFullB[indexIn], INFINITE);
		WaitForSingleObject(semMEcB[indexIn], INFINITE);
		
		_tprintf(_T("THREAD B %d GET IN QUEUE B %d\n"), p->id, indexIn);

		ReleaseSemaphore(semMEcB[indexIn], 1, NULL);
		ReleaseSemaphore(semEmptyB[indexIn], 1, NULL);
		
		//Sleep(t);

		//choose a random output file
		indexOut = rand() % n;


		EnterCriticalSection(&csFileOut[indexOut]);
		//write the file
		WriteFile(filesIn[indexOut], &r, sizeof(record), &nOut, NULL);
		if (nOut != sizeof(record)) {
			_tprintf(_T("Error: impossible write the file\n"));
			return;
		}
		_tprintf(_T("THREAD B %d PUT IN FILE %d\n"), p->id, indexOut);
		LeaveCriticalSection(&csFileOut[indexOut]);		
	}

	ExitThread(0);
}



INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE *threadsA, *threadsB,*threadsC;
	TCHAR tmpName[25];
	DWORD *thIdA, *thIdB, *thIdC;
	ThreadParameter *thParA, *thParB, *thParC;
	if (argc != 2) {
		_tprintf(_T("Error in the number of parameters %s <nPC>\n"), argv[0]);
		return -1;
	}
	n = _ttoi(argv[1]);

	//allocation of memory
	threadsA = (HANDLE *)malloc(n * sizeof(HANDLE));
	threadsB = (HANDLE *)malloc(n * sizeof(HANDLE));
	threadsC = (HANDLE *)malloc(n * sizeof(HANDLE));
	thIdA = (DWORD *)malloc(n * sizeof(DWORD));
	thIdB = (DWORD *)malloc(n * sizeof(DWORD));
	thIdC = (DWORD *)malloc(n * sizeof(DWORD));
	thParA = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));
	thParB = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));
	thParC = (ThreadParameter *)malloc(n * sizeof(ThreadParameter));
	semFullA = (HANDLE *)malloc(n * sizeof(HANDLE));
	semEmptyA = (HANDLE *)malloc(n * sizeof(HANDLE));
	semMEpA = (HANDLE *)malloc(n * sizeof(HANDLE));
	semMEcA = (HANDLE *)malloc(n * sizeof(HANDLE));
	semFullB = (HANDLE *)malloc(n * sizeof(HANDLE));
	semEmptyB = (HANDLE *)malloc(n * sizeof(HANDLE));
	semMEpB = (HANDLE *)malloc(n * sizeof(HANDLE));
	semMEcB = (HANDLE *)malloc(n * sizeof(HANDLE));
	queueA = (queue_t *)malloc(n * sizeof(queue_t));
	queueB = (queue_t *)malloc(n * sizeof(queue_t));
	filesIn = (HANDLE *)malloc(n * sizeof(HANDLE));
	filesOut = (HANDLE *)malloc(n * sizeof(HANDLE));
	csFileIn = (CRITICAL_SECTION *)malloc(n * sizeof(CRITICAL_SECTION));
	csFileOut = (CRITICAL_SECTION *)malloc(n * sizeof(CRITICAL_SECTION));

	//creation of semaphores
	for (int i = 0; i < n; i++) {
		semFullA[i] = CreateSemaphore(NULL, 0, n, NULL);
		semEmptyA[i] = CreateSemaphore(NULL, n, n, NULL);
		semMEpA[i] = CreateSemaphore(NULL, 1, 1, NULL);
		semMEcA[i] = CreateSemaphore(NULL, 1, 1, NULL);
		semFullB[i] = CreateSemaphore(NULL, 0, n, NULL);
		semEmptyB[i] = CreateSemaphore(NULL, n, n, NULL);
		semMEpB[i] = CreateSemaphore(NULL, 1, 1, NULL);
		semMEcB[i] = CreateSemaphore(NULL, 1, 1, NULL);
		
	}

	//creation of critical sections
	for (int i = 0; i < n; i++) {
		InitializeCriticalSection(&csFileIn[i]);
		InitializeCriticalSection(&csFileOut[i]);
	}

	//creation of queues
	for (int i = 0; i < n; i++) {
		queueA[i].size = n;
		queueA[i].buffer = (record *)malloc(n * sizeof(record));
		queueA[i].nE = 0;
		queueA[i].in = 0;
		queueA[i].out = 0;
		queueB[i].size = n;
		queueB[i].buffer = (record *)malloc(n * sizeof(record));
		queueB[i].nE = 0;
		queueB[i].in = 0;
		queueB[i].out = 0;
	}

	//open input files
	for (int i = 0; i < n; i++) {
		_stprintf(tmpName, _T("FileI%d.bin"), i);
		filesIn[i] = CreateFile(tmpName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (filesIn[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Impossible open the input file n:%d\n"), i);
			return -1;
		}
	}

	//create output files
	for (int i = 0; i < n; i++) {
		_stprintf(tmpName, _T("FileO%d.bin"), i);
		filesOut[i] = CreateFile(tmpName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (filesOut[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Impossible create the output file n:%d\n"), i);
			return -1;
		}
	}

	//create n thA
	for (int i = 0; i < n; i++) {
		thIdA[i] = i;
		thParA[i].id = i;	
		threadsA[i] = CreateThread(NULL, 0, thA, &thParA[i], NULL, &thIdA[i]);
		if (threadsA[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of thread a \n"));
			return -1;
		}
	}

	//create n thB
	for (int i = 0; i < n; i++) {
		thIdB[i] = i;
		thParB[i].id = i;
		threadsB[i] = CreateThread(NULL, 0, thB, &thParB[i], NULL, &thIdB[i]);
		if (threadsB[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of thread b \n"));
			return -1;
		}
	}

	//create n thC
	for (int i = 0; i < n; i++) {
		thIdC[i] = i;
		thParC[i].id = i;
		threadsC[i] = CreateThread(NULL, 0, thC, &thParC[i], NULL, &thIdC[i]);
		if (threadsC[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of thread c \n"));
			return -1;
		}
	}

	/* Wait for the thA to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &threadsA[i], TRUE, INFINITE);

	/* Wait for the thB to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &threadsB[i], TRUE, INFINITE);

	/* Wait for the thC to complete */
	for (int i = 0; i < n; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &threadsC[i], TRUE, INFINITE);



	return 0;
}