#include "bridge.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int MAX_CARS;

bridge_t BRIDGE;

int main(int argc, char * argv[]) {
	if (argc != 2) {
		printf("%s\n", USAGE);
		exit(1);
	}

	MAX_CARS = strtol(argv[1], NULL, 10);

	// Check for strtol parse errors
	if ((errno == ERANGE) || (errno != 0 && MAX_CARS == 0)) {
		printf("Error parsing max_cars.\n");
		printf("%s\n", USAGE);
		exit(1);
	}

	if (MAX_CARS <= 0) {
		printf("max_cars must be greater than 0.\n");
		printf("%s\n", USAGE);
		exit(1);
	}

	pthread_mutex_init(BRIDGE.m, NULL);
	pthread_cond_init(BRIDGE.cvars[TO_NORWICH], NULL);
	pthread_cond_init(BRIDGE.cvars[TO_HANOVER], NULL);

	pthread_t test;
	int rc;
	int direction = TO_HANOVER;
	rc = pthread_create(&test, NULL, OneVehicle, &direction);
	if (rc) {
		printf("Error creating thread.\n");
		exit(-1);
	}
	pthread_join(test, NULL);


	pthread_mutex_destroy(BRIDGE.m);
	exit(EXIT_SUCCESS);
}

void *OneVehicle(void * v_direction) {
	const int direction =  (int) v_direction;
	ArriveBridge(direction);
	OnBridge(direction);
	ExitBridge(direction);

	pthread_exit(NULL);
}

void ArriveBridge(const int direction) {
	printf("Car arrived at bridge towards %s\n.", DIR_STRING[direction]);
	pthread_mutex_lock(BRIDGE.m);
	while ((BRIDGE.cars[!direction] > 0) || (BRIDGE.cars[direction] >= MAX_CARS)) {
		pthread_cond_wait(BRIDGE.cvars[direction], BRIDGE.m);
	}
	++(BRIDGE.cars[direction]);
	pthread_mutex_unlock(BRIDGE.m);
}

void OnBridge(const int direction) {
	pthread_mutex_lock(BRIDGE.m);
	printf("Car on bridge to %s\n.", DIR_STRING[direction]);
	printf("Bridge State: \n\t %d cars to Hanover \n\t %d cars to Norwich\n", BRIDGE.cars[TO_HANOVER], BRIDGE.cars[TO_NORWICH]);
	pthread_mutex_unlock(BRIDGE.m);
}

void ExitBridge(const int direction) {
	pthread_mutex_lock(BRIDGE.m);
	--(BRIDGE.cars[direction]);
	printf("Car to %s disembarked.\n", DIR_STRING[direction]);
	if (BRIDGE.cars[direction] == 0) {
		pthread_cond_signal(BRIDGE.cvars[!direction]);
	}
	pthread_cond_signal(BRIDGE.cvars[direction]);
	pthread_mutex_unlock(BRIDGE.m);
}