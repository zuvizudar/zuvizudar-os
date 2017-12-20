[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "crack7.nas"]
		GLOBAL	_ZuviMain
[SECTION .text]

_ZuviMain:
		MOV		AX,1005*8
		MOV		DS,AX
		CMP		DWORD [DS:0x0004],'Zuvi'
		JNE		fin					; アプリではないようなので何もしない
		MOV		ECX,[DS:0x0000]		; このアプリのデータセグメントの大きさを読み取る
		MOV		AX,2005*8
		MOV		DS,AX

crackloop:							; 123で埋め尽くす
		ADD		ECX,-1
		MOV		BYTE [DS:ECX],123
		CMP		ECX,0
		JNE		crackloop

fin:								; 終了
		MOV		EDX,4
		INT		0x40
