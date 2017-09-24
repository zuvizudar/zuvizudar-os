[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

	GLOBAL _api_putchar
	GLOBAL _api_end
	GLOBAL _api_putstr0
	GLOBAL _api_openwin
	GLOBAL _api_putstrwin
	GLOBAL _api_boxfilwin
	GLOBAL _api_initmalloc
	GLOBAL _api_malloc
	GLOBAL _api_free
	GLOBAL _api_point
	GLOBAL _api_refreshwin
	GLOBAL _api_linewin,_api_closewin,_api_getkey
	GLOBAL _api_alloctimer,_api_inittimer,_api_settimer,_api_freetimer

[SECTION .text]
_api_putchar:
	MOV EDX,1
	MOV AL,[ESP+4]
	INT 0x40
	RET

_api_end:
	MOV EDX,4
	INT 0x40

_api_putstr0:
	PUSH EBX
	MOV EDX,2
	MOV EBX,[ESP+8]
	INT 0x40
	POP EBX
	RET

_api_openwin:
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX,5
	MOV EBX,[ESP+16]
	MOV ESI,[ESP+20]
	MOV EDI,[ESP+24]
	MOV EAX,[ESP+28]
	MOV ECX,[ESP+32]
	INT 0x40
	POP EBX
	POP ESI
	POP EDI
	RET

_api_putstrwin:	; void api_putstrwin(int win, int x, int y, int col, int len, char *str);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,6
	MOV		EBX,[ESP+20]
	MOV		ESI,[ESP+24]
	MOV		EDI,[ESP+28]
	MOV		EAX,[ESP+32]
	MOV		ECX,[ESP+36]
	MOV		EBP,[ESP+40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_boxfilwin:	; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,7
	MOV		EBX,[ESP+20]	; win
	MOV		EAX,[ESP+24]	; x0
	MOV		ECX,[ESP+28]	; y0
	MOV		ESI,[ESP+32]	; x1
	MOV		EDI,[ESP+36]	; y1
	MOV		EBP,[ESP+40]	; col
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_initmalloc:	; void api_initmalloc(void);
	PUSH	EBX
	MOV		EDX,8
	MOV		EBX,[CS:0x0020]
	MOV		EAX,EBX
	ADD		EAX,32*1024
	MOV		ECX,[CS:0x0000]
	SUB		ECX,EAX
	INT		0x40
	POP		EBX
	RET

_api_malloc:		; char *api_malloc(int size);
	PUSH	EBX
	MOV		EDX,9
	MOV		EBX,[CS:0x0020]
	MOV		ECX,[ESP+8]
	INT		0x40
	POP		EBX
	RET

_api_free:			; void api_free(char *addr, int size);
	PUSH	EBX
	MOV		EDX,10
	MOV		EBX,[CS:0x0020]
	MOV		EAX,[ESP+ 8]
	MOV		ECX,[ESP+12]
	INT		0x40
	POP		EBX
	RET

_api_point:
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,11
	MOV		EBX,[ESP+16]
	MOV		ESI,[ESP+20]
	MOV		EDI,[ESP+24]
	MOV		EAX,[ESP+28]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_refreshwin:
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,12
	MOV		EBX,[ESP+16]
	MOV		EAX,[ESP+20]
	MOV		ECX,[ESP+24]
	MOV		ESI,[ESP+28]
	MOV		EDI,[ESP+32]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_linewin:
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,13
	MOV		EBX,[ESP+20]
	MOV		EAX,[ESP+24]
	MOV		ECX,[ESP+28]
	MOV		ESI,[ESP+32]
	MOV		EDI,[ESP+36]
	MOV		EBP,[ESP+40]
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_closewin:
	PUSH	EBX
	MOV		EDX,14
	MOV		EBX,[ESP+8]
	INT		0x40
	POP		EBX
	RET

_api_getkey:
	MOV		EDX,15
	MOV		EAX,[ESP+4]
	INT		0x40
	RET

_api_alloctimer:
		MOV		EDX,16
		INT		0x40
		RET

_api_inittimer:
		PUSH	EBX
		MOV		EDX,17
		MOV		EBX,[ESP+ 8]
		MOV		EAX,[ESP+12]
		INT		0x40
		POP		EBX
		RET

_api_settimer:
		PUSH	EBX
		MOV		EDX,18
		MOV		EBX,[ESP+ 8]
		MOV		EAX,[ESP+12]
		INT		0x40
		POP		EBX
		RET

_api_freetimer:
		PUSH	EBX
		MOV		EDX,19
		MOV		EBX,[ESP+ 8]
		INT		0x40
		POP		EBX
		RET
