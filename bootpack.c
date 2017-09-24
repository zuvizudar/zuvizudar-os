#include<stdio.h>
#include<string.h>
#include"bootpack.h"

#define KEYCMD_LED 0xed
void ZuviMain(void){

	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	FIFO32 fifo,keycmd;
	char s[40];
	int fifobuf[128],keycmd_buf[32];
	int mx,my,i,cursor_x,cursor_c;//count=0;
	unsigned int memtotal;
	MOUSE_DEC mdec;
	MEMMAN *memman=(MEMMAN*)MEMMAN_ADDR;
	SHTCTL *shtctl;
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	unsigned char *buf_back,buf_mouse[256],*buf_win,*buf_cons;
	SHEET *sht_back,*sht_mouse,*sht_win,*sht_cons;
	TASK *task_a,*task_cons;
	TIMER *timer;
	int key_to=0,key_shift=0,key_leds=(boot_info->leds >> 4)&7,keycmd_wait=1;
	CONSOLE *cons;

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
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp +8 )) =memtotal;
	task_run(task_cons, 2, 2);

	/*//win_b
	for (i = 0; i < 3; i++) {
		sht_win_b[i] = sheet_alloc(shtctl);
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
		sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1);
		sprintf(s, "task_b%d", i);
		make_window8(buf_win_b, 144, 52, s, 0);
		task_b[i] = task_alloc();
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];
	//	task_run(task_b[i],2,i+1);
	}
	*/
	//win
	sht_win=sheet_alloc(shtctl);
	buf_win=(unsigned char *)memman_alloc_4k(memman,144*52);
	sheet_setbuf(sht_win,buf_win,144,52,-1);
  make_window8(buf_win,144,52,"task_a",1);
	make_textbox8(sht_win,8,28,128,16,COL8_FFFFFF);
	cursor_x=8;
	cursor_c=COL8_FFFFFF;
	timer=timer_alloc();
	timer_init(timer,&fifo,1);
	timer_settime(timer,50);
	
	//mouse
	sht_mouse=sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
	init_mouse_cursor8(buf_mouse,99);
	mx=(boot_info->screen_x-16)/2;
	my=(boot_info->screen_y-28-16)/2;

	sheet_slide(sht_back,0,0);
	sheet_slide(sht_mouse,mx,my);
	sheet_slide(sht_win,8,56);
	sheet_slide(sht_cons,32,4);
/*	sheet_slide(sht_win_b[0],168,56);
	sheet_slide(sht_win_b[1],8,116);
	sheet_slide(sht_win_b[2],168,116);
*/
	sheet_updown(sht_back,0);

/*	sheet_updown(sht_win_b[0],1);
	sheet_updown(sht_win_b[1],2);
	sheet_updown(sht_win_b[2],3);*/
	sheet_updown(sht_cons,1);
	sheet_updown(sht_win,2);
	sheet_updown(sht_mouse,3);
	/*sprintf(s,"(%3d,%3d)",mx,my);
	putfonts8_asc_sht(sht_back,0,0,COL8_FFFFFF,COL8_840084,s,10);

	sprintf(s,"memory %dMB free:%dKB",memtotal/(1024*1024),memman_total(memman)/1024);
	putfonts8_asc_sht(sht_back,0,32,COL8_FFFFFF,COL8_840084,s,40);
	*/
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
			if(256<=i&&i<=511){
		/*		sprintf(s,"%02x",i-256);
				putfonts8_asc_sht(sht_back,0,16,COL8_FFFFFF,COL8_840084,s,2);
				*/
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
				if(s[0]!=0){
					if(key_to==0){
						if(cursor_x<128){
							s[1]=0;
							putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);
							cursor_x+=8;
						}
					}
					else{
						fifo32_put(&task_cons->fifo,s[0]+256);
					}
				}
				if(i == 256 + 0x0e) {
					if (key_to == 0) {
						if (cursor_x > 8) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if(i == 256+0x1c){
					if(key_to!=0){
						fifo32_put(&task_cons->fifo,10+256);
					}
				}
				if (i == 256 + 0x0f) { //TAB
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c=-1;
						boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
						fifo32_put(&task_cons->fifo,2);
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c=COL8_000000;
						fifo32_put(&task_cons->fifo,3);
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
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
				if (i == 256 + 0xb6) {
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
				if (i ==256 +0x3b && key_shift != 0 && task_cons->tss.ss0 != 0){
					cons =(CONSOLE *) *((int *)0x0fec);
					cons_putstr0(cons,"\nBreak(key):\n");
					io_cli();
					task_cons->tss.eax=(int)&(task_cons->tss.esp0);
					task_cons->tss.eip=(int)asm_end_app;
					io_sti();
				}
				if (i == 256 + 0xfa) {
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {
					wait_keyboard_controller_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if(cursor_c<=0){
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
				}
				boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
				sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
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
					if((mdec.btn & 0x01 )!=0){
						sheet_slide(sht_win,mx-80,my-8);
					}

				}
			} 
			else if(i<=1){
				if(i!=0){
					timer_init(timer,&fifo,0);
					if(cursor_c>=0){
						cursor_c=COL8_000000;
					}
				}
				else{
					timer_init(timer,&fifo,1);
					if(cursor_c>=0){
						cursor_c=COL8_FFFFFF;
					}
				}
				timer_settime(timer,50);
				if(cursor_c>=0){
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
					sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
				}
			}
		}
	}
}



