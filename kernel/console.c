#include "bootpack.h"
#include<stdio.h>
#include<string.h>
unsigned char t[7];
CONSOLE *tcons;

void console_task(SHEET *sheet,int memtotal){
	TASK *task = task_now();
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int i,*fat = (int *) memman_alloc_4k(memman,4*2880);
	CONSOLE cons;
	FILEHANDLE fhandle[8];
	char cmdline[30];
	unsigned char *nihongo =(char*)*((int *)0x0fe8);

	cons.sht=sheet;
	cons.cursor_x=8;
	cons.cursor_y=28;
	cons.cursor_c=-1;
	//*((int *)0x0fec)= (int)&cons;
	task->cons = &cons;
	task->cmdline = cmdline;
	if(cons.sht != 0){
		cons.timer =timer_alloc();
		timer_init(cons.timer, &task->fifo, 1);
		timer_settime(cons.timer, 50);
	}
	file_readfat(fat,(unsigned char *)(ADR_DISKIMG+0x000200));
	for(i=0;i<8;i++){
		fhandle[i].buf=0;
	}
	task->fhandle=fhandle;
	task->fat=fat;
	if(nihongo[4096]!=0xff){
		task->langmode=1;
	}
	else{
		task->langmode=0;
	}
	task->langbyte1=0;
	cons_putchar(&cons,'>',1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0) {
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0);
					if(cons.cursor_c>=0){
						cons.cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1);
					if(cons.cursor_c>=0){
						cons.cursor_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if(i==2){
				cons.cursor_c=COL8_FFFFFF;
			}
			if(i==3){
				boxfill8(cons.sht->buf,cons.sht->bxsize,COL8_000000,cons.cursor_x,28,cons.cursor_x+7,43);
				cons.cursor_c=-1;
			}
			if(i==4){
				cmd_exit(&cons,fat);
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
					if(cons.sht ==0){
						cmd_exit(&cons,fat);
					}
					cons_putchar(&cons,'>',1);
				}
				else {
					if (cons.cursor_x < 240) {
						cmdline[cons.cursor_x/8-2]=i-256;
						cons_putchar(&cons,i-256,1);
					}
				}
			}
			if(cons.sht != 0){
				if(cons.cursor_c>=0){
					boxfill8(cons.sht->buf,cons.sht->bxsize,cons.cursor_c,cons.cursor_x,cons.cursor_y,cons.cursor_x+7,cons.cursor_y+15);
				}
				sheet_refresh(cons.sht,cons.cursor_x,cons.cursor_y,cons.cursor_x+8,cons.cursor_y+16);
			}
		}
	}
}

