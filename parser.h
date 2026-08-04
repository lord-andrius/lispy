#ifndef PARSER_H
#define PARSER_H

typedef enum  {
	EMPTY = 0,
	OPERATOR,
	NUMBER,
} node_type;

typedef enum {
	PARENTESIS,
	MINUS,
	PLUS,
	DIVISION,
	MULTIPLICATION,
	SUBTRACTION,
	ADDITION,
} operator_type;

typedef enum {
	PARENTESIS_PRECEDENCE = 0,
	MINUS_PRECEDENCE = 1,
	PLUS_PRECEDENCE = 1,
	DIVISION_PRECEDENCE = 2,
	MULTIPLICATION_PRECEDENCE = 2,
	SUBTRACTION_PRECEDENCE = 3,
	ADDITION_PRECEDENCE = 3,
} operator_precedence;

typedef struct {
	operator_type operator_type;
	int precedence;
} operator_info;

typedef double number_info;

typedef union {
	operator_info operator_info;
	number_info number_info;
} node_info;

typedef struct _node {
	node_type node_type;
	node_info node_info;
	int index_start;
	int index_end;
	int number_of_children;
	struct _node *children;
	struct _node *father;
	const char *text;
} node;

typedef struct {
	node *root;
} abstract_syntax_tree;

int parse(const char *text, abstract_syntax_tree *ast);
void print_ast(abstract_syntax_tree *ast);

#endif