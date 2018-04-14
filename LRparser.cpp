#include "Gamma.h"

class LRparser {
	std::stack<int> parser_stack;
public:
	int lookahead;
	LRparser() {
		initToken();
		lookahead = head->token_type;
	}
	void execute() {
		while (lookahead != T_eof) {
			switch (lookahead) {
			case T_int:
			case T_char:
			case T_variable:
			case T_assign:
			case T_if:
			case T_while:
			case T_lparen:
				shift();
				break;
			case T_rparen:
				shift();
				reduce();
				accept();
				break;
			case T_add:
			case T_sub:
				reduce();
				reduce();
				reduce();
				shift();
				shift();
				break;
			case T_mod:
			case T_div:
			case T_mul:
				reduce();
				reduce();
				shift();
				shift();
				break;
			case T_semicolon:
				while (parser_stack.top() != T_stmt)
					reduce();
				accept();
				break;
			}
		}
	}
	void match(int n) {
		if (n == parser_stack.top()) {
			parser_stack.pop();
		}
		else {
			printf("LRparser match Error");
			exit(1);
		}
	}
	void shift() {
		if (lookahead == T_eof)
			return;
		if (token_p->next) {
			parser_stack.push(token_p->token_type);
			token_p = token_p->next;
			lookahead = token_p->next->token_type;
		}
		else {
			printf("parser Error");
			exit(1);
		}
	}
	void reduce() {
		switch (parser_stack.top()) {
		case T_rparen:
			match(T_rparen); execute(); match(T_expr); match(T_lparen);
			if (parser_stack.top()==T_while || parser_stack.top()==T_if) {
				while (!parser_stack.empty)
					parser_stack.pop();
				parser_stack.push(T_stmt);
			}
			else {
				parser_stack.push(T_form);
			}
			break;
		case T_variable:
			match(T_variable); parser_stack.push(T_form);
			break;
		case T_const:
			match(T_const); parser_stack.push(T_form);
		case T_form:
			match(T_form);
			if (parser_stack.top() == T_mul || parser_stack.top() == T_div) {
				parser_stack.pop();
				match(T_term);
			}
			parser_stack.push(T_term);
			break;
		case T_term:
			match(T_term);
			if (parser_stack.top() == T_add || parser_stack.top() == T_sub) {
				parser_stack.pop();
				match(T_expr);
			}
			parser_stack.push(T_expr);
			break;
		case T_expr:
			match(T_expr); match(T_assign); match(T_variable);
			if (parser_stack.top()==T_int) {
				match(T_int);
			}
			else if (parser_stack.top() == T_char) {
				match(T_char);
			}
			parser_stack.push(T_stmt);
			break;
		}
	}
	void accept() {
		if (parser_stack.top()==T_stmt) {
			parser_stack.pop();
		}
		else {
			printf("Accept Error");
			exit(1);
		}
	}
};
