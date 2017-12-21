[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api029.nas"]

		GLOBAL	_api_fclose_w

[SECTION .text]

_api_fclose_w:		; void api_fclose_w(int fhandle);
		MOV		EDX,29
		MOV		EAX,[ESP+4]	
		INT		0x40
		RET
