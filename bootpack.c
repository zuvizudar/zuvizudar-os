#include<stdio.h>
#include<string.h>
#include"bootpack.h"

#define KEYCMD_LED 0xed
void keywin_off(SHEET *key_win);
void keywin_on(SHEET *key_win);
void ZuviMain(void){

	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	SHTCTL *shtctl;
	char s[40];
	FIFO32 fifo,keycmd;
	int fifobuf[128],keycmd_buf[32],*cons_fifo[2];
	int mx,my,i;
	unsigned int memtotal;
	MOUSE_DEC mdec;
	MEMMAN *memman=(MEMMAN*)MEMMAN_ADDR;
	unsigned char *buf_back,buf_mouse[256],*buf_cons[2];
	SHEET *sht_back,*sht_mouse,*sht_cons[2];
	TASK *task_a,*task_cons[2],*task;
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
	int key_shift=0,key_leds=(boot_info->leds >> 4)&7,keycmd_wait=-1;
	int j,x,y,mmx=-1,mmy=-1;
	SHEET *sht=0,*key_win;

	init_gdtidt();
	init_pic();
	io_sti();
	fifo32_init(&fifo,32,fifobuf,0);
	init_pit();
	init_keyboard(&fifo,256);
	enable_mouse(&fifo,512,&mdec);
	io_out8(PIC0_IMR,0xf8);//キーボード
	io_out8(PIC1_IMR,0xef);//マウス
	fifo32_init(&keycmd,32,keycmd_buf,0);

	memtotal=memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman,0x00001000,0x0009e000);
	memman_free(memman,0x00400000,memtotal-0x00400000);

	init_palette();
	shtctl=shtctl_init(memman,boot_info->vram,boot_info->screen_x,boot_info->screen_y);
	task_a=task_init(memman);
	fifo.task=task_a;
	task_run(task_a,1,2);
	*((int *)0x0fe4) = (int) shtctl;
	//back
	sht_back=sheet_alloc(shtctl);
	buf_back=(unsigned char *)memman_alloc_4k(memman,boot_info->screen_x*boot_info->screen_y);
	sheet_setbuf(sht_back,buf_back,boot_info->screen_x,boot_info->screen_y,-1);
	init_screen(buf_back,boot_info->screen_x,boot_info->screen_y);

	//console
	for(i=0;i<2;i++){
		sht_cons[i] = sheet_alloc(shtctl);
		buf_cons[i] = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
		sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1);
		make_window8(buf_cons[i], 256, 165, "console", 0);
		make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
		task_cons[i] = task_alloc();
		task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_cons[i]->tss.eip = (int) &console_task;
		task_cons[i]->tss.es = 1 * 8;
		task_cons[i]->tss.cs = 2 * 8;
		task_cons[i]->tss.ss = 1 * 8;
		task_cons[i]->tss.ds = 1 * 8;
		task_cons[i]->tss.fs = 1 * 8;
		task_cons[i]->tss.gs = 1 * 8;
		*((int *) (task_cons[i]->tss.esp + 4)) = (int) sht_cons[i];
		*((int *) (task_cons[i]->tss.esp +8 )) =memtotal;
		task_run(task_cons[i], 2, 2);
		sht_cons[i]->task = task_cons[i];
		sht_cons[i]->flags |= 0x20;  
		cons_fifo[i]=(int *)memman_alloc_4k(memman,128*4);
		fifo32_init(&task_cons[i]->fifo,128,cons_fifo[i],task_cons[i]);
	}
	//mouse
	sht_mouse=sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
	init_mouse_cursor8(buf_mouse,99);
	mx=(boot_info->screen_x-16)/2;
	my=(boot_info->screen_y-28-16)/2;

	sheet_slide(sht_back,0,0);
	sheet_slide(sht_mouse,mx,my);
	sheet_slide(sht_cons[1],56,6);
	sheet_slide(sht_cons[0],8,2);

	sheet_updown(sht_back,0);
	sheet_updown(sht_cons[0],1);
	sheet_updown(sht_cons[1],2);
	sheet_updown(sht_mouse,3);
	key_win = sht_cons[0];
	keywin_on(key_win);

	fifo32_put(&keycmd,KEYCMD_LED);
	fifo32_put(&keycmd,key_leds);
	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			keycmd_wait = fifo32_get(&keycmd);
			wait_keyboard_controller_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if(fifo32_status(&fifo)==0){
			task_sleep(task_a);
			io_sti();
		}
		else {
			i=fifo32_get(&fifo);
			io_sti();
			if(key_win->flags == 0){
				key_win = shtctl->sheets[shtctl->top -1];
				keywin_on(key_win);
			}
			if(256<=i&&i<=511){
				if (i < 0x80 + 256) {
					if (key_shift==0){
							s[0] = keytable0[i - 256];
					}
					else{
							s[0] = keytable1[i - 256];
					}
				}
				else{
					s[0]=0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {
					if (((key_leds & 4) == 0 && key_shift == 0) ||((key_leds & 4) != 0 && key_shift != 0)){ 
						s[0] += 0x20;
					}
				}
				if(s[0]!=0){ //通常、バック、enter
					fifo32_put(&key_win->task->fifo,s[0]+256);
				}
				if(i ==256 +0x0f){ //tab
					keywin_off(key_win);
					j=key_win->height-1;
					if(j==0){
						j=shtctl->top-1;
					}
					key_win=shtctl->sheets[j];
					keywin_on(key_win);
				}

				if (i == 256 + 0x2a){
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {
					key_shift &= ~1;
				}
				if(i == 256 + 0xb6) {
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i ==256 +0x3b && key_shift != 0){
					//shift+f1
					task=key_win->task;
					if(task!=0 && task->tss.ss0 != 0){
						cons_putstr0(task->cons,"\nBreak(key):\n");
						io_cli();
						task->tss.eax=(int)&(task->tss.esp0);
						task->tss.eip=(int)asm_end_app;
					io_sti();
					}
				}
				if(i==256+0x57){
					sheet_updown(shtctl->sheets[1],shtctl->top);
				}
				if (i == 256 + 0xfa) {
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {
					wait_keyboard_controller_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			}
			else if(512<=i&&i<=767){
				//マウスのデータは3byteずつ
				if (mouse_decode(&mdec,i-512)!= 0) {
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0)
						mx=0;
					if(my<0)
						my=0;
					if(mx>boot_info->screen_x-1)
						mx=boot_info->screen_x-1;
					if(my>boot_info->screen_y-1)
						mx=boot_info->screen_y-1;
					
				sheet_slide(sht_mouse,mx,my);
				if ((mdec.btn & 0x01) != 0) {
						if (mmx < 0) {
							for (j = shtctl->top - 1; j > 0; j--) {
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										sheet_updown(sht, shtctl->top - 1);
										if (sht != key_win) {
											keywin_off(key_win);
											key_win=sht;
											keywin_on(key_win);
										}
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
											mmx = mx;
											mmy = my;
										}
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {
											//xクリック
											if ((sht->flags & 0x10) != 0) {	
												task=sht->task;
												cons_putstr0(task->cons,"\nBreak(mouse):\n");
												io_cli();	
												task->tss.eax = (int) &(task->tss.esp0);
												task->tss.eip = (int) asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						else {
							x = mx - mmx;
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;
							mmy = my;
						}
					} 
					else {
						mmx = -1;
					}
				}
			}
		}
	}
}


void keywin_off(SHEET *key_win){
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3);
	}
	return;
}

void keywin_on(SHEET *key_win){
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2);
	}
	return;
}
