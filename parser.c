#include "parser.h"
#include <stdlib.h>
#include <string.h>

/* functions declarations of private functions*/
static int append_child_to_node(node *father, node *child);
static int internal_parse(const char *text, abstract_syntax_tree *ast, node *node);


/* structs */
typedef struct {
	enum {
		SEARCHING_NODE_START,
		SEARCHING_NODE_END,
		FINISHED,
	} parsing_state;
	int index_start;
	int index_end;
	node_type node_type;
} parsing_context;



/* node */
static int append_child_to_node(node *father, node *child) {
	node *children = realloc(father->children, sizeof(node) * (father->number_of_children + 1)); /* if father->children is null it acts ass a malloc*/
	if (children == (void *)0)
	{
		return -1; /* allocation failed */
	}
	father->children = children;
	father->children[father->number_of_children++] = *child;
	return 0;
}


/* ast*/


static int internal_parse(const char *text, abstract_syntax_tree *ast, node *current_node) {
	parsing_context ctx = {0};
	ctx.index_start = current_node->index_start;
	int i;
	for(i = current_node->index_start; i <= current_node->index_end; i++) {
		switch (text[i]) {
			case '(':
				ctx.parsing_state = SEARCHING_NODE_END;
				ctx.node_type = OPERATOR;
				current_node->node_type = OPERATOR;
				current_node->node_info.operator_info = (operator_info){
					.operator_type = PARENTESIS,
					.precedence = PARENTESIS_PRECEDENCE,
				};
				current_node->index_start = i;
				break;
			case ')':
				ctx.parsing_state = FINISHED;
				ctx.index_end = i;
				current_node->index_end = i;

				node child;
				child.node_type = EMPTY;
				child.index_start = current_node->index_start + 1;
				child.index_end = current_node->index_end - 1;
				child.number_of_children = 0;
				child.children = (void *)0;
				child.father = current_node;
				child.text = text;
				if (append_child_to_node(current_node, &child) == -1) {
					return -1;
				}
				break;
			case '/':
				break;
			case '*':
				break;
			case '-':
				break;
			case '+':
				break;
			default:
				break;
		}
	}

	if (ctx.parsing_state != FINISHED) {
		return -1; /* invalid input */
	}
}


int parse(const char *text, abstract_syntax_tree *ast) {
	ast->root = calloc(1, sizeof(*ast->root));
	if (ast->root == (void *)0) {
		return -1; /* allocation failed */
	}
	ast->root->node_type = EMPTY;
	ast->root->index_start = 0;
	ast->root->index_end = strlen(text) - 1;
	ast->root->father = (void *)0;
	ast->root->text = text;

	return internal_parse(text, ast, ast->root);
}

void print_ast(abstract_syntax_tree *ast) {
	
}