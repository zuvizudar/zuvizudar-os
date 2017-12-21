#include <string.h>		
#include "apilib.h"

struct PADINFO {
	int x;
	int y;
	int c;
};

void cons_putchar(int win ,int chr, char move);
void cons_newline(int win);
char *skipspace(char *p);
int strtol(char *s, char **endp, int base);	/* 標準関数（stdlib.h） */

struct PADINFO padinfo;
short winbuf[336 * 261];

void ZuviMain(void){
	int win,w_x,w_y;
	int i,j = 0,c=0;
	int key_shift = 0;
	char line[40];
	char size[512];
	int x, y;
	int w=30,h=10,t=4;
	char s[40],*p,*q=0,*r=0;
	int sizef = 0;
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int fh;	

	w_x = 336;
	w_y = 261;
	x = 76;
	y = 56;
	padinfo.x = 8;
	padinfo.y = 30;
	padinfo.c = 0;
	win = api_openwin(winbuf, w_x, w_y, -1, "vim");
	api_boxfilwin(win, 6, 26, w_x-7, w_y-7, 15);
	api_boxfilwin(win, 7, 27, w_x-8, w_y-8, 7);
	api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,padinfo.c);
	/* コマンドライン解析 */
	api_cmdline(s, 30);
	for (p = s; *p > ' '; p++) { }	/* スペースが来るまで読み飛ばす */
	for (; *p != 0; ) {
		p = skipspace(p);
		if (*p == '-') {
			if (p[1] == 'w') {
				w = strtol(p + 2, &p, 0);
				if (w < 20) {
					w = 20;
				}
				if (w > 126) {
					w = 126;
				}
			} else if (p[1] == 'h') {
				h = strtol(p + 2, &p, 0);
				if (h < 1) {
					h = 1;
				}
				if (h > 45) {
					h = 45;
				}
			} else if (p[1] == 't') {
				t = strtol(p + 2, &p, 0);
				if (t < 1) {
					t = 4;
				}
			} else {
err:
				api_putstr0(" >tview file [-w30 -h10 -t4]\n");
				api_end();
			}
		} else {	/* ファイル名発見 */
			if (q != 0) {
				goto err;
			}
			q = p;
			for (; *p > ' '; p++) { }	/* スペースが来るまで読み飛ばす */
			r = p;
		}
	}
	if (q == 0) {
		goto err;
	}


	fh = api_fopen_w(q);
	

	for (;;) {
		i = api_getkey(1);
		if (i == 0x2a) {
			key_shift |= 1;
			
		}
		if (i < 0x80) { /* キーコードを文字コードに変換 */		
			s[0] = keytable0[i];
			if (key_shift == 0) {
				s[0] = keytable0[i];
			} else {
				s[0] = keytable1[i];
			}
		} else {
			s[0] = 0;
		}
		if ('A' <= s[0] && s[0] <= 'Z') {
			if (key_shift == 0) {
				s[0] += 0x20;
			}
		}		
		if (i == 8) {
			if (padinfo.x > 8) {/*	Delete*/						
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,7);	
				cons_putchar(win, ' ', 0);				
				size[j] = 0;
				i = ' ';
				//size[j++] = 0;
				j--;
				//line[padinfo.x/8 - 1] = ' ';
				padinfo.x -= 8;
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,0);
				if(sizef >0){
					sizef--;
				}							
			}
		} else if (i == 9) {/* Enter */		
			if (padinfo.x < 320+8) {
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,7);				
				cons_putchar(win, ' ', 0);
				//line[padinfo.x/8 - 1] = ' ';
				//line[padinfo.x/8 - 1] = '\n';
				cons_newline(win);				
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,0);			
			}			
		} else {
			if (padinfo.x < 320) {		
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,7);
				line[padinfo.x/8 - 1] = i;
				cons_putchar(win ,i, 1);
				sizef++;
				/*sprintf(s, "%c",i);						
				api_putstrwin(win,padinfo.x, padinfo.y, 0, 1,s);
				
				padinfo.x += 8;	*/
				
				api_boxfilwin(win, padinfo.x,padinfo.y,padinfo.x+7,padinfo.y+15,0);	
			}
		}
		api_refreshwin(win, 8,30,336-7,261-7);	
		
		size[j] = i;
		j++;
		if(j == 1){
			size[j] = 0;
		}
		c++;
		if(j == c){
			size[j+1] = 0;
		}
		if (i == '$') {/* space */	
			sizef--;
			if (fh != 0) {
				api_fwrite((char *)size, sizef, fh);
				api_fclose_w(fh);
				break;
			}						
		}
		
	}
	api_end();

}

char *skipspace(char *p)
{
	for (; *p == ' '; p++) { }	/* スペースを読み飛ばす */
	return p;
}

void cons_putchar(int win ,int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {
		for (;;) {		
			//putfouts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			api_putstrwin(win, padinfo.x, padinfo.y, 0, 1," ");
			padinfo.x += 8;
			if (padinfo.x == 8 + 320) {
				cons_newline(win);
			}
			if (((padinfo.x - 8) & 0x1f) == 0) {
				break;	/* 32で割り切れたらbreak */
			}
		}
	} else if (s[0] == 0x0a) {
		cons_newline(win);
	} else if (s[0] == 0x0d) {

	} else {
		if (win != 0) {
			api_putstrwin(win, padinfo.x, padinfo.y, 0, 1,s);
		}
		if (move != 0) {
			padinfo.x += 8;
			if (padinfo.x == 8 + 320) {
				cons_newline(win);
			}
		}
	}
	return;
}

void cons_newline(int win){
{
	int x, y;
	
	if ( padinfo.y < 30 + 16*5) {
		padinfo.y += 16;
	} else {
		for (y = 30; y < 30 + 16*5; y++) {
			for (x = 8; x < 8 + 320; x++) {
				winbuf[x + y * 336] = winbuf[x + (y + 16) * 336];
			}
		}
		/*for (y = 30 + 16*5; y < 30 + 16*6; y++) {
			for (x = 8; x < 8 + 320; x++) {
				winbuf[x + y * 336] = 7;
			}
		}*/
		api_boxfilwin(win, 8,30 + 16*5,8 + 320,30 + 16*6,7);
		api_refreshwin(win, 8, 30, 8 + 320, 30 + 16*7);
		}
	}
	padinfo.x = 8;
	return;
}
