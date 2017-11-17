TOOLPATH = z_tools/
INCPATH  = z_tools/zuvizudar/

MAKE     = make -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = cp
DEL      = rm -f
QEMU = /usr/bin/qemu-system-i386

# デフォルト動作

default :
	$(MAKE) zuvizudar.img

# ファイル生成規則

zuvizudar.img : zuvizudar/ipl20.bin zuvizudar/zuvizudar.sys Makefile \
		a/a.zuv hello3/hello3.zuv hello4/hello4.zuv hello5/hello5.zuv \
		winhelo/winhelo.zuv winhelo2/winhelo2.zuv winhelo3/winhelo3.zuv \
		star1/star1.zuv stars/stars.zuv stars2/stars2.zuv \
		lines/lines.zuv walk/walk.zuv noodle/noodle.zuv \
		beepdown/beepdown.zuv color/color.zuv color2/color2.zuv \
		sosu/sosu.zuv sosu2/sosu2.zuv typeipl/typeipl.zuv cat/cat.zuv iroha/iroha.c\
		chklang/chklang.zuv nihongo/nihongo.fnt 

	$(EDIMG)   imgin:z_tools/fdimg0at.tek \
		wbinimg src:zuvizudar/ipl20.bin len:512 from:0 to:0 \
		copy from:zuvizudar/zuvizudar.sys to:@: \
		copy from:zuvizudar/ipl20.nas to:@: \
		copy from:a/a.zuv to:@: \
		copy from:hello3/hello3.zuv to:@: \
		copy from:hello4/hello4.zuv to:@: \
		copy from:hello5/hello5.zuv to:@: \
		copy from:winhelo/winhelo.zuv to:@: \
		copy from:winhelo2/winhelo2.zuv to:@: \
		copy from:winhelo3/winhelo3.zuv to:@: \
		copy from:star1/star1.zuv to:@: \
		copy from:stars/stars.zuv to:@: \
		copy from:stars2/stars2.zuv to:@: \
		copy from:lines/lines.zuv to:@: \
		copy from:walk/walk.zuv to:@: \
		copy from:noodle/noodle.zuv to:@: \
		copy from:beepdown/beepdown.zuv to:@: \
		copy from:color/color.zuv to:@: \
		copy from:color2/color2.zuv to:@: \
		copy from:sosu/sosu.zuv to:@: \
		copy from:sosu2/sosu2.zuv to:@: \
		copy from:typeipl/typeipl.zuv to:@: \
		copy from:cat/cat.zuv to:@: \
		copy from:iroha/iroha.zuv to:@: \
		copy from:chklang/chklang.zuv to:@: \
		copy from:euc.txt to:@: \
		copy from:nihongo/nihongo.fnt to:@: \
		imgout:zuvizudar.img

# コマンド

run :
	$(MAKE) zuvizudar.img
	$(COPY) zuvizudar.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) zuvizudar.img
	$(IMGTOL) w a: zuvizudar.img

full :
	$(MAKE) -C zuvizudar
	$(MAKE) -C apilib
	$(MAKE) -C a
	$(MAKE) -C hello3
	$(MAKE) -C hello4
	$(MAKE) -C hello5
	$(MAKE) -C winhelo
	$(MAKE) -C winhelo2
	$(MAKE) -C winhelo3
	$(MAKE) -C star1
	$(MAKE) -C stars
	$(MAKE) -C stars2
	$(MAKE) -C lines
	$(MAKE) -C walk
	$(MAKE) -C noodle
	$(MAKE) -C beepdown
	$(MAKE) -C color
	$(MAKE) -C color2
	$(MAKE) -C sosu
	$(MAKE) -C sosu2
	$(MAKE) -C typeipl
	$(MAKE) -C cat
	$(MAKE) -C iroha
	$(MAKE) -C chklang
	$(MAKE) zuvizudar.img

run_full :
	$(MAKE) full
	$(QEMU) -fda zuvizudar.img

install_full :
	$(MAKE) full
	$(IMGTOL) w a: zuvizudar.img

run_os :
	$(MAKE) -C zuvizudar
	$(MAKE) run

clean :
# 何もしない

src_only :
	$(MAKE) clean
	-$(DEL) zuvizudar.img

clean_full :
	$(MAKE) -C zuvizudar	clean
	$(MAKE) -C apilib		clean
	$(MAKE) -C a			clean
	$(MAKE) -C hello3		clean
	$(MAKE) -C hello4		clean
	$(MAKE) -C hello5		clean
	$(MAKE) -C winhelo		clean
	$(MAKE) -C winhelo2		clean
	$(MAKE) -C winhelo3		clean
	$(MAKE) -C star1		clean
	$(MAKE) -C stars		clean
	$(MAKE) -C stars2		clean
	$(MAKE) -C lines		clean
	$(MAKE) -C walk			clean
	$(MAKE) -C noodle		clean
	$(MAKE) -C beepdown		clean
	$(MAKE) -C color		clean
	$(MAKE) -C color2		clean
	$(MAKE) -C sosu			clean
	$(MAKE) -C sosu2		clean
	$(MAKE) -C typeipl		clean
	$(MAKE) -C cat 			clean
	$(MAKE) -C iroha 		clean
	$(MAKE) -C chklang 		clean

src_only_full :
	$(MAKE) -C zuvizudar	src_only
	$(MAKE) -C apilib		src_only
	$(MAKE) -C a			src_only
	$(MAKE) -C hello3		src_only
	$(MAKE) -C hello4		src_only
	$(MAKE) -C hello5		src_only
	$(MAKE) -C winhelo		src_only
	$(MAKE) -C winhelo2		src_only
	$(MAKE) -C winhelo3		src_only
	$(MAKE) -C star1		src_only
	$(MAKE) -C stars		src_only
	$(MAKE) -C stars2		src_only
	$(MAKE) -C lines		src_only
	$(MAKE) -C walk			src_only
	$(MAKE) -C noodle		src_only
	$(MAKE) -C beepdown		src_only
	$(MAKE) -C color		src_only
	$(MAKE) -C color2		src_only
	$(MAKE) -C sosu			src_only
	$(MAKE) -C sosu2		src_only
	$(MAKE) -C typeipl		src_only
	$(MAKE) -C cat			src_only
	$(MAKE) -C chklang		src_only
	-$(DEL) zuvizudar.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) zuvizudar.img
