#ifndef PARSER_H
#define PARSER_H

typedef enum  {
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

typedef struct {
	operator_type operator_type;
	int precedence;
} operator_info;

typedef double number_info;

typedef union {
	operator_info operator_info;
	number_info number_info;
} node_info;

typedef struct node {
	node_type node_type;
	node_info node_info;
	int index_start;
	int index_end;
	int number_of_children;
	struct node *children;
	struct node *father;
} node;

typedef struct {
	node *nodes;
} abstract_syntax_tree;

int parse(const char *text, abstract_syntax_tree *ast);

#endif