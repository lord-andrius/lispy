#include <stdio.h>
#include <string.h>
#include "editline.h"

/*** defines ***/
#define LISPY_VERSION "0.0.1"
#define INPUT_BUFFER_SIZE 2048

/*** declare a input buffer of [INPUT_BUFFER_SIZE] ***/
static char input_buffer[INPUT_BUFFER_SIZE];

/*** input ***/
#if 0
int readline(char **line) {
	*line = fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
	if (*line == (void *)0 || (*line)[strlen(*line) - 1] != '\n') {
		return -1;
	}
	return 0;
}
#endif

/*** output **/
void write_welcome_message(void) {
	printf("Lispy version %s\n", LISPY_VERSION);
	puts("Press Ctrl+c or Ctrl+d to exit.");
}

void write_prompt(void) {
	fputs("lispy> ", stdout);
	fflush(stdout);
}

int main(void) {
	char *line;
	write_welcome_message();
	do {
		write_prompt();
		if (readline(&line) == 0 && line != (void *)0) { /* CHECK */
			printf("%s\n", line);
		}
	} while (line != (void *)0);
}