#include<stdio.h>
#include"bootpack.h"

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
