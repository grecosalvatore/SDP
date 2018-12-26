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

#define iterations 1000
#define sentinel -999999

typedef struct _ThreadParameter {
	int id;
	LPTSTR filename;
	HANDLE semFull, semEmpty, semMEp, semMEc;
} ThreadParameter;

typedef struct queue_s {
	DWORD *buffer;
	DWORD in;
	DWORD out;
	DWORD size;
	DWORD nE;
} queue_t;

//global variables
DWORD n, nC, nP, t;
queue_t queue;
HANDLE semFull, semEmpty, semMEp, semMEc;

void produce() {
	float r;
	DWORD wt;
	//producer sleep [0-T] seconds
	r = ((float)(rand())) / ((float)(RAND_MAX));
	wt = (DWORD)(r * t);
	Sleep(wt * 1000);

	return;
}

void consume() {
	float r;
	DWORD wt;
	//producer sleep [0-T] seconds
	r = ((float)(rand())) / ((float)(RAND_MAX));
	wt = (DWORD)(r * t);
	Sleep(wt * 1000);
	return;
}

void enqueue(DWORD id) {
	DWORD val;

	val = rand();
	val = val * id;
	queue.buffer[queue.in] = val;
	queue.in = (queue.in + 1) % queue.size;
	queue.nE = queue.nE + 1;
	_tprintf(_T("Element produced\n"));
	return;
}

void dequeue(BOOLEAN *termination) {
	DWORD val;
	val = queue.buffer[queue.out];
	queue.nE = queue.nE - 1;
	_tprintf(_T("   - Consumer: val=%d pos=%d nE=%d\n"),
		val, queue.out, queue.nE);
	queue.out = (queue.out + 1) % queue.size;
	if (val == sentinel) {
		*(termination) = TRUE;
		_tprintf(_T("Termination sentinel recived\n"));
	}
	return;
}

DWORD WINAPI producer(PVOID pThParam) {
	ThreadParameter *p;

	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("Producer %d created\n"), p->id);

	for (int i =0 ; i < iterations ; i++){
		produce();
		WaitForSingleObject(semEmpty, INFINITE);
		WaitForSingleObject(semMEp, INFINITE);
		enqueue(p->id);
		ReleaseSemaphore(semMEp, 1, NULL);
		ReleaseSemaphore(semFull, 1, NULL);
		
	}

	ExitThread(0);
}

DWORD WINAPI consumer(PVOID pThParam) {
	ThreadParameter *p;
	BOOLEAN termination;
	//cast of parameter
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("Consumer %d created\n"), p->id);

	while (1) {
	
		WaitForSingleObject(semFull, INFINITE);
		WaitForSingleObject(semMEc, INFINITE);
		dequeue(&termination);
		ReleaseSemaphore(semMEc, 1, NULL);
		ReleaseSemaphore(semEmpty, 1, NULL);
		consume();
		if (termination == TRUE) {
			ExitThread(0);
		}
	}

	ExitThread(0);
}



INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE *p, *c;
	

	DWORD *thIdP, *thIdC;
	ThreadParameter *thParP, *thParC;
	if (argc != 5) {
		_tprintf(_T("Error in the number of parameters %s <nP> <nC> <N> <T> \n"), argv[0]);
		return -1;
	}
	nP = _ttoi(argv[1]);
	nC = _ttoi(argv[2]);
	n = _ttoi(argv[3]); // size of buffer

	//allocation of memory
	c = (HANDLE *)malloc(nC * sizeof(HANDLE));
	p = (HANDLE *)malloc(nP * sizeof(HANDLE));
	thIdC = (DWORD *)malloc(nC * sizeof(DWORD));
	thIdP = (DWORD *)malloc(nP * sizeof(DWORD));
	thParP = (ThreadParameter *)malloc(nP * sizeof(ThreadParameter));
	thParC = (ThreadParameter *)malloc(nC * sizeof(ThreadParameter));

	//creation of semaphores
	semFull = CreateSemaphore(NULL, 0, n, NULL);
	semEmpty = CreateSemaphore(NULL, n, n, NULL);
	semMEp = CreateSemaphore(NULL, 1, 1, NULL);
	semMEc = CreateSemaphore(NULL, 1, 1, NULL);

	//creation of the queue
	queue.size = n;
	queue.buffer = (DWORD *)malloc(n * sizeof(DWORD));
	queue.nE = 0;

	//create n producers
	for (int i = 0; i < nP; i++) {
		thIdP[i] = i;
		thParP[i].id = i;
		thParP[i].semEmpty = semEmpty;
		thParP[i].semFull = semFull;
		thParP[i].semMEc = semMEc;
		thParP[i].semMEp = semMEp;
		p[i] = CreateThread(NULL, 0, producer, &thParP[i], NULL, &thIdP[i]);
		if (p[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of producer \n"));
			return -1;
		}
	}

	//create n consumers
	for (int i = 0; i < nC; i++) {
		thIdC[i] = i;
		thParC[i].id = i;
		c[i] = CreateThread(NULL, 0, consumer, &thParC[i], NULL, &thIdC[i]);
		if (c[i] == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error in the creation of consumer \n"));
			return -1;
		}
	}

	/* Wait for the producers to complete */
	for (int i = 0; i < nP; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &p[i], TRUE, INFINITE);

	//send sentinel to cosnumer
	for (int i = 0; i < nC;i++) {
		WaitForSingleObject(semEmpty, INFINITE);
		WaitForSingleObject(semMEp, INFINITE);
		queue.buffer[queue.in] = sentinel;
		queue.in = (queue.in + 1) % queue.size;
		queue.nE = queue.nE + 1;
		ReleaseSemaphore(semMEp, 1, NULL);
		ReleaseSemaphore(semFull, 1, NULL);
		_tprintf(_T("Sentinel sended\n"));
	}

	/* Wait for the consumers to complete */
	for (int i = 0; i < nC; i += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(n - i, MAXIMUM_WAIT_OBJECTS), &c[i], TRUE, INFINITE);
		
	return 0;
}