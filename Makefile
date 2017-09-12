TOOLPATH=z_tools/
INCPATH=z_tools/zuvizudar/

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

bootpack.bim : bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

zuvizudar.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > zuvizudar.sys

zuvizudar.img : ipl10.bin zuvizudar.sys Makefile
	$(EDIMG)   imgin:z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
	 	copy from:zuvizudar.sys to:@: \
		imgout:zuvizudar.img


%.gas :%.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj: %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst


img :
	$(MAKE) zuvizudar.img

run :
	$(MAKE) img
	$(QEMU) -fda zuvizudar.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) dsctbl.nas
	-$(DEL) graphic.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) zuvizudar.sys

src_only :
	$(MAKE) clean
	-$(DEL) zuvizudar.img
