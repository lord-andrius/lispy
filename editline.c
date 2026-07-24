#include "editline.h"
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x9f)
#define LINE_CAPACITY 256
#define HISTORY_CAPACITY 128



/*** enums ***/
typedef enum {
	PREVIOUS_LINE,
	NEXT_LINE,
	MOVE_LEFT,
	MOVE_RIGHT,
	HOME,
	END,
	CHAR,
	NO_OP,
	DELETE,
	ENTER,
	EXIT,
} input_actions;

/*** structs ***/
typedef struct {
	char raw_byte;
	input_actions action;
} input;


typedef struct {
	struct termios original_termios;
	int terminal_x;
	int terminal_y;
	int terminal_columns;
	int terminal_rows;
	int terminal_x_begin_line;
	int terminal_y_begin_line;
	char line[LINE_CAPACITY];
	int line_index;
	int line_length;
	int should_exit;
	const char *prompt;
	char *history; /* this will be purposefully lekead so don't be a square about it and let the os handles this.*/
	int history_index; /* this should go from 0 to history_length but when it is == to history_length it has a special meaning of adding to the end of history. */
	int history_length; /* how many lines are currently in history. It goes from 0 to HISTORY_CAPACITY*/
} editline_context;


/*** declaration of private functions ***/
static int enter_raw_mode(void);
static int exit_raw_mode(void);
static int get_terminal_coordinate_and_size(int *x, int *y, int *columns, int *rows);
static int handle_low_level_input(input *in);
static int handle_high_level_input(input in);
static void draw_line(void);
static int prepare_for_read_line(const char *prompt);
static void copy_history_line_to_current_line(void);

/*** context ***/
static editline_context context = {0};


/*** raw mode handling ***/
static int enter_raw_mode(void) {
	struct termios modified_termios = {0};
	if (tcgetattr(STDIN_FILENO, &context.original_termios) != 0) {
		return -1;
	}
	/* took this flags based on cfmakeraw() */
	modified_termios = context.original_termios;
	modified_termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | 
								  ISTRIP | INLCR  | IGNCR  | 
								  ICRNL  | IXON);
	modified_termios.c_oflag &= ~OPOST;
	modified_termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	modified_termios.c_cflag &= ~(CSIZE | PARENB);
	modified_termios.c_cflag |= CS8;
	modified_termios.c_cc[VMIN] = 1;
	modified_termios.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &modified_termios) != 0) {
		return -1;
	}
	return 0;
}

static int exit_raw_mode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &context.original_termios) != 0) {
		return -1;
	}
	return 0;
}


static int get_terminal_coordinate_and_size(int *x, int *y, int *columns, int *rows) {
	/* getting the size */
	{
		struct winsize window_size = {0};
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1) {
			return -1;
		}
		*columns = window_size.ws_col;
		*rows = window_size.ws_row;
	}
	
	/* getting the coordinates */
	{
		if (write(STDOUT_FILENO, "\033[6n", strlen("\033[6n")) == -1) { /* device report ansi escape */
			return -1;
		}
		scanf("\033[%d;%dR", y, x);
	}
	return 0;
}

/*** HISTORY ***/
static void copy_history_line_to_current_line(void) {
	const char *history_line = &context.history[context.history_index * LINE_CAPACITY];
	int history_line_length = strlen(history_line);
	context.line_length = history_line_length;
	memcpy(context.line, history_line, history_line_length);
	context.line_index = context.line_length;
}

/*** input ***/
static int handle_low_level_input(input *in) {
	char input_buffer[4] = {0};
	int input_buffer_index = 0;
	if (read(STDIN_FILENO, &input_buffer[input_buffer_index++], 1) == -1) {
		return -1;
	}
	
	if (iscntrl(input_buffer[0])) {
		switch(input_buffer[0]) {
			case CTRL_KEY('c'): /* fallthrough */
			case CTRL_KEY('d'):
				in->action = EXIT;
				break;
			case 127: /* delete */
				in->action = DELETE;
				break;
			case 13: /* enter */
				in->action = ENTER;
				break;
			case 033: /* escape sequences */
				if (read(STDIN_FILENO, &input_buffer[input_buffer_index++], 1) == -1) {
					return -1;
				}

				if (input_buffer[1] == '[') {
					if (read(STDIN_FILENO, &input_buffer[input_buffer_index++], 1) == -1) {
						return -1;
					}
					switch(input_buffer[2]) {
						case 'A':
							in->action = PREVIOUS_LINE;
							break;
						case 'B':
							in->action = NEXT_LINE;
							break;
						case 'D':
							in->action = MOVE_LEFT;
							break;
						case 'C':
							in->action = MOVE_RIGHT;
							break;
						case 'H':
							in->action = HOME;
							break;
						case 'F':
							in->action = END;
							break;
					}
				} else {
					in->action = NO_OP;
				}
				break;
		}
		return 0;
	}
	
	
	if (isalnum(input_buffer[0]) || input_buffer[0] == ' ') {
		in->action = CHAR;
		in->raw_byte = input_buffer[0];
		return 0;
	}
	
	in->action = NO_OP;
	
	return 0;
}

