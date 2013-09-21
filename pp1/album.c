#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>

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
		blocking_display_image(tb_name);
	}

	return 0;
}

void blocking_convert_to_thumbnail(char *image, char *name) {
	int rc = fork();
	if (0 ==rc) {
		int cs = execlp("/usr/bin/convert", "convert", "-geometry 10\%", name, NULL);
		exit(-1); //TODO: error message
	} else {
		int status;
		waitpid(rc, &status, 0);
	}
}

void blocking_display_image(char *image) {
	int rc = fork();
	if (0 == rc) {
		int es = execlp("/usr/bin/display", "display", image, NULL);
		exit(-1); //TODO: error message
	} else {
		int status;
		waitpid(rc, &status, 0);
	}
}

void strip_image_name(char *name, char *image_path) {
	char *last_slash = strrchar(image_path, '/');
	strcpy(name, last_slash);
}
