#include "bridge.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

const int MAX_CARS = 4;

bridge_t BRIDGE = { PTHREAD_MUTEX_INITIALIZER, {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER}, {0, 0}};

int main(int argc, char * argv[]) {
	if (argc != 2) {
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


	/*
	printf("%s\n", DIR_STRING[TO_HANOVER]);
	printf("%s\n", DIR_STRING[OtherDirection(TO_HANOVER)]);
	printf("%s\n", DIR_STRING[TO_NORWICH]);
	printf("%s\n", DIR_STRING[OtherDirection(TO_NORWICH)]);
	*/

	/*
	pthread_t test;
	int rc;
	int direction = TO_HANOVER;
	rc = pthread_create(&test, NULL, OneVehicle, (void *) direction);
	if (rc) {
		printf("Error creating thread.\n");
		exit(-1);
	}
	pthread_join(test, NULL);
	*/
	for (unsigned int i = 0; i < 3; i++) {
		RandomArrivals(i, num_cars);
	}

	pthread_mutex_destroy(&BRIDGE.m);
	exit(EXIT_SUCCESS);
}

void RandomArrivals(unsigned int _seed, const int num_cars) {
	printf("======= STARTING WITH SEED %d ========\n", _seed);
	pthread_t cars[num_cars];
	int seed = _seed;
	for (int i = 0; i < num_cars; i++) {
		int args[3];
		args[0] = rand_r(&seed) % 2; // DIRECTION
		args[1] = (rand_r(&seed) % MAX_SLEEP) + 1; // TRAVEL TIME
		int rc = pthread_create(&cars[i], NULL, OneVehicle, (void *) args);
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
	int *args = (int *) _args;
	const int direction = args[0];
	const int travel_time = args[1];
	ArriveBridge(direction);
	OnBridge(direction);
	sleep(travel_time);
	ExitBridge(direction);

	pthread_exit(NULL);
}

void ArriveBridge(const int direction) {
	printf("Car arrived at bridge towards %s.\n", DIR_STRING[direction]);
	pthread_mutex_lock(&BRIDGE.m);
	while ((BRIDGE.cars[OtherDirection(direction)] > 0) || (BRIDGE.cars[direction] >= MAX_CARS)) {
		/*
		printf("-----Arrived, can't embark.\n");
		printf("-----Bridge State: \n\t %d car(s) to Hanover \n\t %d car(s) to Norwich\n", BRIDGE.cars[TO_HANOVER], BRIDGE.cars[TO_NORWICH]);
		*/
		pthread_cond_wait(&BRIDGE.cvars[direction], &BRIDGE.m);
	}
	++(BRIDGE.cars[direction]);
	pthread_mutex_unlock(&BRIDGE.m);
}

void OnBridge(const int direction) {
	pthread_mutex_lock(&BRIDGE.m);
	printf("Car on bridge to %s.\n", DIR_STRING[direction]);
	printf("Bridge State: \n\t %d car(s) to Hanover \n\t %d car(s) to Norwich\n", BRIDGE.cars[TO_HANOVER], BRIDGE.cars[TO_NORWICH]);
	if (BRIDGE.cars[TO_NORWICH] && BRIDGE.cars[TO_HANOVER]) {
		printf("ERROR: COLLISION!!");
	}
	pthread_mutex_unlock(&BRIDGE.m);
}

void ExitBridge(const int direction) {
	pthread_mutex_lock(&BRIDGE.m);
	--(BRIDGE.cars[direction]);
	printf("Car to %s disembarked.\n", DIR_STRING[direction]);
	if (BRIDGE.cars[direction] == 0) {
		pthread_cond_signal(&BRIDGE.cvars[OtherDirection(direction)]);
	}
	pthread_cond_signal(&BRIDGE.cvars[direction]);
	pthread_mutex_unlock(&BRIDGE.m);
}

inline int OtherDirection(const int direction) {
	return !direction;
}