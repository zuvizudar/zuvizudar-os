#include <stdio.h>
#include <string.h>
#include "apilib.h"

int var[256];
char *p;

int get_num();
int printf(char *format, ...);
void syntaxError(int pos);
int calc();
void skip_space();
int get_var();

int ZuviMain(void){
	int i;
	char buf[10000];
	int fh;
	char c,cmdline[30];
	int cat_flag=0;

	api_cmdline(cmdline,30);
	for(p = cmdline;*p>' ';p++){ }
	skip_space();
	if(*p=='-'){
		p++;
		if(*p=='s'){
			cat_flag=1;
		}
		p++;
		skip_space();
	}
	fh = api_fopen(p);
	int cnt=0;
	if(fh != 0){
		for(;;){
			if( api_fread(&c,1,fh) == 0){
				break;
			}
			buf[cnt]=c;
			if(cat_flag)api_putchar(buf[cnt]);
			cnt++;
		}
	}
	buf[cnt] = -1; //end

	for (i = 0; i < 256; i++)
		var[i] = 0;

	p = &buf[0];
	for (;;) {
		skip_space();
		if (p[0] == -1) break; //end
		if (p[0] == '\n') {
			p++;
			continue;
		}
		if (strncmp(p,"print",5)==0&&(p[5]==' '||p[5]=='\t')) {
			p+=6;
			printf("%d", calc());
			skip_space();
			if(p[0]!='\n')syntaxError(4);
			continue;
		}
		int l=get_var();
		skip_space();
		if (p[0] == '=') {
			p++;
			var[l]=calc();
			skip_space();
			if(p[0]!='\n'){
				syntaxError(5);
			}
			continue;
		}
		syntaxError(6);
	}
	api_end();
	return 0;
}
int printf(char *format, ...){
	va_list ap;
	char s[1000];
	int i;

	va_start(ap,format);
	i = vsprintf(s,format,ap);
	api_putstr0(s);
	va_end(ap);
	return i;
}

int check_char(unsigned char c,int not_front_flag){
	if ('A' <= c && c <= 'Z') return 1;
	if ('a' <= c && c <= 'z') return 1;
	if (c == '_') return 1;
	if(not_front_flag){
		if ('0' <= c && c <= '9') return 1;
	}
	return 0;
}

int get_num(){
	int l;
	skip_space();
	if(p[0]=='('){
		p++;
		l=calc();
		skip_space();
		if(p[0]!=')'){
			syntaxError(1);
		}
		p++;
		return l;
	}
	if (check_char(p[0],0) != 0) {
		l = var[get_var()];
	} 
	else if ('0' <= p[0] && p[0] <= '9') {
		l = 0;
        while ('0' <= p[0] && p[0] <= '9') {
			l = l * 10 + (p[0] - '0');
			p++;
		}
	}
   	else if (p[0] == '+') {
		p++;
		l = get_num();
	}
   	else if (p[0] == '-') {
		p++;
		l = - get_num();
	}
   	else{
		syntaxError(2);
	}
	return l;
}

int get_var(){
	int i;
	if (check_char(p[0],0) == 0)
		syntaxError(3);
	i = p[0];
	while (check_char(p[0],1) != 0)
		p++;
	return i;
}

void syntaxError(int pos){
	printf("syntax error %d: %.30s\n",pos,p);
	api_end();
}
void skip_space(){
	while(p[0]==' '||p[0]=='\t')p++;
}
int calc(){
	int l;
	l=get_num();
	for(;;){
		skip_space();
		switch(p[0]){
			case '+':
				p++;
				l += get_num();
				continue;
			case '-':
				p++;
				l -= get_num();
				continue;
			case '*':
				p++;
				l *= get_num();
				continue;
			case '/':
				p++;
				l /= get_num();
				continue;
			goto calc_end;
		}
		break;
	}
	calc_end:;
	return l;
}


