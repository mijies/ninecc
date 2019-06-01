#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	TK_NUM = 256,	// Integer token
	TK_EOF,
	TK_EQ,
	TK_NE,
	TK_LE,
	TK_GE,
};

typedef struct {
	int ty;			// Token type
	int val;		// Numeric value when ty is TK_NUM
	char *input;	// Token string
} Token;

Token tokens[100];
int pos = 0;


enum {
	ND_NUM = 256,	// Represents type for Interfer Node
};

typedef struct Node_ Node;
typedef struct Node_ {
	int ty;
	Node *lhs;	// Left hand side of Node tree
	Node *rhs;	// Left hand side of Node tree
	int val;			// when ty is ND_NUM
} Node;

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
void error(char *fmt, ...);

Node *new_node(int ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

int consume(int ty) {
	if (tokens[pos].ty != ty)
		return 0;
	pos++;
	return 1;
}

Node *expr() {
	Node *node = equality();


}

Node *equality() {
	Node *node = relational();
}

Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume('<'))
			node = new_node('<', node, add());
		else if (consume(TK_LE))
			node = new_node(TK_LE, node, add());
		else if (consume('>'))
			node = new_node('>', node, add());
		else if (consume(TK_GE))
			node = new_node(TK_GE, node, add());
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node('+', node, mul());
		else if (consume('-'))
			node = new_node('-', node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume('*'))
			node = new_node('*', node, unary());
		else if (consume('/'))
			node = new_node('/', node, unary());
		else
			return node;
	}
}

Node *unary() {
	if (consume('+'))
		return term();

	if (consume('-'))
		return new_node('-', new_node_num(0), term());

	return term();
}

Node *term() {
	if (consume('(')) {
		Node *node = add();
		if (!consume(')'))
			error("No close parenthese found: s", tokens[pos].input);
		return node;
	}

	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokens[pos++].val);

	error("Neither Number nor open parenthese found", tokens[pos].input);
}

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void tokenize(char *p) {
	int i = 0;
	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strncmp(p, "<=", 2)==0) {
			tokens[i].ty = TK_LE;
			tokens[i].input = "<=";
			i++;
			p++;
			p++;
			continue;
		}

		if (strncmp(p, ">=", 2)==0) {
			tokens[i].ty = TK_GE;
			tokens[i].input = ">=";
			i++;
			p++;
			p++;
			continue;
		}

		if (*p == '<' || *p == '>' ||
		    *p == '+' || *p == '-' ||
			*p == '*' || *p == '/' ||
			*p == '(' || *p == ')') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		error("can not tokenize: %s", p);

	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

void gen(Node *node) {
	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n"); // Value is set by rhs
	printf("  pop rax\n"); // Value is set by lhs

	switch (node->ty) {
	case '<':
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case TK_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	case '>':
		printf("  cmp rdi, rax\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case TK_GE:
		printf("  cmp rdi, rax\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	case '+':
		printf("  add rax, rdi\n");
		break;
	case '-':
		printf("  sub rax, rdi\n");
		break;
	case '*':
		printf("  mul rdi\n");
		break;
	case '/':
		printf("  mov rdx, 0\n");
		printf("  div rdi\n");
	}

	printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2 ) {
		fprintf(stderr, "The number of the arguments is not right\n");
		return 1;
	}

	tokenize(argv[1]);
	Node *node = relational();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("  pop rax\n"); // Pop the total cumputed value from the stack top
	printf("  ret\n");
	return 0;
}
