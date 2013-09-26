// album.c
// Derek Salama
// CS58 Programming Project 1

// Contains main function for creating the photo album and some helper
// functions.

#define _POSIX_SOURCE  // Necessary for "kill"
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <signal.h>

#include "input_prompt.h"

#define INPUT_BUFFER_LEN 32

// Function Prototypes

// Create thumbnail from source image. The parent process will wait until
// the conversion is complete.
void blocking_convert_to_thumbnail(const char *image, const char *name);

// Display an image with "display". Returns pid of child process.
int display_image(const char *image);

// Converts to medium image, rotated by specified number of degrees.
void convert_to_medium_output(const char *image, const char *name, 
                              const int rotate_degrees);

// Display prompt to ask user if they want to rotate the image. Checks for
// valid response and returns the number of degrees to rotate.
int rotation_prompt();

// Rotate the image by the specified number of degrees. Replaces original
// image.
void rotate_image(const char *image, const int rotate_degrees);

// Find the image name from the end of the path. I.e., if the path is
// foo/bar.jpg, then the name will be bar.jpg.
void strip_image_name(char *name, const char *image_path);

// Print the appropriate html for an image.
void print_image(FILE *fp, const char* md_image, const char* tb_image, 
                 const char* caption);

int main(int argc, char *argv[]) {

  FILE *fp;
  fp = fopen("index.html", "w");
  if (NULL == fp) {
    printf("Error opening index.html\n");
    exit(-1);
  }

  fprintf(fp, "<html>\n");
  fprintf(fp, "<h1>Photo Album<h1>\n");
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

    char *caption = calloc(INPUT_BUFFER_LEN, sizeof(char));
    input_string("Enter caption for image", caption, INPUT_BUFFER_LEN);
    print_image(fp, md_name, tb_name, caption);
    free(caption);
    kill(display_pid, SIGTERM);

    // Wait until after we kill the display rotate the image, since replacing
    // the source file seems to mess with the display process.
    if (0 < rotate_degrees) {
      rotate_image(tb_name, rotate_degrees);
    }

  }

  fprintf(fp, "</html>");
  fclose(fp);

  return 0;
}

void print_image(FILE *fp, const char* md_image, const char* tb_image, 
  const char* caption) {
  fprintf(fp, "<h2>%s</h2>\n", caption);
  fprintf(fp, "<a href=\"%s\"><img src=\"%s\" border=\"1\"></a>\n", 
    md_image, tb_image);
}

int rotation_prompt() {
  char* buffer = calloc(INPUT_BUFFER_LEN, sizeof(char));
  int degrees = 0;
  while (1) {
    input_string(
      "Rotate image? (c (clockwise)/cc (counterclockwise)/f (flip)/ n)",
       buffer, INPUT_BUFFER_LEN);
    if (0 == strcasecmp(buffer, "c")) {
      degrees = 90;
      break;
    } else if (0 == strcasecmp(buffer, "cc")) {
      degrees = 270;
      break;
    } else if (0 == strcasecmp(buffer, "f")) {
      degrees = 180;
      break;
    } else if (0 == strcasecmp(buffer, "n")) {
      break;
    } else {
      printf("%s\n", "Invalid input.");
    }
  }

  free(buffer);
  return degrees;
}

void convert_to_medium_output(const char *image, const char *output_name, 
                              const int rotate_degrees) {
  int rc = fork();
  if (0 == rc) {
    char rotate_str[3];
    sprintf(rotate_str, "%d", rotate_degrees);
    int cs = execlp("convert", "convert", "-geometry", "25\%", "-rotate", 
      rotate_str, image, output_name, NULL);
    perror("Failed to create medium image");
  }
}

void rotate_image(const char *image, const int rotate_degrees) {
  int rc = fork();
  if (0 == rc) {
    char rotate_str[3];
    sprintf(rotate_str, "%d", rotate_degrees);
    int cs = execlp("convert", "convert", "-rotate", rotate_str, image, image, 
      NULL);
    perror("Failed to create medium image");
  }
}

void blocking_convert_to_thumbnail(const char *image, const char *output_name) {
  int rc = fork();
  if (0 ==rc) {
    int cs = execlp("convert", "convert", "-geometry", "10\%", image, 
      output_name, NULL);
    perror("Failed to create thumbnail");
  } else {
    int status;
    waitpid(rc, &status, 0);
  }
}

int display_image(const char *image) {
  int rc = fork();
  if (0 == rc) {
    int es = execlp("display", "display", image, NULL);
    perror("Failed to display image");
  } else {
    return rc;
  }
}

void strip_image_name(char *name, const char *image_path) {
  char *last_slash = strrchr(image_path, '/');
  ++last_slash; // Advance past '/'
  strcpy(name, last_slash);
}
