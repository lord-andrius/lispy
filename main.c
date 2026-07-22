#include <stdio.h>
#include <string.h>
#include "editline.h"

/*** defines ***/
#define LISPY_VERSION "0.0.1"
#define INPUT_BUFFER_SIZE 2048



/*** output **/
void write_welcome_message(void) {
	printf("Lispy version %s\n", LISPY_VERSION);
	puts("Press Ctrl+c or Ctrl+d to exit.");
}


int main(void) {
	char *line;
	write_welcome_message();
	do {
		if (readline(&line, "lispy> ") == 0 && line != (void *)0) { /* CHECK */
			printf("%s\n", line);
		}
	} while (line != (void *)0);
}