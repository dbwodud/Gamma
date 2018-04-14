#include "Gamma.h"
int cnt = 1;
void parser_init() {
	lookahead = head;
}
void LLparser() {
	parser_init();	
	while (lookahead->token_type != T_eof) {
		S();
	}
}

void match(int terminal) {
	if (lookahead->token_type == terminal) {
		lookahead = lookahead->next;
		cnt++;
	} 
	else {
		printf("count : %d\n", cnt);
		printf("Terminal : %d\n",terminal);
		printf("Token_type : %d\n",lookahead->token_type);
		printf("match Error");
		exit(1);
	}
}

int DT(){
	if(lookahead->token_type==T_char){
		match(T_char); return 1;
	}
	else if(lookahead->token_type==T_int){
		match(T_int); return 1;
	}
	else{
		printf("Data Type Error");
		return 0;
	}
}

void arguments() {
	while (1) {
		switch (lookahead->token_type) {
		case T_variable:
			match(T_variable);
			if (lookahead->token_type == T_peroid) {
				match(T_peroid);
				continue;
			}
			else if(lookahead->token_type==T_rparen){
				return;
			}
		case T_rparen:
			return;
		default:
			printf("arguments Error");
			exit(1);
			break;
		}
	}
}

void parameters() {
	while (1) {
		switch (lookahead->token_type) {
		case T_int:
		case T_char:
			DT(); match(T_variable);
			if (lookahead->token_type == T_peroid) {
				match(T_peroid);
				continue;
			}
			else {
				return;
			}
			break;
		case T_rparen:
			return;
		default:
			printf("parameters Error");
			exit(1);
			break;
		}
	}
}

void S() {
	switch (lookahead->token_type) {
	case T_int:
	case T_char:
		DT(); match(T_variable); 
		if (lookahead->token_type == T_semicolon) {
			match(T_semicolon);
		}
		else if (lookahead->token_type == T_assign) {
			while (lookahead->token_type == T_assign) {
				match(T_assign); E();
			}
			match(T_semicolon);
		}
		break;
	case T_def:
		match(T_def); match(T_variable); match(T_lparen); parameters(); match(T_rparen); 
		match(T_lbrace);
		while (lookahead->token_type != T_rbrace) {
			S();
		}
		match(T_rbrace);
		break;
	case T_variable:
		match(T_variable);
		while (lookahead->token_type == T_assign) {
			match(T_assign); E();
		}
		if (lookahead->token_type==T_lparen) {
			match(T_lparen); arguments(); match(T_rparen);
		}
		match(T_semicolon);
		break;
	case T_if:
		match(T_if); match(T_lparen); E(); match(T_rparen);
		match(T_lbrace);
		while(lookahead->token_type != T_rbrace) {
			S();
		}
		match(T_rbrace);
		break;
	case T_while:
		match(T_while); match(T_lparen); E(); match(T_rparen);
		match(T_lbrace);
		while (lookahead->token_type != T_rbrace) {
			S();
		}
		match(T_rbrace);
		break;
	default:
		printf("Count : %d\n", cnt);
		printf("Token_type : %d\n",lookahead->token_type);
		printf("Error");
		exit(1);
		break;
	}
}

void E() {
	T();
	switch (lookahead->token_type)
	{
	case T_add:
		match(T_add); E();
		break;
	case T_sub:
		match(T_sub); E();
		break;
	case T_equal:
		match(T_equal); E();
		break;
	case T_notequal:
		match(T_notequal); E();
		break;
	}
	
}

void T() {
	F();
	switch (lookahead->token_type) 
	{
	case T_mul:
		match(T_mul); T();
		break;
	case T_div:
		match(T_div); T();
		break;
	}
}

void F() {
	switch (lookahead->token_type) 
	{
	case T_variable:
		match(T_variable);
		break;
	case T_const:
		match(T_const);
		break;
	case T_lparen:
		match(T_lparen); E(); match(T_rparen);
	default:
		printf("Form Error");
		exit(1);
	}
}