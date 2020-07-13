
.code

_switch proc
	mov qword ptr[rcx + 00h], rsp
	mov qword ptr[rcx + 08h], r15
	mov qword ptr[rcx + 10h], r14
	mov qword ptr[rcx + 18h], r13
	mov qword ptr[rcx + 20h], r12
	mov qword ptr[rcx + 28h], rbx
	mov qword ptr[rcx + 30h], rbp

	mov rsp, qword ptr[rdx + 00h]
	mov r15, qword ptr[rdx + 08h]
	mov r14, qword ptr[rdx + 10h]
	mov r13, qword ptr[rdx + 18h]
	mov r12, qword ptr[rdx + 20h]
	mov rbx, qword ptr[rdx + 28h]
	mov rbp, qword ptr[rdx + 30h]
	ret

_switch endp

end
