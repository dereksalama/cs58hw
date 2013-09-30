CFLAGS = -g -std=c99 -Wall -pthread

bridge: bridge.c bridge.h
	gcc $(CFLAGS) -o bridge bridge.c bridge.h

clean:
	rm -f bridge