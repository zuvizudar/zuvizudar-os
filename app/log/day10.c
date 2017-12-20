#include<stdio.h>
#include"bootpack.h"

void ZuviMain(void){
	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	char s[101],mccursor[256],keybuf[32],mousebuf[128];
	int mx,my,i;
	unsigned int memtotal;
	MOUSE_DEC mdec;
	MEMMAN *memman=(MEMMAN*)MEMMAN_ADDR;
	SHTCTL *shtctl;
	SHEET *sht_back,*sht_mouse;
	unsigned char *buf_back,buf_mouse[256];

	init_gdtidt();
	init_pic();
	io_sti();
	fifo8_init(&keyfifo,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);
	io_out8(PIC0_IMR,0xf9);//キーボード
	io_out8(PIC1_IMR,0xef);//マウス

	init_keyboard();
	enable_mouse(&mdec);
	memtotal=memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman,0x00001000,0x0009e000);
	memman_free(memman,0x00400000,memtotal-0x00400000);

	init_palette();
	shtctl=shtctl_init(memman,boot_info->vram,boot_info->screen_x,boot_info->screen_y);
	sht_back=sheet_alloc(shtctl);
	sht_mouse=sheet_alloc(shtctl);
	buf_back=(unsigned char *)memman_alloc_4k(memman,boot_info->screen_x*boot_info->screen_y);
	sheet_setbuf(sht_back,buf_back,boot_info->screen_x,boot_info->screen_y,-1);
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);
	init_screen(buf_back,boot_info->screen_x,boot_info->screen_y);
	init_mouse_cursor8(buf_mouse,99);
	sheet_slide(shtctl,sht_back,0,0);
	mx=(boot_info->screen_x-16)/2;
	my=(boot_info->screen_y-28-16)/2;
	sheet_slide(shtctl,sht_mouse,mx,my);
	sheet_updown(shtctl,sht_back,0);
	sheet_updown(shtctl,sht_mouse,1);
	//putblock8_8(boot_info->vram,boot_info->screen_x,16,16,mx,my,mccursor,16);
	sprintf(s,"(%3d,%3d)",mx,my);
	//putfonts8_asc(boot_info->vram,boot_info->screen_x,0,0,COL8_FFFFFF,s);
	putfonts8_asc(buf_back,boot_info->screen_x,0,0,COL8_FFFFFF,s);

	sprintf(s,"memory %dMB free:%dKB",memtotal/(1024*1024),memman_total(memman)/1024);
	putfonts8_asc(buf_back,boot_info->screen_x,0,32,COL8_FFFFFF,s);
	sheet_refresh(shtctl,sht_back,0,0,boot_info->screen_x,48);

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
				boxfill8(buf_back,boot_info->screen_x,COL8_840084,0,16,15,31);
				putfonts8_asc(buf_back,boot_info->screen_x,0,16,COL8_FFFFFF,s);
				sheet_refresh(shtctl,sht_back,0,16,16,32);
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
					boxfill8(buf_back, boot_info->screen_x, COL8_840084,32,16,32+15* 8-1,31);
					putfonts8_asc(buf_back, boot_info->screen_x,32,16,COL8_FFFFFF,s);
					sheet_refresh(shtctl,sht_back,32,16,32+15*8,32);

					//マウス移動
				//	boxfill8(buf_back,boot_info->screen_x,COL8_840084,mx,my,mx+15,my+15);
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
					boxfill8(buf_back,boot_info->screen_x,COL8_840084,0,0,79,15);
					putfonts8_asc(buf_back,boot_info->screen_x,0,0,COL8_FFFFFF,s);
					//putblock8_8(buf_back,boot_info->screen_x,16,16,mx,my,mccursor,16);
					sheet_refresh(shtctl,sht_back,0,0,80,16);
					sheet_slide(shtctl,sht_mouse,mx,my);

				}
			}
		}
	}
}

