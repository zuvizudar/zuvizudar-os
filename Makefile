TOOLPATH=z_tools/
INCPATH=z_tools/zuvizudar/
OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj timer.obj mtask.obj window.obj console.obj file.obj

MAKE     = make -r
NASK = $(TOOLPATH)nask.exe
CC1 = $(TOOLPATH)cc1.exe -I $(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM = $(TOOLPATH)obj2bim.exe
BIN2OBJ = $(TOOLPATH)bin2obj.exe
BIM2HRB = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)zuvizudar/zuvizudar.rul
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
MAKEFONT = $(TOOLPATH)makefont.exe
GOLIB = $(TOOLPATH)golib00.exe
COPY = cp
DEL = rm -f
QEMU = /usr/bin/qemu-system-i386


default :
	$(MAKE) img


ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin:hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj: hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

hello.zuv :	hello.nas Makefile
	$(NASK) hello.nas hello.zuv hello.lst

hello2.zuv :	hello2.nas Makefile
	$(NASK) hello2.nas hello2.zuv hello2.lst

a.bim: a.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:a.bim map:a.map a.obj a_nask.obj


a.zuv: a.bim Makefile
	$(BIM2HRB) a.bim a.zuv 0

hello3.bim: hello3.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello3.bim map:hello3.map hello3.obj a_nask.obj

hello3.zuv: hello3.bim Makefile
	$(BIM2HRB) hello3.bim hello3.zuv 0

hello4.bim: hello4.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello4.bim map:hello4.map hello4.obj a_nask.obj

hello4.zuv: hello4.bim Makefile
	$(BIM2HRB) hello4.bim hello4.zuv 0

hello5.bim : hello5.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello5.bim stack:1k map:hello5.map hello5.obj

hello5.zuv : hello5.bim Makefile
	$(BIM2HRB) hello5.bim hello5.zuv 0

winhelo.bim: winhelo.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:winhelo.bim map:winhelo.map winhelo.obj a_nask.obj

winhelo.zuv: winhelo.bim Makefile
	$(BIM2HRB) winhelo.bim winhelo.zuv 0

winhelo2.bim: winhelo2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:winhelo2.bim map:winhelo2.map winhelo2.obj a_nask.obj

winhelo2.zuv: winhelo2.bim Makefile
	$(BIM2HRB) winhelo2.bim winhelo2.zuv 0

winhelo3.bim: winhelo3.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:winhelo3.bim map:winhelo3.map winhelo3.obj a_nask.obj

winhelo3.zuv: winhelo3.bim Makefile
	$(BIM2HRB) winhelo3.bim winhelo3.zuv 0


star1.bim : star1.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:star1.bim stack:1k map:star1.map \
		star1.obj a_nask.obj

star1.zuv : star1.bim Makefile
	$(BIM2HRB) star1.bim star1.zuv 47k

stars.bim : stars.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:stars.bim stack:1k map:stars.map \
		stars.obj a_nask.obj

stars.zuv : stars.bim Makefile
	$(BIM2HRB) stars.bim stars.zuv 47k

stars2.bim : stars2.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:stars2.bim stack:1k map:stars2.map \
		stars2.obj a_nask.obj

stars2.zuv : stars2.bim Makefile
	$(BIM2HRB) stars2.bim stars2.zuv 47k

lines.bim : lines.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:lines.bim stack:1k map:lines.map \
		lines.obj a_nask.obj

lines.zuv : lines.bim Makefile
	$(BIM2HRB) lines.bim lines.zuv 48k

walk.bim : walk.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:walk.bim stack:1k map:walk.map \
		walk.obj a_nask.obj

walk.zuv : walk.bim Makefile
	$(BIM2HRB) walk.bim walk.zuv 48k

noodle.bim : noodle.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:noodle.bim stack:1k map:noodle.map \
		noodle.obj a_nask.obj

noodle.zuv : noodle.bim Makefile
	$(BIM2HRB) noodle.bim noodle.zuv 40k


beepdown.bim : beepdown.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:beepdown.bim stack:1k map:beepdown.map \
		beepdown.obj a_nask.obj

beepdown.zuv : beepdown.bim Makefile
	$(BIM2HRB) beepdown.bim beepdown.zuv 40k

color.bim : color.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color.bim stack:1k map:color.map \
		color.obj a_nask.obj

color.zuv : color.bim Makefile
	$(BIM2HRB) color.bim color.zuv 56k

color2.bim : color2.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color2.bim stack:1k map:color2.map \
		color2.obj a_nask.obj

color2.zuv : color2.bim Makefile
	$(BIM2HRB) color2.bim color2.zuv 56k

zuvizuda.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > zuvizuda.sys

zuvizuda.img : ipl10.bin zuvizuda.sys hello.zuv hello2.zuv a.zuv hello3.zuv \
								hello4.zuv hello5.zuv  winhelo.zuv winhelo2.zuv\
							 winhelo3.zuv star1.zuv	stars.zuv stars2.zuv lines.zuv walk.zuv noodle.zuv\
							 beepdown.zuv color.zuv color2.zuv\
							 Makefile
	$(EDIMG)   imgin:z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
	 	copy from:zuvizuda.sys to:@: \
		copy from:ipl10.nas to:@:\
		copy from:hello.zuv to:@:\
		copy from:hello2.zuv to:@:\
		copy from:a.zuv to:@:\
		copy from:hello3.zuv to:@:\
		copy from:hello4.zuv to:@:\
		copy from:hello5.zuv to:@:\
		copy from:winhelo.zuv to:@:\
		copy from:winhelo2.zuv to:@:\
		copy from:winhelo3.zuv to:@:\
		copy from:star1.zuv to:@:\
		copy from:stars.zuv to:@:\
		copy from:stars2.zuv to:@:\
		copy from:lines.zuv to:@:\
		copy from:walk.zuv to:@:\
		copy from:noodle.zuv to:@:\
		copy from:beepdown.zuv to:@:\
		copy from:color.zuv to:@:\
		copy from:color2.zuv to:@:\
		imgout:zuvizuda.img


%.gas :%.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj: %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst


img :
	$(MAKE) zuvizuda.img

run :
	$(MAKE) img
	$(QEMU) -fda zuvizuda.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.zuv
	-$(DEL) *.bim
	-$(DEL) bootpack.nas
	-$(DEL) dsctbl.nas
	-$(DEL) graphic.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) zuvizuda.sys

src_only :
	$(MAKE) clean
	-$(DEL) zuvizuda.img
