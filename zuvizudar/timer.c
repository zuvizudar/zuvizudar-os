#include "bootpack.h"
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	//確保
#define TIMER_FLAGS_USING		2	//作動中

void init_pit(void)
{
	int i;
	TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0;
	}
	t=timer_alloc();
	t->timeout=0xffffffff;
	t->flags=TIMER_FLAGS_USING;
	timerctl.t0=t;
	timerctl.next_time = 0xffffffff;
	return;
}

TIMER *timer_alloc(void){
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

void timer_free( TIMER *timer)
{
	timer->flags = 0;
	return;
}

void timer_init( TIMER *timer,  FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime( TIMER *timer, unsigned int timeout)
{
	int e;
	TIMER *t,*s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t=timerctl.t0;
	if(timer->timeout<=t->timeout){
		timerctl.t0=timer;
		timer->next_timer=t;
		timerctl.next_time=timer->timeout;
		io_store_eflags(e);
		return;
	}
	for(;;){
		s=t;
		t=t->next_timer;
		if(timer->timeout <= t->timeout){
			s->next_timer=timer;
			timer->next_timer=t;
			io_store_eflags(e);
			return;
		}
	}
	s->next_timer=timer;
	timer->next_timer=0;
	io_store_eflags(e);
	return;
}

void inthandler20(int *esp)
{
	TIMER *timer;
	char ts=0;
	io_out8(PIC0_OCW2, 0x60);	
	timerctl.count++;
	if (timerctl.next_time > timerctl.count) {
		return;
	}
	timer=timerctl.t0;
	for (;;) {
		if (timer->timeout > timerctl.count) {
			break;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		if(timer!=task_timer){
			fifo32_put(timer->fifo,timer->data);
		}
		else{
			ts=1;
		}
		timer=timer->next_timer;
	}
	timerctl.t0=timer;
	timerctl.next_time=timer->timeout;
	if(ts!=0){
		task_switch();
	}
	return;
}
int timer_cancel(TIMER *timer){
	int e;
	TIMER *t;
	e = io_load_eflags();
	io_cli();	
	if (timer->flags == TIMER_FLAGS_USING) {
		if (timer == timerctl.t0) {
			t = timer->next_timer;
			timerctl.t0 = t;
			timerctl.next_time = t->timeout;
		} else {
			t = timerctl.t0;
			for (;;) {
				if (t->next_timer == timer) {
					break;
				}
				t = t->next_timer;
			}
			t->next_timer = timer->next_timer;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;
	}
	io_store_eflags(e);
	return 0; 
}
void timer_cancelall(FIFO32 *fifo){
	int e, i;
	TIMER *t;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}
