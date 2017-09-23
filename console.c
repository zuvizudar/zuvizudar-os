#include "bootpack.h"
#include<stdio.h>
#include<string.h>

void console_task(SHEET *sheet,int memtotal){
	TIMER *timer;
	TASK *task = task_now();
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int i, fifobuf[128],*fat = (int *) memman_alloc_4k(memman,4*2880);
	CONSOLE cons;
	char cmdline[30];
	cons.sht=sheet;
	cons.cursor_x=8;
	cons.cursor_y=28;
	cons.cursor_c=-1;
	*((int *)0x0fec)= (int)&cons;

	fifo32_init(&task->fifo,128,fifobuf,task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat,(unsigned char *)(ADR_DISKIMG+0x000200));

	cons_putchar(&cons,'>',1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) {
				if (i != 0) {
					timer_init(timer, &task->fifo, 0);
					if(cons.cursor_c>=0){
						cons.cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1);
					if(cons.cursor_c>=0){
						cons.cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if(i==2){
				cons.cursor_c=COL8_FFFFFF;
			}
			if(i==3){
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cons.cursor_x,28,cons.cursor_x+7,43);
				cons.cursor_c=-1;
			}
			if(256<=i&&i<=511){
				if (i == 8 + 256) {
					if (cons.cursor_x > 16) {
						cons_putchar(&cons,' ',0);
						cons.cursor_x -= 8;
					}
				}
				else if(i == 10+256){
					cons_putchar(&cons,' ',0);
					cmdline[cons.cursor_x/8-2]=0;
					cons_newline(&cons);
					cons_runcmd(cmdline,&cons,fat,memtotal);
					cons_putchar(&cons,'>',1);
				}
				else {
					if (cons.cursor_x < 240) {
						cmdline[cons.cursor_x/8-2]=i-256;
						cons_putchar(&cons,i-256,1);
					}
				}
			}
			if(cons.cursor_c>=0){
				boxfill8(sheet->buf,sheet->bxsize,cons.cursor_c,cons.cursor_x,cons.cursor_y,cons.cursor_x+7,cons.cursor_y+15);
			}
			sheet_refresh(sheet,cons.cursor_x,cons.cursor_y,cons.cursor_x+8,cons.cursor_y+16);
		}
	}
}

void cons_putchar(CONSOLE *cons, int chr, char move){
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cursor_x += 8;
			if (cons->cursor_x == 8 + 240) {
				cons_newline(cons);
			}
			if (((cons->cursor_x - 8) & 0x1f) == 0) {
				break;
			}
		}
	}
 	else if (s[0] == 0x0a) {
		cons_newline(cons);//改行
	}
 	else if (s[0] == 0x0d) {
	} 
	else{
		putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			cons->cursor_x += 8;
			if (cons->cursor_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}
void cons_newline(CONSOLE *cons){
	int x, y;
	SHEET *sheet =cons->sht;
	if (cons->cursor_y < 28 + 112) {
		cons->cursor_y += 16;
	} 
	else {
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	cons->cursor_x=8;
	return;
}
void cons_putstr0(CONSOLE *cons,char *s){
	for(;*s!=0;s++){
		cons_putchar(cons,*s,1);
	}
	return ;
}

void cons_putstr1(CONSOLE *cons,char *s,int l){
	int i;
	for(i=0;i<l;i++){
		cons_putchar(cons,s[i],1);
	}
	return;
}

void cons_runcmd(char *cmdline, CONSOLE *cons, int *fat, int memtotal){
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} 
	else if (strcmp(cmdline, "clear") == 0) {
		cmd_clear(cons);
	}
 	else if (strcmp(cmdline, "ls") == 0) {
		cmd_ls(cons);
	}
 	else if (strncmp(cmdline, "cat ", 4) == 0) {
		cmd_cat(cons, fat, cmdline);
	}
 	else if (cmdline[0] != 0) {
		if(cmd_app(cons,fat,cmdline)==0){
			cons_putstr0(cons,"Bad command\n\n");
		}
	}
	return;
}

void cmd_mem(CONSOLE *cons, int memtotal)
{
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024),memman_total(memman));
	cons_putstr0(cons,s);
	return;
}

void cmd_clear(CONSOLE *cons)
{
	int x, y;
	SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cursor_y = 28;
	return;
}

void cmd_ls(CONSOLE *cons){
	FILEINFO *finfo = (FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons,s);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_cat(CONSOLE *cons, int *fat, char *cmdline){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo = file_search(cmdline + 4, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p,finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} 
	else {
		cons_putstr0(cons,"File not found.\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(CONSOLE *cons, int *fat,char *cmdline){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;
	char *p,name[18];
	int i;
	for(i=0;i<13;i++){
		if(cmdline[i]<=' '){
			break;
		}
		name[i]=cmdline[i];
	}
	name[i]=0;
	finfo=file_search(name,(FILEINFO*)(ADR_DISKIMG+0x002600),224);
	if(finfo==0 && name[i-1]!= '.'){
		name[i]='.';
		name[i+1]='Z';
		name[i+2]='U';
		name[i+3]='V';
		name[i+4]=0;
		finfo=file_search(name,(FILEINFO*)(ADR_DISKIMG+0x002600),224);
	}
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
		farcall(0, 1003 * 8);
		memman_free_4k(memman, (int) p, finfo->size);
		cons_newline(cons);
		return 1;
	} 
	return 0;
}
void zuv_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
	CONSOLE *cons = (CONSOLE *) *((int *) 0x0fec);
	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if (edx == 2) {
		cons_putstr0(cons, (char *) ebx);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx, ecx);
	}
	return;
}