static int handle_high_level_input(input in) {
	static int previous_history_index = 0;
	switch (in.action) {
		case CHAR:
			if (context.line_length + 1 < LINE_CAPACITY) { /* should be fine to put another char*/
				char previous_byte = context.line[context.line_index];
				int i;
				context.line_length++;
				for (i = context.line_index + 1; i <= context.line_length; i++) {
					char tmp = context.line[i];
					context.line[i] = previous_byte;
					previous_byte = tmp;
				}
				context.line[context.line_index++] = in.raw_byte;
			}
			context.history_index = context.history_length;
			break;
		case DELETE:
			{
				if (context.line_length > 0 && context.line_index > 0) {
					int i;
					for (i = context.line_index - 1; i < context.line_length; i++) {
						context.line[i] = context.line[i + 1];
					}
					context.line[context.line_length--] = '\0';
					context.line_index--;
				}
				context.history_index = context.history_length;
			}
			break;
		case MOVE_LEFT:
			if (context.line_index) context.line_index--;
			break;
		case MOVE_RIGHT:
			if (context.line_index < context.line_length) context.line_index++;
			break;
		case HOME:
			context.line_index = 0;
			break;
		case END:
			context.line_index = context.line_length;
			break;
		case ENTER:
			context.line[context.line_length] = '\0';
			if (context.history_index == context.history_length) {
				if (context.history_length < HISTORY_CAPACITY) {
					memcpy(
						context.history + (context.history_index * LINE_CAPACITY),
						context.line,
						LINE_CAPACITY
					);
					context.history_length++;
					context.history_index++;
				} else {
					/* it needs to scroll */
					int i;
					for (i = 0; i < context.history_index; i++) {
						memmove(
							context.history + (i * LINE_CAPACITY),
							context.history + ((i + 1) * LINE_CAPACITY),
							LINE_CAPACITY
						);
					}
					memcpy(
						context.history + ((context.history_index - 1) * LINE_CAPACITY),
						context.line,
						LINE_CAPACITY
					);
				}
			} else {
				memmove(
					context.history + (context.history_index * LINE_CAPACITY),
					context.history + ((context.history_length - 1) * LINE_CAPACITY),
					LINE_CAPACITY
				);
				memcpy(
					context.history + ((context.history_length - 1) * LINE_CAPACITY),
					context.line,
					LINE_CAPACITY
				);
				context.history_index = context.history_length;
			}			
			write(STDOUT_FILENO, "\r\n", 2);
			context.should_exit = 1;
			break;
		case PREVIOUS_LINE:
			if (context.history_index) context.history_index--;
			copy_history_line_to_current_line();
			break;
		case NEXT_LINE:
			if (context.history_index < context.history_length) context.history_index++;
			copy_history_line_to_current_line();
			break;
		case EXIT:
			context.should_exit = 1;
			exit_raw_mode();
			exit(0);
			break;
	}
	return 0;
}

/*** output ***/
static void draw_line(void) {
	int i = 0;
	int cursor_x = context.terminal_x_begin_line;
	int cursor_y = context.terminal_y_begin_line;
	context.terminal_x = cursor_x;
	context.terminal_y = cursor_y;
	printf("\033[%d;%dH", cursor_y, cursor_x);
	fflush(stdout);
	printf("\033[0J"); /* clear screen */
	fflush(stdout);
	while (i < context.line_length) {
		int prompt_lenght = context.prompt == (void *)0 ? 0 : strlen(context.prompt);
		int how_many_chars_fit_on_this_line = context.terminal_columns - prompt_lenght;
		
		while (1) {
			if (how_many_chars_fit_on_this_line < 1 || i >= context.line_length) {
				break;
			}
			write(STDOUT_FILENO, &context.line[i++], 1);
			context.terminal_x++;
			if (i == context.line_index) {
				cursor_x = context.terminal_x;
				cursor_y = context.terminal_y;
			}

			how_many_chars_fit_on_this_line--;
			
		}
		
		if (how_many_chars_fit_on_this_line == 0) {
			if (context.terminal_y % context.terminal_rows == 0) {
				context.terminal_y_begin_line--; /* we need to do this becouse scrolling. :) */
			}
			write(STDOUT_FILENO, "\r\n", 2);
			context.terminal_y++;
			context.terminal_x = 1;
		}
		
		if (i == context.line_index) {
			cursor_x = context.terminal_x;
			cursor_y = context.terminal_y;
		}
	}
	

	printf("\033[%d;%dH", cursor_y, cursor_x);
	fflush(stdout);

}

static int prepare_for_read_line(const char *prompt) {
	if (enter_raw_mode() == -1) {
		return -1;
	}
	
	if (get_terminal_coordinate_and_size(
		&context.terminal_x,
		&context.terminal_y,
		&context.terminal_columns,
		&context.terminal_rows
	) == -1) {
		return -1;
	}

	context.prompt = prompt;
	context.terminal_x_begin_line = context.terminal_x;
	context.terminal_y_begin_line = context.terminal_y;
	context.line[0] = '\0';
	context.line_index = 0;
	context.line_length = 0;
	context.should_exit = 0;

	if (context.history == (void *)0) {
		context.history = calloc(LINE_CAPACITY * HISTORY_CAPACITY, sizeof(char));
		if (context.history == (void *) 0) return -1; /* allocation failure */
		context.history_length = 0;
		context.history_index = 0;
	}

	
	
	
	return 0;
}

int readline(char **line, const char *prompt) {
	input in = {0};

	write(STDOUT_FILENO, prompt, strlen(prompt));
	
	if (prepare_for_read_line(prompt) == -1) {
		return -1;
	}

	
	while(1) {
		handle_low_level_input(&in);
		handle_high_level_input(in);
		if (context.should_exit) break;
		draw_line();
	}
		
	exit_raw_mode();
	
	*line = context.line;
	return 0;
}