void cons_putchar(CONSOLE *cons, int chr, char move){
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {
		for (;;) {
			if(cons->sht != 0){
			putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);	
			}
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
		if(cons->sht != 0){
			putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
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
	TASK *task =task_now();
	if (cons->cursor_y < 28 + 112) {
		cons->cursor_y += 16;
	} 
	else {
		if(sheet != 0){
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
	}
	cons->cursor_x=8;
	if(task->langmode == 1 && task->langbyte1 != 0){
		cons->cursor_x+=8;
	}
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
	if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
		cmd_mem(cons, memtotal);
	} 
	else if (strcmp(cmdline, "clear") == 0 && cons->sht != 0) {
		cmd_clear(cons);
	}
 	else if (strcmp(cmdline, "ls") == 0 && cons->sht != 0) {
		cmd_ls(cons);
	}
 	/*else if (strncmp(cmdline, "cat ", 4) == 0 && cons->sht != 0) {
		cmd_cat(cons, fat, cmdline);
	}*/
	else if (strcmp(cmdline,"exit")==0){
		cmd_exit(cons,fat);
	}
	else if (strncmp(cmdline,"start ",6)==0){
		cmd_start(cons,cmdline,memtotal);
	}
	else if (strncmp(cmdline,"ncst ",5)==0){
		cmd_ncst(cons,cmdline,memtotal);
	}
	else if(strncmp(cmdline,"langmode ",9)==0){
		cmd_langmode(cons,cmdline);
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
/*
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
*/
void cmd_exit(CONSOLE *cons, int *fat){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	TASK *task = task_now();
	SHTCTL *shtctl = (SHTCTL *) *((int *) 0x0fe4);
	FIFO32 *fifo = (FIFO32 *) *((int *) 0x0fec);
	timer_cancel(cons->timer);
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if(cons->sht != 0){
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);
	}
	else{
		fifo32_put(fifo, task-taskctl->tasks0 +1024);
	}
	io_sti();
	for (;;) {
		task_sleep(task);
	}
}

void cmd_start(CONSOLE *cons, char *cmdline, int memtotal){
  SHTCTL *shtctl = (SHTCTL *) *((int *) 0x0fe4);
	SHEET *sht = open_console(shtctl, memtotal);
	FIFO32 *fifo = &sht->task->fifo;
	int i;
	sheet_slide(sht, 32, 4);
	sheet_updown(sht, shtctl->top);
	for (i = 6; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

void cmd_ncst(CONSOLE *cons, char *cmdline, int memtotal){
	TASK *task = open_constask(0, memtotal);
	FIFO32 *fifo = &task->fifo;
	int i;
	for (i = 5; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

void cmd_langmode(CONSOLE *cons, char *cmdline){
	TASK *task = task_now();
	unsigned char mode = cmdline[9] - '0';
	if (mode <= 2) {
		task->langmode = mode;
	} else {
		cons_putstr0(cons, "mode number error.\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(CONSOLE *cons, int *fat,char *cmdline){
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *finfo;
	char *p,name[18],*q;
	TASK *task=task_now();
	int i,segsiz,datsiz,esp,datzuv;
	SHTCTL *shtctl;
	SHEET *sht;
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
		if (finfo->size >= 36 && strncmp(p + 4, "Zuvi", 4) == 0 && *p == 0x00) {
			segsiz = *((int *) (p + 0x0000));
			esp    = *((int *) (p + 0x000c));
			datsiz = *((int *) (p + 0x0010));
			datzuv = *((int *) (p + 0x0014));
			q = (char *) memman_alloc_4k(memman, segsiz);
			//*((int *) 0xfe8) = (int) q;
			task->ds_base =(int)q;
			set_segmdesc(task->ldt + 0, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
			set_segmdesc(task->ldt + 1, segsiz - 1,      (int) q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datsiz; i++) {
				q[esp + i] = p[datzuv + i];
			}
			start_app(0x1b, 0*8+4, esp,1*8+4, &(task->tss.esp0));
			shtctl = (SHTCTL *) *((int *) 0x0fe4);
			for (i = 0; i < MAX_SHEETS; i++) {
				sht = &(shtctl->sheets0[i]);
				if ((sht->flags & 0x11) == 0x11 && sht->task == task) {
					sheet_free(sht);
				}
			}
			for (i = 0; i < 8; i++) {
				if (task->fhandle[i].buf != 0) {
					memman_free_4k(memman, (int) task->fhandle[i].buf, task->fhandle[i].size);
					task->fhandle[i].buf = 0;
				}
			}
			timer_cancelall(&task->fifo);
			memman_free_4k(memman, (int) q, segsiz);
			task->langbyte1=0;
		} else {
			cons_putstr0(cons, ".zuv file format error.\n");
		}
		memman_free_4k(memman, (int) p, finfo->size);
		cons_newline(cons);
		return 1;
	}
	return 0;
}
int *zuv_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
	TASK *task=task_now();
	int ds_base= task->ds_base;
	CONSOLE *cons = task->cons;
	SHTCTL *shtctl = (SHTCTL *) *((int *) 0x0fe4);
	SHEET *sht;
	FIFO32 *sys_fifo = (FIFO32 *) *((int *)0x0fec);
	int *reg = &eax +1;
	int i;
	FILEINFO *finfo;
	FILEHANDLE *fh;
	MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
	if (edx == 1) { //a char 
		cons_putchar(cons, eax & 0xff, 1);
	}
 	else if (edx == 2) { // str0
		cons_putstr0(cons, (char *) ebx+ds_base);
	}
 	else if (edx == 3) { // str1(pos,num)
		cons_putstr1(cons, (char *) ebx+ds_base, ecx);
	}
	else if (edx == 4){ //end
		return &(task->tss.esp0);
	}
	else if (edx == 5 ){ //openwin
		sht = sheet_alloc(shtctl);
		sht->task=task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
		make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
		sheet_slide(sht, (shtctl->xsize-esi)/2&~3,(shtctl->ysize-edi)/2);
		sheet_updown(sht, shtctl->top);	
		reg[7] = (int) sht;
	}
	else if (edx == 6){ // putstrwin
		sht = (SHEET *)(ebx & 0xfffffffe);
		putfonts8_asc(sht->buf,sht->bxsize,esi,edi,eax,(char *)ebp+ds_base);
		if((ebx &1)==0){
			sheet_refresh(sht,esi,edi,esi+ecx*8,edi+16);
		}
	}
	else if (edx == 7){ //boxfilwin
		sht = (SHEET *)(ebx & 0xfffffffe);
		boxfill8(sht->buf,sht->bxsize,ebp,eax,ecx,esi,edi);
		if((ebx & 1)==0){
			sheet_refresh(sht,eax,ecx,esi+1,edi+1);
		}
	}
	else if (edx == 8) { //initmalloc
		memman_init(( MEMMAN *) (ebx + ds_base));
		ecx &= 0xfffffff0;
		memman_free(( MEMMAN *) (ebx + ds_base), eax, ecx);
	}
	else if (edx == 9) { //malloc
		ecx = (ecx + 0x0f) & 0xfffffff0;
		reg[7] = memman_alloc((MEMMAN *) (ebx + ds_base), ecx);
	}
	else if (edx == 10) { //free
		ecx = (ecx + 0x0f) & 0xfffffff0; 
		memman_free((MEMMAN *) (ebx + ds_base), eax, ecx);
	}
	else if (edx == 11){ //point
		sht = (SHEET*) (ebx & 0xfffffffe);
		sht->buf[sht->bxsize*edi+esi] =eax;
		if((ebx & 1)==0){
			sheet_refresh(sht,esi,edi,esi+1,edi+1);
		}
	}
	else if (edx == 12){ //refreshwin
		sht = (SHEET *)ebx;
		sheet_refresh(sht,eax,ecx,esi,edi);
	}
	else if (edx ==13){ //linewin
		sht = (SHEET *) (ebx & 0xfffffffe);
		zuv_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	else if(edx ==14){ //closewin
		sheet_free((SHEET *)ebx);
	}
	else if (edx == 15){ //getkey
		for (;;) {
			io_cli();
			if (fifo32_status(&task->fifo) == 0) {
				if (eax != 0) {
					task_sleep(task);	
				} else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) {
				timer_init(cons->timer, &task->fifo, 1); 
				timer_settime(cons->timer, 50);
			}
			if (i == 2) {
				cons->cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {
				cons->cursor_c = -1;
			}
			if (i == 4) {
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo,cons->sht - shtctl->sheets0 +2024);
				cons->sht =0;
				io_sti();
			}
			if (i >=256) { 
				reg[7] = i - 256;
				return 0;
			}
		}
	}
	else if (edx == 16){ //alloctimer
		reg[7] =(int) timer_alloc();
		((TIMER *)reg[7])->flags2 =1;
	}
	else if (edx ==17){ //inittimer
		timer_init((TIMER *)ebx,&task->fifo,eax+256);
	}
	else if (edx ==18){ //settimer
		timer_settime((TIMER *)ebx,eax);
	}
	else if (edx ==19){ //freetimer
		timer_free((TIMER *)ebx);
	}
	else if (edx == 20){ //beep
		if(eax == 0){
			i = io_in8(0x61);
			io_out8(0x61,i & 0x0d);
		}
		else{
			i =1193180000/eax;
			io_out8(0x43,0xb6);
			io_out8(0x42,i & 0xff);
			io_out8(0x42,i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03)& 0x0f);
		}
	}
	else if (edx == 21) { //fopen
		for (i = 0; i < 8; i++) {
			if (task->fhandle[i].buf == 0) {
				break;
			}
		}
		fh = &task->fhandle[i];
		reg[7] = 0;
		if (i < 8) {
			finfo = file_search((char *) ebx + ds_base,
					(FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
			if (finfo != 0) {
				reg[7] = (int) fh;
				fh->buf = (char *) memman_alloc_4k(memman, finfo->size);
				fh->size = finfo->size;
				fh->pos = 0;
				file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, (char *) (ADR_DISKIMG + 0x003e00));
			}
		}
	}
	else if (edx == 22) { //fclose
		fh = (FILEHANDLE *) eax;
		memman_free_4k(memman, (int) fh->buf, fh->size);
		fh->buf = 0;
	}
	else if (edx == 23) { //fseek
		fh = (FILEHANDLE *) eax;
		if (ecx == 0) {
			fh->pos = ebx;
		} else if (ecx == 1) {
			fh->pos += ebx;
		} else if (ecx == 2) {
			fh->pos = fh->size + ebx;
		}
		if (fh->pos < 0) {
			fh->pos = 0;
		}
		if (fh->pos > fh->size) {
			fh->pos = fh->size;
		}
	}
	else if (edx == 24) { //fsize
		fh = (FILEHANDLE *) eax;
		if (ecx == 0) {
			reg[7] = fh->size;
		} else if (ecx == 1) {
			reg[7] = fh->pos;
		} else if (ecx == 2) {
			reg[7] = fh->pos - fh->size;
		}
	}
	else if (edx == 25) { //fread
		fh = (FILEHANDLE *) eax;
		for (i = 0; i < ecx; i++) {
			if (fh->pos == fh->size) {
				break;
			}
			*((char *) ebx + ds_base + i) = fh->buf[fh->pos];
			fh->pos++;
		}
		reg[7] = i;
	}
	else if (edx == 26) { //cmdline
		i = 0;
		for (;;) {
			*((char *) ebx + ds_base + i) =  task->cmdline[i];
			if (task->cmdline[i] == 0) {
				break;
			}
			if (i >= ecx) {
				break;
			}
			i++;
		}
		reg[7] = i;
	}
	else if( edx ==27 ){ //getline
		reg[7]=task->langmode;
	}
	 else if (edx == 28) { //fopne_w
		for (i = 0; i < 8; i++) {
			if (task->fhandle[i].buf == 0) {
				break;
			}
		}
		fh = &task->fhandle[i];
		reg[7] = 0;
		if (i < 8) {
			fh->bsize   = 4096;
			fh->pos     = 0;
			fh->bpos    = 0;
			fh->clustno = 0;
			fh->buf     = (char *) memman_alloc_4k(memman, fh->bsize);
			finfo = file_search((char *) ebx + ds_base,
					(FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
			if (finfo != 0) {
				fh->finfo = finfo;
				finfo->size = 0;
			} else {
				finfo = file_insert((char *) ebx + ds_base,
						(FILEINFO *) (ADR_DISKIMG + 0x002600), 224, task->fat);
				fh->finfo = finfo;
			}
			if (finfo != 0) {
				reg[7]   = (int) fh;
				fh->size = finfo->size;
			}
		}
	} else if (edx == 29) { //fclose_r
		fh = (FILEHANDLE *) eax;
		if (fh->bpos != 0) {
			file_savefile(fh, fh->bpos, task->fat, (char *) (ADR_DISKIMG + 0x003e00));
		}
		memman_free_4k(memman, (int) fh->buf, fh->bsize);
		fh->buf = 0;
	} else if (edx == 30) { //fwrite
		int size = ecx, dsize, wsize = 0, usize = 4096;
		char *sbuf = (char *) ebx + ds_base;
		fh = (FILEHANDLE *) eax;
		for (; usize == 4096;) {
			if (fh->bpos + size < 4096) {
				memcpy(fh->buf + fh->bpos, sbuf, size);
				fh->pos  += size;
				fh->bpos += size;
				wsize    += size;
				break;
			}
			dsize = 4096 - fh->bpos;
			memcpy(fh->buf + fh->bpos, sbuf, dsize);
			usize = file_savefile(fh, 4096, task->fat, (char *) (ADR_DISKIMG + 0x003e00));
			size     -= dsize;
			sbuf     += dsize;
			fh->pos  += dsize;
			wsize    += (usize - fh->bpos);
			fh->bpos  = 0;
		}
		reg[7] = wsize;
	}

	return 0;
}

int *inthandler0c(int *esp){
	TASK *task = task_now();
	CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0c :\n Stack Exception.\n");
	sprintf(s,"EIP=%08x\n",esp[11]);
	cons_putstr0(cons,s);
	return &(task->tss.esp0);
}

int *inthandler0d(int *esp){
	TASK *task =task_now();
  CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	sprintf(s,"EIP =%08x\n",esp[11]);
	cons_putstr0(cons,s);
	return &(task->tss.esp0);
}

void zuv_api_linewin(SHEET *sht, int x0, int y0, int x1, int y1, int col){
	int i, x, y, len, dx, dy;

	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0) {
		dx = - dx;
	}
	if (dy < 0) {
		dy = - dy;
	}
	if (dx >= dy) {
		len = dx + 1;
		if (x0 > x1) {
			dx = -1024;
		}
	 	else {
			dx =  1024;
		}

		if (y0 <= y1) {
			dy = ((y1 - y0 + 1) << 10) / len;
		}
	 	else {
			dy = ((y1 - y0 - 1) << 10) / len;
		}

	} 
	else {
		len = dy + 1;
		if (y0 > y1) {
			dy = -1024;
		}
	 	else {
			dy =  1024;
		}

		if (x0 <= x1) {
			dx = ((x1 - x0 + 1) << 10) / len;
		}
	 	else {
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}

	for (i = 0; i < len; i++) {
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}

	return;
}
