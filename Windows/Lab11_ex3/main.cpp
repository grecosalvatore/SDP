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



typedef struct _ThreadParameter {
	DWORD time_A;
	DWORD time_T;
	DWORD n_car;
} ThreadParameter;

typedef struct _CarParameter {
	DWORD direction;
	DWORD time;
	DWORD n;
} CarParameter;



//global variables
DWORD time_A_L2R, time_A_R2L, time_T_L2R, time_T_R2L, number_L2R, number_R2L;
HANDLE sem1, sem2, busy;
DWORD n1 = 0, n2 = 0;

DWORD WINAPI car(PVOID pThParam) {
	CarParameter *p;
	//direction 0 l2r / 1 r2l;
	p = (CarParameter *)pThParam;
	if (p->direction == 0) {
		//left to right
		_tprintf(_T("DIRECTION: LEFT TO RIGHT CAR:%d ENTER IN THE TUNNEL \n"), p->n);
	}
	else {
		_tprintf(_T("DIRECTION: RIGHT TO LEFT CAR:%d ENTER IN THE TUNNEL \n"), p->n);
	}
	Sleep(p->time*1000);
	if (p->direction == 0) {
		//left to right
		_tprintf(_T("DIRECTION: LEFT TO RIGHT CAR:%d EXIT IN THE TUNNEL \n"), p->n);
	}
	else {
		_tprintf(_T("DIRECTION: RIGHT TO LEFT CAR:%d EXIT IN THE TUNNEL \n"), p->n);
	}
	ExitThread(0);
}


DWORD WINAPI Left2Right(PVOID pThParam) {
	ThreadParameter *p;
	CarParameter cp;
	HANDLE hCar;
	
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("LEFT TO RIGHT CREATED %d \n"), p->n_car);
	for (int i = 0; i < p->n_car; i++) {
		Sleep(p->time_A*1000);
		WaitForSingleObject(sem1, INFINITE);
		n1++;
		if (n1 == 1)
			WaitForSingleObject(busy, INFINITE);
		ReleaseSemaphore(sem1, 1, NULL);
		
		//create thread car
		cp.direction = 0;
		cp.time = p->time_T;
		cp.n = i;
		hCar = CreateThread(NULL, 0, car, &cp, 0, NULL);
		if (hCar == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error: impossible create left to right\n"));
			return -1;
		}
		WaitForSingleObject(hCar, INFINITE);
		CloseHandle(hCar);

		WaitForSingleObject(sem1, INFINITE);
		n1--;
		if (n1 == 0)
			ReleaseSemaphore(busy, 1, NULL);
		ReleaseSemaphore(sem1, 1, NULL);
	}
	
	ExitThread(0);
}

DWORD WINAPI Right2Left(PVOID pThParam) {
	ThreadParameter *p;
	CarParameter cp;
	HANDLE hCar;
	
	p = (ThreadParameter *)pThParam;
	_tprintf(_T("RIGHT TO LEFT CREATED n:%d \n"), p->n_car);
	for (int i = 0; i < p->n_car; i++) {
		Sleep(p->time_A*1000);
		WaitForSingleObject(sem2, INFINITE);
		n2++;
		if (n2 == 1) {
			WaitForSingleObject(busy, INFINITE);
		}
		ReleaseSemaphore(sem2, 1, NULL);
		
		//create thread car
		cp.direction = 1;
		cp.time = p->time_T;
		cp.n = i;
		hCar = CreateThread(NULL, 0, car, &cp, 0, NULL);
		if (hCar == INVALID_HANDLE_VALUE) {
			_tprintf(_T("Error: impossible create left to right\n"));
			return -1;
		}
		WaitForSingleObject(hCar, INFINITE);
		CloseHandle(hCar);

		WaitForSingleObject(sem2, INFINITE);
		n2--;
		if (n2 == 0) {
			ReleaseSemaphore(busy, 1, NULL);
		}
		ReleaseSemaphore(sem2, 1, NULL);
	}

	ExitThread(0);
}



INT _tmain(INT argc, LPTSTR argv[]) {
DWORD time_A_L2R, time_A_R2L, time_T_L2R, time_T_R2L, number_L2R, number_R2L;
HANDLE L2R, R2L;
DWORD L2RID,R2LID;
ThreadParameter L2Rpar, R2Lpar;

	if (argc != 7) {
		_tprintf(_T("Error in the number of parameters %s <time_A_L2R> <time_A_R2L> <time_T_L2R> <time_T_R2L> <number_L2R> <number_R2L> \n"), argv[0]);
		return -1;
	}
	time_A_L2R = _ttoi(argv[1]);
	time_A_R2L = _ttoi(argv[2]);
	time_T_L2R = _ttoi(argv[3]);
	time_T_R2L = _ttoi(argv[4]);
	number_L2R = _ttoi(argv[5]);
	number_R2L = _ttoi(argv[6]);
	
	L2RID = 1;
	R2LID = 2;

	//creation of semaphores
	sem1 = CreateSemaphore(NULL, 1,1, NULL);
	sem2 = CreateSemaphore(NULL, 1, 1, NULL);
	busy = CreateSemaphore(NULL, 1, 1, NULL);

	L2Rpar.time_A = time_A_L2R;
	L2Rpar.time_T = time_T_L2R;
	L2Rpar.n_car = number_L2R;
	//create left to right thread
	L2R = CreateThread(NULL,0,Left2Right,&L2Rpar,0, &L2RID);
	if (L2R == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error: impossible create left to right\n"));
		return -1;
	}

	R2Lpar.time_A = time_A_R2L;
	R2Lpar.time_T = time_T_R2L;
	R2Lpar.n_car = number_R2L;
	R2L = CreateThread(NULL, 0, Right2Left, &R2Lpar, 0, &R2LID);
	if (R2L == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error: impossible create right to left\n"));
		return -1;
	}

	WaitForSingleObject(R2L, INFINITE);
	WaitForSingleObject(L2R, INFINITE);

	return 0;
}