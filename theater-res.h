#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {zoneA, zoneB} zones;
typedef enum {false, true} bool;

void* reservation(void*);

void phone_operator(unsigned int*, int, zones, int, int*, bool*);
void cashier(unsigned int*, int, zones, int, int, int);

int seats_searching(zones, int, bool*);
char* seats_string(int, int, char*);

void mutex_init(pthread_mutex_t*, const pthread_mutexattr_t *);
void cond_init(pthread_cond_t*, const pthread_condattr_t*);

void mutex_lock(pthread_mutex_t*);
void mutex_unlock(pthread_mutex_t*);

void cond_wait(pthread_cond_t*, pthread_mutex_t*);
void cond_signal(pthread_cond_t*);

void mutex_destroy(pthread_mutex_t*);
void cond_destroy(pthread_cond_t*);

#define N_tel 3
#define N_cash 2

#define N_seat 10
#define N_zoneA 10
#define N_zoneB 20
#define N_totalSeats (N_zoneA + N_zoneB) * N_seat

#define zoneA_low 0
#define zoneA_upper N_zoneA
#define zoneB_low zoneA_upper
#define zoneB_upper N_zoneA + N_zoneB

#define P_zoneA 0.3f
#define P_zoneB 0.7f
#define C_zoneA 30
#define C_zoneB 20
#define N_seatlow 1
#define N_seathigh 5

#define t_reslow 1
#define t_reshigh 5
#define t_seatlow 5
#define t_seathigh 13
#define t_cashlow 4
#define t_cashhigh 8

#define P_cardsuccess 0.9f
