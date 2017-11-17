TOOLPATH=../z_tools/
INCPATH=../z_tools/zuvizudar/

OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj timer.obj mtask.obj window.obj console.obj file.obj 

MAKE     = make -r
NASK = $(TOOLPATH)nask.exe
CC1 = $(TOOLPATH)cc1.exe -I $(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM = $(TOOLPATH)obj2bim.exe
BIN2OBJ = $(TOOLPATH)bin2obj.exe
BIM2ZUV = $(TOOLPATH)bim2zuv
RULEFILE = ../zuvizudar.rul
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
MAKEFONT = $(TOOLPATH)makefont.exe
GOLIB = $(TOOLPATH)golib00.exe
COPY = cp
DEL = rm -f
QEMU = /usr/bin/qemu-system-i386

default :
	$(MAKE) ipl20.bin
	$(MAKE) zuvizudar.sys

ipl20.bin : ipl20.nas Makefile
	$(NASK) ipl20.nas ipl20.bin ipl20.lst

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
	$(BIM2ZUV) bootpack.bim bootpack.hrb 0

zuvizudar.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > zuvizudar.sys

%.gas :%.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj: %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean:
	-$(DEL) asmhead.bin
	-$(DEL) hankaku.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.zuv
	-$(DEL) *.hrb

src_only:
	$(MAKE) clean
	-$(DEL) ipl20.bin
	-$(DEL) zuvizudar.sys
