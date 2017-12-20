#include<stdio.h>
#include"bootpack.h"

typedef struct{
	unsigned char buf[3],phase;
	int x,y,btn;
}MOUSE_DEC;
 
extern FIFO8 keyfifo,mousefifo;
void enable_mouse(MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(MOUSE_DEC *mdec,unsigned char dat);

void ZuviMain(void){
	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	char s[101],mccursor[256],keybuf[32],mousebuf[128];
	int mx,my,i;
	MOUSE_DEC mdec;

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

	enable_mouse(&mdec);
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
				//マウスのデータは3byteずつ
				if (mouse_decode(&mdec,i)!= 0) {
					sprintf(s, "[lcr %4d %4d]",mdec.x,mdec.y);
					if((mdec.btn&0x01)!=0){
						s[1]='L';
					}
					if((mdec.btn&0x02)!=0){
						s[3]='R';
					}
					if((mdec.btn&0x04)!=0){
						s[2]='C';
					}
					boxfill8(boot_info->vram, boot_info->screen_x, COL8_840084,32,16,32+15* 8-1,31);
					putfonts8_asc(boot_info->vram, boot_info->screen_x,32,16,COL8_FFFFFF,s);

					//マウス移動
					boxfill8(boot_info->vram,boot_info->screen_x,COL8_840084,mx,my,mx+15,my+15);
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0)
						mx=0;
					if(my<0)
						my=0;
					if(mx>boot_info->screen_x-16)
						mx=boot_info->screen_x-16;
					if(my>boot_info->screen_y-16)
						mx=boot_info->screen_y-16;
					
					sprintf(s,"(%3d,%3d)",mx,my);
					boxfill8(boot_info->vram,boot_info->screen_x,COL8_840084,0,0,79,15);
					putfonts8_asc(boot_info->vram,boot_info->screen_x,0,0,COL8_FFFFFF,s);
					putblock8_8(boot_info->vram,boot_info->screen_x,16,16,mx,my,mccursor,16);

				}
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

void enable_mouse(MOUSE_DEC *mdec){
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_keyboard_controller_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	mdec->phase=0;
	return;
}

int mouse_decode(MOUSE_DEC *mdec, unsigned char dat){
	if (mdec->phase == 0) {
			if (dat == 0xfa) {
						mdec->phase = 1;
					}
			return 0;
		}
	if (mdec->phase == 1) {
		if((dat&0xc8)==0x08){
			mdec->buf[0] = dat; //クリック、移動
			mdec->phase = 2;
		}
			return 0;
		}
	if (mdec->phase == 2) {
			mdec->buf[1] = dat; //左右
			mdec->phase = 3;
			return 0;
		}
	if (mdec->phase == 3) {
			mdec->buf[2] = dat; //上下
			mdec->phase = 1;
			mdec->btn=mdec->buf[0]& 0x07;
			mdec->x=mdec->buf[1];
			mdec->y=mdec->buf[2];
			if((mdec->buf[0]&0x10)!=0){
				mdec->x|=0xffffff00;
			}
			if((mdec->buf[0]&0x20)!=0){
				mdec->y|=0xffffff00;
			}
			mdec->y=-mdec->y;
			return 1;
		}
	return -1;
}
