TOOLPATH = z_tools/
INCPATH  = z_tools/include/

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

zuvizudar.img : kernel/ipl20.bin kernel/zuvizudar.sys Makefile \
		app/a/a.zuv app/hello3/hello3.zuv app/hello4/hello4.zuv app/hello5/hello5.zuv \
		app/winhelo/winhelo.zuv app/winhelo2/winhelo2.zuv app/winhelo3/winhelo3.zuv \
		app/star1/star1.zuv app/stars/stars.zuv app/stars2/stars2.zuv \
		app/lines/lines.zuv app/walk/walk.zuv app/noodle/noodle.zuv \
		app/beepdown/beepdown.zuv app/color/color.zuv app/color2/color2.zuv \
		app/sosu/sosu.zuv app/sosu2/sosu2.zuv app/typeipl/typeipl.zuv app/cat/cat.zuv app/iroha/iroha.c\
		app/zuv/zuv.zuv \
		app/chklang/chklang.zuv app/nihongo/nihongo.fnt app/sosu3/sosu3.zuv app/tview/tview.zuv

	$(EDIMG)	imgin:z_tools/fdimg0at.tek \
		wbinimg src:kernel/ipl20.bin len:512 from:0 to:0 \
		copy from:kernel/zuvizudar.sys to:@: \
		copy from:kernel/ipl20.nas to:@: \
		copy from:app/a/a.zuv to:@: \
		copy from:app/hello3/hello3.zuv to:@: \
		copy from:app/hello4/hello4.zuv to:@: \
		copy from:app/hello5/hello5.zuv to:@: \
		copy from:app/winhelo/winhelo.zuv to:@: \
		copy from:app/winhelo2/winhelo2.zuv to:@: \
		copy from:app/winhelo3/winhelo3.zuv to:@: \
		copy from:app/star1/star1.zuv to:@: \
		copy from:app/stars/stars.zuv to:@: \
		copy from:app/stars2/stars2.zuv to:@: \
		copy from:app/lines/lines.zuv to:@: \
		copy from:app/walk/walk.zuv to:@: \
		copy from:app/noodle/noodle.zuv to:@: \
		copy from:app/beepdown/beepdown.zuv to:@: \
		copy from:app/color/color.zuv to:@: \
		copy from:app/color2/color2.zuv to:@: \
		copy from:app/sosu/sosu.zuv to:@: \
		copy from:app/sosu2/sosu2.zuv to:@: \
		copy from:app/typeipl/typeipl.zuv to:@: \
		copy from:app/cat/cat.zuv to:@: \
		copy from:app/iroha/iroha.zuv to:@: \
		copy from:app/chklang/chklang.zuv to:@: \
		copy from:app/zuv/zuv.zuv to:@: \
		copy from:euc.txt to:@: \
		copy from:app/zuv/test.txt to:@: \
		copy from:app/nihongo/nihongo.fnt to:@: \
		copy from:app/sosu3/sosu3.zuv to:@: \
		copy from:app/tview/tview.zuv to:@: \
		imgout:zuvizudar.img

# コマンド

run :
	$(MAKE) zuvizudar.img
	$(COPY) zuvizudar.img z_tools\qemu\fdimage0.bin
	$(COPY) zuvizudar.img z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) zuvizudar.img
	$(IMGTOL) w a: zuvizudar.img

full :
	$(MAKE) -C kernel
	$(MAKE) -C apilib
	$(MAKE) -C app/a
	$(MAKE) -C app/hello3
	$(MAKE) -C app/hello4
	$(MAKE) -C app/hello5
	$(MAKE) -C app/winhelo
	$(MAKE) -C app/winhelo2
	$(MAKE) -C app/winhelo3
	$(MAKE) -C app/star1
	$(MAKE) -C app/stars
	$(MAKE) -C app/stars2
	$(MAKE) -C app/lines
	$(MAKE) -C app/walk
	$(MAKE) -C app/noodle
	$(MAKE) -C app/beepdown
	$(MAKE) -C app/color
	$(MAKE) -C app/color2
	$(MAKE) -C app/sosu
	$(MAKE) -C app/sosu2
	$(MAKE) -C app/typeipl
	$(MAKE) -C app/cat
	$(MAKE) -C app/iroha
	$(MAKE) -C app/chklang
	$(MAKE) -C app/zuv
	$(MAKE) -C app/sosu3
	$(MAKE) -C app/tview
	$(MAKE) zuvizudar.img

run_full :
	$(MAKE) full
	$(QEMU) -fda zuvizudar.img

install_full :
	$(MAKE) full
	$(IMGTOL) w a: zuvizudar.img

run_os :
	$(MAKE) -C kernel
	$(MAKE) run

clean :
# 何もしない

src_only :
	$(MAKE) clean
	-$(DEL) zuvizudar.img

clean_full :
	$(MAKE) -C kernel 		clean
	$(MAKE) -C apilib		clean
	$(MAKE) -C app/a			clean
	$(MAKE) -C app/hello3		clean
	$(MAKE) -C app/hello4		clean
	$(MAKE) -C app/hello5		clean
	$(MAKE) -C app/winhelo		clean
	$(MAKE) -C app/winhelo2		clean
	$(MAKE) -C app/winhelo3		clean
	$(MAKE) -C app/star1		clean
	$(MAKE) -C app/stars		clean
	$(MAKE) -C app/stars2		clean
	$(MAKE) -C app/lines		clean
	$(MAKE) -C app/walk			clean
	$(MAKE) -C app/noodle		clean
	$(MAKE) -C app/beepdown		clean
	$(MAKE) -C app/color		clean
	$(MAKE) -C app/color2		clean
	$(MAKE) -C app/sosu			clean
	$(MAKE) -C app/sosu2		clean
	$(MAKE) -C app/typeipl		clean
	$(MAKE) -C app/cat 			clean
	$(MAKE) -C app/iroha 		clean
	$(MAKE) -C app/chklang 		clean
	$(MAKE) -C app/zuv  		clean
	$(MAKE) -C app/tview  		clean

src_only_full :
	$(MAKE) -C kernel		src_only
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
