#ifndef BRIDGE_H_
#define BRIDGE_H_

#include <pthread.h>

#define USAGE "./bridge num_cars where num_cars > 0"
#define TO_NORWICH 0
#define TO_HANOVER 1
#define MAX_SLEEP 5

const char * DIR_STRING[] = { "NORWICH", "HANOVER" };

// Gaurded by m 
// Invariants TODO
typedef struct bridge {
	pthread_mutex_t m;
	pthread_cond_t cvars[2];
	int cars[2];
} bridge_t ;

void *OneVehicle(void *v_direction);
void ArriveBridge(const int direction);
void OnBridge(const int direction);
void ExitBridge(const int direction);
void RandomArrivals(unsigned int _seed, const int num_cars);
int OtherDirection(const int direction);

#endif // BRIDGE_H_