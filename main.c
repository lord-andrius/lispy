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


mpc_val_t *fold_numbers(int count, mpc_val_t **numbers) {
	char **number_list = calloc(count + 1, sizeof(char *));
	int number_list_index = 0;
	int i;
	for (i = 0; i < count; i++) {
		char *number_or_space = ((char **)numbers)[i];
		if (number_or_space[0] != ' ') {
			char *alocated_number = calloc(strlen(number_or_space) + 1, sizeof(char));
			memcpy(alocated_number, number_or_space, strlen(number_or_space));
			number_list[number_list_index++] = alocated_number;
		}
	}
	number_list[number_list_index] = (void *)0;
	return number_list;
}

mpc_val_t *fold_operation(int count, mpc_val_t **operation_parts) {
	char operator = ((char **)operation_parts)[0][0];
	char **operands = (char **)(operation_parts[1]);
	char *operand;
	static double result = 0;
	result = 0;
	for (operand = *operands; operand != (void *)0; operand = *operands) {
		operands++;
		double operand_double = strtod(operand, NULL);
		if (operand == *((char **)(operation_parts[1]))) {
			result = operand_double;
		} else {
			switch (operator)
			{
			case '+':
				result += operand_double;
				break;
			case '-':
				result -= operand_double;
				break;
			case '*':
				result *= operand_double;
				break;
			case '/':
				result /= operand_double;
				break;
			}
		}
		
	}
	return (mpc_val_t *)&result;
}

int main(void) {
	mpc_parser_t *operator = mpc_or(4, mpc_sym("+"), mpc_sym("-"), mpc_sym("*"), mpc_sym("/"));
	mpc_parser_t *number = mpc_or(2, mpc_sym(" "), mpc_real());
	mpc_parser_t *operands = mpc_many(fold_numbers, number);
	mpc_parser_t *operation = mpc_and(2, fold_operation, operator, operands);
	mpc_result_t result;


	char *line;
	write_welcome_message();
	do {
		if (readline(&line, "lispy> ") == 0 && line != (void *)0) { /* CHECK */
			/* printf("%s\n", line); */
			if (mpc_parse("prompt", line, operation, &result)) {
				printf("%.2f\n", *((double *)result.output)); 
			} else {
				printf("{invalid}\n");
			}
		}
	} while (line != (void *)0);
}