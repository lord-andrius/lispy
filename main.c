#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "editline.h"
#include "vendor/mpc/mpc.h"

/*** defines ***/
#define LISPY_VERSION "0.0.1"
#define INPUT_BUFFER_SIZE 2048



/*** output **/
void write_welcome_message(void) {
	printf("Lispy version %s\n", LISPY_VERSION);
	puts("Press Ctrl+c or Ctrl+d to exit.");
}


int main(void) {
	mpc_parser_t *number = mpc_new("number");
	mpc_parser_t *operator = mpc_new("operator");
	mpc_parser_t *expression = mpc_new("expression");
	mpc_parser_t *lispy = mpc_new("lispy");
	mpc_result_t result;

	

	mpc_err_t * erro = mpca_lang(
		MPCA_LANG_DEFAULT,
		"number :/^-?[0-9]+(\\.[0-9]+)?$/;"
		"operator :'+'|'-'|'*'|'/';"
		"expression :<number>|\"(\"<operator>/-?[0-9]+(\\.[0-9]+)?/\")\";"
		"lispy :/^/ <operator> <expression>+ /$/;",
		number,
		operator,
		expression,
		lispy
	);


	char *line;
	write_welcome_message();
	do {
		if (readline(&line, "lispy> ") == 0 && line != (void *)0) { /* CHECK */
			/* printf("%s\n", line); */
			if (mpc_parse("prompt", line, lispy, &result)) {
				mpc_ast_print(result.output);
				mpc_ast_delete(result.output);
			} else {
				mpc_err_print(result.error);
				mpc_err_delete(result.error);
			}
		}
	} while (line != (void *)0);
}