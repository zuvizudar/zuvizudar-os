//asmhead.nas
typedef struct{
	char cyls,leds,vmode,reserve;
	short screen_x,screen_y;
	char *vram;
}BOOTINFO;

#define ADR_BOOTINFO 0x00000ff0

//naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void write_mem8(int addr, int data);
void io_out8(int port,int data);
void io_out16(int port,int data);
void io_out32(int port,int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit,int addr);
void load_idtr(int limit,int addr);
int load_cr0(void);
void store_cr0(int cr0);
void asm_inthandler21(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start,unsigned int end);

//fifo.c
typedef struct{
	unsigned char data[32],next;
	int next_r,next_w,len;
}KEYBUF;
typedef struct{
	unsigned char *buf;
	int p,q,size,free,flags;
}FIFO8;
void fifo8_init(FIFO8 *fifo,int size,unsigned char *buf);
int fifo8_put(FIFO8 *fifo,unsigned char data);
int fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);

//graphic.c
void init_palette(void);
void set_paletta(int start,int end,unsigned char *rgb);
void boxfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen(char *vram,int x,int y);
void putfont8(char *vram,int xsize,int x,int y,char c,char *font);
void putfonts8_asc(char *vram,int xsize,int x,int y,char c,unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, char *buf, int bxsize);

#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

//dsctbl.c
typedef struct{
	short limit_low,base_low;
	char base_mid,access_right;
	char limit_high,base_high;
}SEGMENT_DESCRIPTOR;

typedef struct{
	short offset_low,selector;
	char dw_count,access_right;
	short offset_high;
}GATE_DESCRIPTOR;

void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


//int.c

void init_pic(void);

//IMR...割り込み目隠し
//ICW...初期化制御
//0x00~0x19はCPUシステム保護に使われている
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

//mouse.c
typedef struct{
	unsigned char buf[3],phase;
	int x,y,btn;
}MOUSE_DEC;
void inthandler2c(int *esp);
void enable_mouse(MOUSE_DEC *mdec);
int mouse_decode(MOUSE_DEC *mdec,unsigned char dat);
extern FIFO8 mousefifo;

//keyboard.c
void inthandler21(int *esp);
void wait_keyboard_controller_sendready(void);
void init_keyboard(void);
extern FIFO8  keyfifo;
#define PORT_KEYDAT 0x0060 
#define PORT_KEYCMD 0x0064 
