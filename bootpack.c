#include<stdio.h>
#include"bootpack.h"

#define MEMMAN_FREES 4090
#define MEMMAN_ADDR 0x003c0000
typedef struct{
	unsigned int addr,size;
}FREEINFO;
typedef struct{
	int frees,maxfrees,lostsize,losts;
	FREEINFO free[MEMMAN_FREES];
}MEMMAN;


unsigned int memtest(unsigned int start,unsigned int end);
void memman_init(MEMMAN *man);
unsigned int memman_total(MEMMAN *man);
unsigned int memman_alloc(MEMMAN *man, unsigned int size);
int memman_free(MEMMAN *man, unsigned int addr, unsigned int size);

void ZuviMain(void){
	BOOTINFO *boot_info=(BOOTINFO*)0x0ff0;
	char s[101],mccursor[256],keybuf[32],mousebuf[128];
	int mx,my,i;
	unsigned int memtotal;
	MOUSE_DEC mdec;
	MEMMAN *memman=(MEMMAN*)MEMMAN_ADDR;

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
	init_screen(boot_info->vram,boot_info->screen_x,boot_info->screen_y);
	mx=(boot_info->screen_x-16)/2;
	my=(boot_info->screen_y-28-16)/2;
	init_mouse_cursor8(mccursor,COL8_840084);
	putblock8_8(boot_info->vram,boot_info->screen_x,16,16,mx,my,mccursor,16);
	sprintf(s,"(%3d,%3d)",mx,my);
	putfonts8_asc(boot_info->vram,boot_info->screen_x,0,0,COL8_FFFFFF,s);

	sprintf(s,"memory %dMB free:%dKB",memtotal/(1024*1024),memman_total(memman)/1024);
	putfonts8_asc(boot_info->vram,boot_info->screen_x,0,32,COL8_FFFFFF,s);

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
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start,unsigned int end){
	char flg486=0;
	unsigned int eflg,cr0,i;

	eflg=io_load_eflags();
	eflg|=EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg=io_load_eflags();
	if((eflg&EFLAGS_AC_BIT)!=0){
		flg486=1;
	}
	eflg&=~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486!=0){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	i=memtest_sub(start,end);
	if(flg486!=0){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	return i;
}
void memman_init(MEMMAN *man)
{
	man->frees = 0;			/* あき情報の個数 */
	man->maxfrees = 0;		/* 状況観察用：freesの最大値 */
	man->lostsize = 0;		/* 解放に失敗した合計サイズ */
	man->losts = 0;			/* 解放に失敗した回数 */
	return;
}

unsigned int memman_total(MEMMAN *man)
/* あきサイズの合計を報告 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(MEMMAN *man, unsigned int size)
/* 確保 */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 十分な広さのあきを発見 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i]がなくなったので前へつめる */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 構造体の代入 */
				}
			}
			return a;
		}
	}
	return 0; /* あきがない */
}

int memman_free(MEMMAN *man, unsigned int addr, unsigned int size)
/* 解放 */
{
	int i, j;
	/* まとめやすさを考えると、free[]がaddr順に並んでいるほうがいい */
	/* だからまず、どこに入れるべきかを決める */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前がある */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 前のあき領域にまとめられる */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 後ろもある */
				if (addr + size == man->free[i].addr) {
					/* なんと後ろともまとめられる */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]の削除 */
					/* free[i]がなくなったので前へつめる */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 構造体の代入 */
					}
				}
			}
			return 0; /* 成功終了 */
		}
	}
	/* 前とはまとめられなかった */
	if (i < man->frees) {
		/* 後ろがある */
		if (addr + size == man->free[i].addr) {
			/* 後ろとはまとめられる */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功終了 */
		}
	}
	/* 前にも後ろにもまとめられない */
	if (man->frees < MEMMAN_FREES) {
		/* free[i]より後ろを、後ろへずらして、すきまを作る */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 最大値を更新 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功終了 */
	}
	/* 後ろにずらせなかった */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失敗終了 */
}
