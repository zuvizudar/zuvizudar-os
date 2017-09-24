//asmhead.nas
typedef struct{
	char cyls,leds,vmode,reserve;
	short screen_x,screen_y;
	char *vram;
}BOOTINFO;

#define ADR_BOOTINFO 0x00000ff0
#define ADR_DISKIMG 0x00100000

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
void load_tr(int tr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_inthandler0d(void);
void asm_inthandler0c(void);
unsigned int memtest_sub(unsigned int start,unsigned int end);
void farjmp(int eip,int cs);
void farcall(int eip,int cs);
void asm_cons_putchar(void);
void asm_zuv_api(void);
void start_app(int eip,int cs,int esp,int ds,int *tss_esp0);
void asm_end_app(void);

//fifo.c

typedef struct _TASK TASK;

typedef struct{
	unsigned char data[32],next;
	int next_r,next_w,len;
}KEYBUF;
typedef struct{
	unsigned char *buf;
	int p,q,size,free,flags;
}FIFO8;
typedef struct{
	int *buf;
	int p,q,size,free,flags;	
	TASK *task;
}FIFO32;
/*void fifo8_init(FIFO8 *fifo,int size,unsigned char *buf);
int fifo8_put(FIFO8 *fifo,unsigned char data);
int fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);
*/
void fifo32_init(FIFO32 *fifo,int size,int *buf,TASK *task);
int fifo32_put(FIFO32 *fifo,int data);
int fifo32_get(FIFO32 *fifo);
int fifo32_status(FIFO32 *fifo);


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
#define AR_TSS32			0x0089
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
void enable_mouse(FIFO32 *fifo,int data,MOUSE_DEC *mdec);
int mouse_decode(MOUSE_DEC *mdec,unsigned char dat);
//extern FIFO8 mousefifo;

//keyboard.c
void inthandler21(int *esp);
void wait_keyboard_controller_sendready(void);
void init_keyboard(FIFO32 *fifo,int data0);
//extern FIFO8  keyfifo;
#define PORT_KEYDAT 0x0060 
#define PORT_KEYCMD 0x0064 

//memory.c
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
unsigned int memman_alloc_4k(MEMMAN *man,unsigned int size);
int memman_free_4k(MEMMAN *man,unsigned int addr,unsigned int size);
//sheet
#define MAX_SHEETS 256

typedef struct _SHTCTL SHTCTL;
typedef struct _SHEET SHEET;
struct _SHEET{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	SHTCTL *ctl;
	TASK *task;
};

struct _SHTCTL{
	unsigned char *vram,*map;
	int xsize, ysize, top;
	SHEET *sheets[MAX_SHEETS];
	SHEET sheets0[MAX_SHEETS];
};
//typedef struct _SHEET SHEET;
//typedef struct _SHTCTL SHTCTL;


SHTCTL *shtctl_init(MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
SHEET *sheet_alloc(SHTCTL *ctl);
void sheet_setbuf(SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(SHEET *sht, int height);
void sheet_refresh(SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide( SHEET *sht, int vx0, int vy0);
void sheet_free(SHEET *sht);

/* timer.c */
#define MAX_TIMER		500
typedef struct _TIMER TIMER;
typedef struct _TIMERCTL TIMERCTL;
struct _TIMER {
	TIMER *next_timer;
	unsigned int timeout;
	char flags,flags2;
	FIFO32 *fifo;
	int data;
};
struct _TIMERCTL {
	unsigned int count, next_time;
	TIMER *t0;
	TIMER timers0[MAX_TIMER];
};
extern TIMERCTL timerctl;
void init_pit(void);
TIMER *timer_alloc(void);
void timer_free(TIMER *timer);
void timer_init(TIMER *timer,FIFO32 *fifo, int data);
void timer_settime(TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);
int timer_cancel(TIMER *timer);
void timer_cancelall(FIFO32 *fifo);

#define MAX_TASKS 1000
#define MAX_TASKS_LV 100
#define MAX_TASKLEVELS 10
#define TASK_GDT0 3
typedef struct {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
}TSS32;

typedef struct _CONSOLE CONSOLE;

struct _TASK{
	int sel,flags;
	int priority,level;
	FIFO32 fifo;
	TSS32 tss;
	CONSOLE *cons;
	int ds_base;
};

typedef struct{
	int running;
	int now;
	TASK *tasks[MAX_TASKS_LV];
}TASKLEVEL;

typedef struct{
	int now_lv;
	char lv_change;
	TASKLEVEL level[MAX_TASKLEVELS];
	TASK tasks0[MAX_TASKS];
}TASKCTL;
extern TIMER *task_timer;
TASK *task_init(MEMMAN *memman);
TASK *task_now(void);
TASK *task_alloc(void);
void task_run(TASK *task,int level,int priority);
void task_switch(void);
void task_sleep(TASK *task);

//window
void make_window8(unsigned char *buf, int xsize,int ysize,char *title,char act);
void putfonts8_asc_sht(SHEET *sth,int x,int y,int c,int b,char *s,int l);
void make_textbox8(SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf,int xsize,char *title,char act);
void change_wtitle8(SHEET *sht,char act);
//console
struct _CONSOLE{
	SHEET *sht;
	int cursor_x,cursor_y,cursor_c;
	TIMER *timer;
};
void console_task(SHEET *sheet,int memtotal);
void cons_putchar(CONSOLE *cons,int chr,char move);
void cons_newline(CONSOLE *cons);
void cons_putstr0(CONSOLE *cons,char *s);
void cons_putstr1(CONSOLE *cons,char *s,int l);
void cons_runcmd(char *cmdline,CONSOLE *cons,int *fat,int memtotal);
void cmd_mem(CONSOLE *cons,int memtotal);
void cmd_clear(CONSOLE *cons);
void cmd_ls(CONSOLE *cons);
void cmd_cat(CONSOLE *cons,int *fat,char *cmdline);
int cmd_app(CONSOLE *cons, int *fat,char *cmdline);
int *zuv_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0d(int *esp);
int *inthandler0c(int *esp);
void zuv_api_linewin(SHEET *sht,int x0,int y0,int x1,int y1,int col);

//file
typedef struct{
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
}FILEINFO;

void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
FILEINFO *file_search(char *name,FILEINFO *finfo,int max);

