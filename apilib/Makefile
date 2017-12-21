OBJS_API = putchar.obj putstr0.obj putstr1.obj end.obj openwin.obj putstrwin.obj\
		   boxfilwin.obj initmalloc.obj malloc.obj free.obj point.obj refreshwin.obj\
		   linewin.obj closewin.obj getkey.obj alloctimer.obj inittimer.obj settimer.obj\
		   freetimer.obj beep.obj fopen.obj fclose.obj fseek.obj fsize.obj fread.obj\
		   cmdline.obj getlang.obj fopen_w.obj fclose_w.obj fwrite.obj alloca.obj
TOOLPATH=../z_tools/
INCPATH=../z_tools/include/

MAKE     = make -r
NASK = $(TOOLPATH)nask.exe
CC1 = $(TOOLPATH)cc1.exe -I $(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM = $(TOOLPATH)obj2bim.exe
BIN2OBJ = $(TOOLPATH)bin2obj.exe
BIM2ZUV = $(TOOLPATH)bim2zuv
RULEFILE = $(TOOLPATH)zuvizudar/zuvizudar.rul
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
MAKEFONT = $(TOOLPATH)makefont.exe
GOLIB = $(TOOLPATH)golib00.exe
COPY = cp
DEL = rm -f

default :
	$(MAKE) apilib.lib

apilib.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	-$(DEL) *.lst
	-$(DEL) *.obj

src_only :
	$(MAKE)clean
	-$(DEL) apilib.lib
