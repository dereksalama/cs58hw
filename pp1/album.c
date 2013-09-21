#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>

#include "input_prompt.h"

#define INPUT_BUFFER_LEN 32

// Function Prototypes
void blocking_convert_to_thumbnail(char *image, char *name);

int display_image(char *image);

void convert_to_medium_output(char *image, char *name, int rotate_degrees);

int rotation_prompt();

void strip_image_name(char *name, char *image_path);

int main(int argc, char *argv[]) {

	//TODO: handle case for no files matched

	for (int i = 1; i < argc; i++) {
		char *image = argv[i];
		char base_name[strlen(image)];
		strip_image_name(base_name, image);

		char tb_name[strlen(base_name) + 3];
		strcpy(tb_name, "tb-");
		strcat(tb_name, base_name);

		blocking_convert_to_thumbnail(image, tb_name);
		int display_pid = display_image(tb_name);
		int rotate_degrees = rotation_prompt();

		char md_name[strlen(base_name) + 3];
		strcpy(md_name, "md-");
		strcat(md_name, base_name);

		convert_to_medium_output(image, md_name, rotate_degrees);

		int status;
		waitpid(display_pid, &status, 0); // Wait for image to close before progressing
	}

	return 0;
}

int rotation_prompt() {
	char buffer[INPUT_BUFFER_LEN];
	while (1) {
		input_string("Rotate image? (c (clockwise)/cc (counterclockwise)/f (flip)/ n)", buffer, INPUT_BUFFER_LEN);
		if (0 == strcasecmp(buffer, "c")) {
			return 90;
		} else if (0 == strcasecmp(buffer, "cc")) {
			return 270;
		} else if (0 == strcasecmp(buffer, "f")) {
			return 180;
		} else if (0 == strcasecmp(buffer, "n")) {
			return 0;
		} else {
			printf("%s\n", "Invalid input.");
		}
	}

	return -1; //should never get here
}

void convert_to_medium_output(char *image, char *name, int rotate_degrees) {
	int rc = fork();
	if (0 == rc) {
		int cs = execlp("/usr/bin/convert", "convert", "-geometry", "25\%", "-rotate", rotate_degrees, image, name, NULL);
		printf("Failed to create output image: %d\n", cs);
		exit(-1); //TODO: error message
	}
}

void blocking_convert_to_thumbnail(char *image, char *name) {
	int rc = fork();
	if (0 ==rc) {
		int cs = execlp("/usr/bin/convert", "convert", "-geometry", "10\%", image, name, NULL);
		exit(-1); //TODO: error message
	} else {
		int status;
		waitpid(rc, &status, 0);
	}
}

// Return the child process pid so it can be killed when necessary.
int display_image(char *image) {
	int rc = fork();
	if (0 == rc) {
		int es = execlp("/usr/bin/display", "display", image, NULL);
		return -1; //TODO: error message
	} else {
		return rc;
	}
}

void strip_image_name(char *name, char *image_path) {
	char *last_slash = strrchr(image_path, '/');
	++last_slash; // Advance past '/'
	strcpy(name, last_slash);
}
