#include "Gamma.h"

char buffer[32];
struct token *head;
struct token *token_p;
struct token *tail;
struct token *lookahead;
// Token ================================================
void initToken() {
	token_p = (struct token*)malloc(sizeof(struct token));
	head = token_p;
}
void insertToken(int value) {
	token_p->token_type = value;
	token_p->next = (struct token*)malloc(sizeof(struct token));
	token_p = token_p->next;
	token_p->next = NULL;
	tail = token_p;
}
void removeLastToken() {
	free(tail);
	token_p = head;
	while (token_p->next != NULL)
		token_p = token_p->next;
	tail = token_p;
}
void printTokens() {
	int cnt = 1;
	token_p = head;
	while (token_p->next != NULL) {
		printf("%d : %d\n", cnt , token_p->token_type);
		token_p = token_p->next;
		cnt++;
	}
}
// Keyword ================================================
// T_NULL=-1, T_int, T_char, T_return, T_if, T_else , T_while,
char *keyword[] = {
	"NULL","void","int","char","return","if","else","while","def"
};

int buffer_cmp(char *str) {
	unsigned int i;
	for (i = 0; i < sizeof(char)*(strlen(str)); i++) {
		if (buffer[i] == str[i]) {
			continue;
		}
		else {
			return 0;
		}
	}
	return 1;
}

int iskeyword() {		// keyword search
	int i;
	for (i = 0; i < 9; i++) {
		if (buffer_cmp(keyword[i])) {
			insertToken(i-1);
			return 1;
		}
		else {
			continue;
		}
	}
	return 0;
}
void init_buffer(){
	int i;
	for(i=0;i<32;i++){
		buffer[i]='\0';
	}
}
// ========================================================
void isbraket(char ch) {
	switch (ch)
	{
	case '{':
		insertToken(T_lbrace);
		break;
	case '}':
		insertToken(T_rbrace);
		break;
	case '(':
		insertToken(T_lparen);
		break;
	case ')':
		insertToken(T_rparen);
		break;
	case '[':
		insertToken(T_lbracket);
		break;
	case ']':
		insertToken(T_rbracket);
		break;
	default:
		printf("Error");
		exit(1);
		break;
	}
}

void isop(char aheadch,FILE *fp) {
	char forword_char = fgetc(fp);
	switch (aheadch) {
	case '!':
		if (forword_char == '=') {
			insertToken(T_notequal);
		}
		else {
			insertToken(T_div);
			fseek(fp,-1L,SEEK_CUR);
		}
		break;
	case '+':
		insertToken(T_div);
		fseek(fp,-1L,SEEK_CUR);
		break;
	case '-':
		insertToken(T_div);
		fseek(fp,-1L,SEEK_CUR);
		break;
	case '%':
		insertToken(T_mod);
		fseek(fp,-1L,SEEK_CUR);
		break;
	case '/':
		insertToken(T_div);
		fseek(fp,-1L,SEEK_CUR);
		break;
	case '*':
		insertToken(T_mul);
		fseek(fp, -1L, SEEK_CUR);
		break;
	case '=':
		if (forword_char == '=') {
			insertToken(T_equal);
		}
		else {
			insertToken(T_assign);
			fseek(fp,-1L,SEEK_CUR);
		}
		break;
	}
}

void lexer(FILE *fp) {

	initToken();
	
	char aheadch;
	char ch;
	int bufptr = 0;

	if (fp == NULL) {
		printf("error while opening the file\n");
		exit(0);
	}

	init_buffer();

	while ((aheadch = fgetc(fp)) != EOF) {
		if (aheadch=='\n' || aheadch==' ' || aheadch=='\t') {
			continue;
		}
		else if(aheadch == '{' || aheadch == '}' || aheadch == '[' || aheadch == ']' || aheadch == '(' || aheadch == ')'){
			isbraket(aheadch);
		}
		else if(aheadch == '!' || aheadch == '+' || aheadch == '-' || aheadch == '%' || aheadch == '/' || aheadch == '=' || aheadch == '*') {
			isop(aheadch,fp);
		}
		else if (aheadch == ';') {
			insertToken(T_semicolon);
		}
		else if (aheadch == ',') {
			insertToken(T_peroid);
		}
		else if (isalpha(aheadch)||aheadch=='_'){
			buffer[bufptr] = aheadch;
			bufptr++;
			while((ch=fgetc(fp))){
				if(isalpha(ch)){
					buffer[bufptr] = ch;
					bufptr++;
				}
				else if(isdigit(ch)){
					buffer[bufptr] = ch;
					bufptr++;
				}
				else if(ch=='_'){
					buffer[bufptr] = ch;
					bufptr++;
				}
				else{
					break;
				}
			}
			fseek(fp,-1L,SEEK_CUR);
			if(iskeyword()){
			}
			else{
				insertToken(T_variable);
			}
			bufptr=0;
			init_buffer();
		}
		else if(isdigit(aheadch)){
			buffer[bufptr] = aheadch;
			bufptr++;
			while((ch=fgetc(fp))){
				if(isdigit(ch)){
					buffer[bufptr] = ch;
					bufptr++;
				}
				else if(isalpha(ch)){
					printf("isdigit : Error");
					exit(1);
				}
				else if(ch=='_'){
					printf("isdigit : Error");
					exit(1);
				}
				else{
					break;
				}
			}
			fseek(fp,-1L,SEEK_CUR);
			init_buffer();
			insertToken(T_const);
			bufptr=0;
		}
	}
	insertToken(T_eof);
}
