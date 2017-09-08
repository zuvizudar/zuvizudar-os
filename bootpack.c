#include<stdio.h>
#include"bootpack.h"
void enable_mouse(void);
void init_keyboard(void);
extern FIFO8 keyfifo,mousefifo;
void ZuviMain(void){
	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	char s[101],mccursor[256],keybuf[32],mousebuf[128];
	int mx,my,i;

	init_gdtidt();
	init_pic();
	io_sti();
	fifo8_init(&keyfifo,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);
	io_out8(PIC0_IMR,0xf9);//キーボード
	io_out8(PIC1_IMR,0xef);//マウス

	init_keyboard();

	init_palette();
	init_screen(boot_info->vram,boot_info->screen_x,boot_info->screen_y);
	mx=(boot_info->screen_x-16)/2;
	my=(boot_info->screen_y-28-16)/2;
	init_mouse_cursor8(mccursor,COL8_840084);

	putblock8_8(boot_info->vram,boot_info->screen_x,16,16,mx,my,mccursor,16);
/*
	putfonts8_asc(boot_info->vram,boot_info->screen_x,8,8,COL8_FFFFFF,"ABC 123");
	putfonts8_asc(boot_info->vram,boot_info->screen_x,31,31,COL8_000000,"Zuvizudar OS");
	putfonts8_asc(boot_info->vram,boot_info->screen_x,30,30,COL8_FFFFFF,"Zuvizudar OS");

	sprintf(s,"screen_x=%d",boot_info->screen_x);
	putfonts8_asc(boot_info->vram,boot_info->screen_x,16,64,COL8_FFFFFF,s);
	*/
	enable_mouse();
	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo)+fifo8_status(&mousefifo)==0){
			io_stihlt();
		}
		else {
			if(fifo8_status(&keyfifo)!=0){
				i=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02x",i);
				boxfill8(boot_info->vram,boot_info->screen_x,COL8_840084,0,16,15,31);
				putfonts8_asc(boot_info->vram,boot_info->screen_x,0,16,COL8_FFFFFF,s);
			}
			else if(fifo8_status(&mousefifo)!=0){
				i=fifo8_get(&mousefifo);
				io_sti();
				sprintf(s,"%02x",i);
				boxfill8(boot_info->vram,boot_info->screen_x,COL8_840084,0,16,47,31);
				putfonts8_asc(boot_info->vram,boot_info->screen_x,32,16,COL8_FFFFFF,s);

			}
		}
	}
}


#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_keyboard_controller_sendready(void){
	for(;;){
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0){
			break;
		}
	}
}

void init_keyboard(void){
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return ;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void){
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	return;
}
