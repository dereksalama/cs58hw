/*
bridge.h
Derek Salama
CS58 Programming Project 2
10/3/2013

Contains bridge struct, constants and function prototypes.
*/

#ifndef BRIDGE_H_
#define BRIDGE_H_

#include <pthread.h>

#define USAGE "./bridge num_cars num_tries where num_cars and num_tries > 0"
#define TO_NORWICH 0
#define TO_HANOVER 1
#define MAX_SLEEP 5
#define MAX_CARS 4

const char * DIR_STRING[] = { "NORWICH", "HANOVER" };

// Gaurded by m 
typedef struct bridge {
	pthread_mutex_t m;
	pthread_cond_t cvars[2];
	int cars_on_bridge[2];
	int cars_waiting[2];
} bridge_t;

typedef struct thread_args {
	int direction;
	int id;
	int travel_time;
} thread_args_t;

/* Starting function for thread */
void *OneVehicle(void *args);

/* Car has arrived at bridge. */
void ArriveBridge(const int direction, const int id);

/* Car allowed onto the bridge. Prints state. */
void OnBridge(const int direction, const int id);

/* Car exits bridge. */
void ExitBridge(const int direction, const int id);

/* Randomly generate car arrivals and travel times. */
void RandomArrivals(int _seed, const int num_cars);

int OtherDirection(const int direction);

#endif // BRIDGE_H_