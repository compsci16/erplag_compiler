SECTION .data
int_message db 'Input: Enter an integer value:',0
int_input_format db '%hd', 0 ; for short as integer is of 2 bytes in erplag
int_output_format_same_line db '%hd ', 0
new_line_character_string db  10, 0
int_output_format db 'You entered: %hd', 10, 0
array_index_out_of_bounds_message db 'RUN TIME ERROR: Array Index Out of Bound', 10, 0
simple_int_output_format db '%hd', 10, 0
simple_bool_output_format db '%s', 0
simple_string_output_format db '%s', 0
simple_real_output_format db '%f', 10, 0
bool_message db 'Input: Enter a boolean value, where 0 is treated as false and anything else as true:',0
bool_input_format db '%hhd', 0 ; for boolean
bool_output_format db 'You entered: %hhd', 10, 0
bool_true db 'true',10, 0
bool_false db 'false',10, 0
array_message resb 64 ; not a constant
global  _main
extern _puts
extern _scanf
extern _printf
extern _malloc
SECTION .text
_main:
	call DRIVER
	exit:
	mov rax, 0x02000001         ; system call for exit; CHANGE IN LINUX
	xor rdi, rdi; exit code 0
	syscall ;invoke OS to exit

	exit_array_error:
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel simple_string_output_format]
	lea rsi, [rel array_index_out_of_bounds_message]
	call _printf
	;restore stack
	mov rsp, rbx
	mov rax, 0x02000001         ; system call for exit; CHANGE IN LINUX
	xor rdi, rdi; exit code 0
	syscall ;invoke OS to exit

;first argument in rax
_print_message:
	push rdi 
	lea  rdi, [rax]    ; First argument is address of message
	call      _puts                   ; puts(message)
	pop rdi
	ret
;1st argument: integer to print in rax
;printf("%d", i); 
_print_integer_id:
	mov r12, rsp; 

	and rsp, -16
	lea rdi, [rel simple_int_output_format]; printf first arg %hd
	mov rsi, rax ; printf second arg 
	call _printf

	;epilog
	mov rsp, r12
	ret
; assumes the boolean value is in rax ; 0 = false, anything else true
_print_boolean_id:
	test rax, rax
	jz printFalse
	lea rdi, [rel simple_bool_output_format]
	lea rsi, [rel bool_true]
	call _printf
	ret
	printFalse: 
		lea rdi, [rel simple_bool_output_format]
		lea rsi, [rel bool_false]
		call _printf
		ret
DRIVER:
	;allocate space for module
	push rbp
	mov rbp, rsp
	sub rsp, 6
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 1
	mov word[rbp - 8], r8w
	mov word[rbp - 2], 1
	mov byte[rbp - 3], 0
	; module use or function call
	; push caller saved registers
	push rbp
	push r10
	push r11
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	mov di, 1
	call mod1
	; pop caller saved registers
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop r11
	pop r10
	pop rbp
	; store received values from callee available in desired locations
	mov [rbp - 5], ax
	mov [rbp - 6], r10b
	;handle upto 6 parameters first
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 5]
	call _print_integer_id
	mov rsp, rbx
	;place return values in desired registers
	;deallocate space from module
	add rsp, 8
	pop rbp
	ret
mod1:
	;allocate space for module
	push rbp
	mov rbp, rsp
	sub rsp, 7
	mov word[rbp - 2], di
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 2
	mov word[rbp - 9], r8w
	mov word[rbp - 7], 2
	; module use or function call
	; push caller saved registers
	push rbp
	push r10
	push r11
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	mov di, word[rbp - 7]
	call scanInt
	; pop caller saved registers
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop r11
	pop r10
	pop rbp
	; store received values from callee available in desired locations
	mov [rbp - 4], ax
	;handle upto 6 parameters first
	mov byte[rbp - 5], 0
	;place return values in desired registers
	mov ax, word[rbp - 4]
	mov r10b, byte[rbp - 5]
	;deallocate space from module
	add rsp, 9
	pop rbp
	ret
scanInt:
	;allocate space for module
	push rbp
	mov rbp, rsp
	sub rsp, 4
	mov word[rbp - 2], di
;getvalue integer
	mov rbx, rsp
	and rsp, -16; align to 16 byte boundary
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	mov rsp, rbx; fix up the stack after each function call

	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 4]; get address of integer to store for i;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 4];
	call _printf
	mov rsp, rbx ; fix stack

	;place return values in desired registers
	mov ax, word[rbp - 4]
	;deallocate space from module
	add rsp, 4
	pop rbp
	ret
