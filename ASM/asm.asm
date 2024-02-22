.586
.model flat, stdcall
includelib libucrt.lib
includelib kernel32.lib
includelib "../Debug/GenLib.lib
ExitProcess PROTO:DWORD 
.stack 4096


 outnum PROTO : DWORD

 outstr PROTO : DWORD

 strcopy PROTO : DWORD, : DWORD

 lenght PROTO : DWORD, : DWORD

 atoii  PROTO : DWORD,  : DWORD

.const
		newline byte 13, 10, 0
		LTRL1 sdword 5
		LTRL2 sdword 20
		LTRL3 sdword 10
.data
		temp sdword ?
		buffer byte 256 dup(0)
		maina sdword 0
		mainb sdword 0
		mainc sdword 0
.code

;------------- MAIN --------------
main PROC
push LTRL1

pop ebx
mov maina, ebx

push maina
push LTRL2
pop ebx
pop eax
add eax, ebx
push eax

pop ebx
mov maina, ebx

push LTRL2

pop ebx
mov mainb, ebx

push maina
push mainc
pop ebx
pop eax
imul eax, ebx
push eax
push LTRL3
push mainb
pop ebx
pop eax
imul eax, ebx
push eax
pop ebx
pop eax
add eax, ebx
push eax

pop ebx
mov mainc, ebx


push mainc
call outnum

push offset newline
call outstr

push 0
call ExitProcess
main ENDP
end main
