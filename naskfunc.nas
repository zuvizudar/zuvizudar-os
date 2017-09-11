; naskfunc
; TAB=4

[FORMAT "WCOFF"];
[INSTRSET "i486p"];
[BITS 32];
[FILE "naskfunc.nas"]			

	GLOBAL _io_hlt,_write_mem8,_io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL _load_gdtr,_load_idtr
	GLOBAL _load_cr0,_store_cr0
	GLOBAL _asm_inthandler21,_asm_inthandler2c
	GLOBAL _memtest_sub
	EXTERN _inthandler21,_inthandler2c

[SECTION .text]	

_io_hlt:	
	HLT
	RET

_write_mem8:	; write_mem8(int addr, int data)
	MOV     ECX,[ESP+4]
	MOV     AL,[ESP+8]
	MOV     [ECX],AL
	RET

_io_cli:
	CLT
	RET

_io_sti:
	STI 
	RET

_io_stihlt:
	STI 
	HLT
	RET

_io_in8:
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN AL,DX
	RET

_io_in16:
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN AX,DX
	RET

_io_in32:
	MOV EDX,[ESP+4]
	IN EAX,DX

_io_out8:
	MOV EDX,[ESP+4]
	MOV AL,[ESP+8]
	OUT DX,AL
	RET

_io_out16:
	MOV EDX,[ESP+4]
	MOV EAX,[ESP+8]
	OUT DX,AX
	RET

_io_out32:
	MOV EDX,[ESP+4]
	MOV EAX,[ESP+8]
	OUT DX,EAX
	RET

_io_load_eflags:
	PUSHFD
	POP EAX
	RET

_io_store_eflags:
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD
	RET

_load_gdtr:
	MOV AX,[ESP+4]
	MOV [ESP+6],AX
	LGDT [ESP+6]
	RET

_load_idtr:
	MOV AX,[ESP+4]
	MOV [ESP+6],AX
	LIDT [ESP+6]
	RET

_load_cr0:
	MOV	EAX,CR0
	RET

_store_cr0:
	MOV EAX,[ESP+4]
	MOV CR0,EAX
	RET
	
_asm_inthandler21:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler21
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

_asm_inthandler2c:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler2c
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;

mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET

mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET
