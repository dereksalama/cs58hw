/*
bridge.h
Derek Salama
CS58 Programming Project 2
10/3/2013

Contains main() (and everything else) for PP2.
*/
#define _POSIX_SOURCE // rand_r
#include "bridge.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

bridge_t BRIDGE = { PTHREAD_MUTEX_INITIALIZER, {PTHREAD_COND_INITIALIZER, 
	PTHREAD_COND_INITIALIZER}, {0, 0}};

int main(int argc, char * argv[]) {
	if (argc != 3) {
		printf("%s\n", USAGE);
		exit(1);
	}

	int num_cars = strtol(argv[1], NULL, 10);

	// Check for strtol parse errors
	if ((errno == ERANGE) || (errno != 0 && MAX_CARS == 0)) {
		printf("Error parsing num_cars.\n");
		printf("%s\n", USAGE);
		exit(1);
	}

	if (MAX_CARS <= 0) {
		printf("num_cars must be greater than 0.\n");
		printf("%s\n", USAGE);
		exit(1);
	}

	int num_tries = strtol(argv[2], NULL, 10);

	// Check for strtol parse errors
	if ((errno == ERANGE) || (errno != 0 && MAX_CARS == 0)) {
		printf("Error parsing num_tries.\n");
		printf("%s\n", USAGE);
		exit(1);
	}

	for (int i = 0; i < num_tries; i++) {
		RandomArrivals(i * i, num_cars);
	}

	pthread_mutex_destroy(&BRIDGE.m);
	exit(EXIT_SUCCESS);
}

void RandomArrivals(int _seed, const int _num_cars) {
	printf("======= STARTING WITH SEED %d ========\n", _seed);
	unsigned int seed = _seed;
	unsigned int num_cars = _num_cars;
	pthread_t cars[num_cars];
	thread_args_t args[num_cars];
	for (int i = 0; i < num_cars; i++) {
		args[i].direction = rand_r(&seed) % 2;
		args[i].travel_time = (rand_r(&seed) % MAX_SLEEP) + 1;
		args[i].id = i;
		int rc = pthread_create(&cars[i], NULL, OneVehicle, (void *) &args[i]);
		if (rc) {
			printf("Error creating thread.\n");
			exit(-1);
		}
	}
	for (int i = 0; i < num_cars; i++) {
		pthread_join(cars[i], NULL);
	}
	printf("========== COMPLETE =============\n");
}

void *OneVehicle(void * _args) {
	thread_args_t *args = (thread_args_t *) _args;
	ArriveBridge(args->direction, args->id);
	OnBridge(args->direction, args->id);
	sleep(args->travel_time);
	ExitBridge(args->direction, args->id);

	pthread_exit(NULL);
}

/* Blocks until car can get on bridge. */
void ArriveBridge(const int direction, const int id) {
	pthread_mutex_lock(&BRIDGE.m);
	printf("Car %d arrived at bridge towards %s.\n", id, DIR_STRING[direction]);
	++(BRIDGE.cars_waiting[direction]);
	/* Wait if there are too many cars or one from other direction. */
	while ((BRIDGE.cars_on_bridge[OtherDirection(direction)] > 0) || 
			(BRIDGE.cars_on_bridge[direction] >= MAX_CARS)) {
		pthread_cond_wait(&BRIDGE.cvars[direction], &BRIDGE.m);
	}
	/* Can now get on bridge. */
	--(BRIDGE.cars_waiting[direction]);
	++(BRIDGE.cars_on_bridge[direction]);
	pthread_mutex_unlock(&BRIDGE.m);
}

void OnBridge(const int direction, const int id) {
	pthread_mutex_lock(&BRIDGE.m); // Lock to gauruntee conistant state
	printf("Car %d on bridge to %s.\n", id, DIR_STRING[direction]);
	printf("Bridge State: \n\t %d car(s) to Hanover \n\t %d car(s) to Norwich\n", 
		BRIDGE.cars_on_bridge[TO_HANOVER], BRIDGE.cars_on_bridge[TO_NORWICH]);
	printf("\t %d car(s) waiting to go to Hanover \n\t %d car(s) waiting to go to Norwich\n", 
		BRIDGE.cars_waiting[TO_HANOVER], BRIDGE.cars_waiting[TO_NORWICH]);
	if (BRIDGE.cars_on_bridge[TO_NORWICH] && BRIDGE.cars_on_bridge[TO_HANOVER]) {
		printf("ERROR: COLLISION!!");
	}
	pthread_mutex_unlock(&BRIDGE.m);
}

void ExitBridge(const int direction, const int id) {
	pthread_mutex_lock(&BRIDGE.m);
	--(BRIDGE.cars_on_bridge[direction]);
	printf("Car %d to %s disembarked.\n", id, DIR_STRING[direction]);
	/* If no more cars in this direction, wake all cars in other direction. */
	if (BRIDGE.cars_on_bridge[direction] == 0) {
		pthread_cond_broadcast(&BRIDGE.cvars[OtherDirection(direction)]);
	}
	/* Wake one car in this directiom. If it was waiting, the bridge must have
	   been full, so at most one new car can get on. */
	pthread_cond_signal(&BRIDGE.cvars[direction]);
	pthread_mutex_unlock(&BRIDGE.m);
}

inline int OtherDirection(const int direction) {
	return !direction;
}