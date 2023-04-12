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
	sub rsp, 12
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 5
	mov word[rbp - 14], r8w
	mov word[rbp - 8], 5
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 9
	mov word[rbp - 16], r8w
	mov word[rbp - 10], 9
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
	lea rsi, [rbp - 2]; get address of integer to store for x;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 2];
	call _printf
	mov rsp, rbx ; fix stack

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
	lea rsi, [rbp - 4]; get address of integer to store for y;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 4];
	call _printf
	mov rsp, rbx ; fix stack

	movsx r8, word[rbp - 4] ; load left for plus
	movsx r9, word[rbp - 10] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 18], r9w
	movsx r8, word[rbp - 2] ; load left for plus
	movsx r9, word[rbp - 18] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 20], r9w
	movsx r8, word[rbp - 8] ; load left for plus
	movsx r9, word[rbp - 10] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 22], r9w
	movsx r8, word[rbp - 22] ; load left for plus
	movsx r9, word[rbp - 4] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 24], r9w
	movsx r8, word[rbp - 20] ; load left for plus
	movsx r9, word[rbp - 24] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 26], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 2
	mov word[rbp - 28], r8w
	movsx r8, word[rbp - 8] ; load left for plus
	mov r9, 2
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 30], r9w
	movsx r8, word[rbp - 26] ; load left for plus
	movsx r9, word[rbp - 30] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 32], r9w
	movsx r8, word[rbp - 10] ; load left for plus
	movsx r9, word[rbp - 2] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 34], r9w
	movsx r8, word[rbp - 32] ; load left for plus
	movsx r9, word[rbp - 34] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 36], r9w
	movsx r8, word[rbp - 36]
	mov word[rbp - 6], r8w
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 6]
	call _print_integer_id
	mov rsp, rbx
	;deallocate space from module
	add rsp, 36
	pop rbp
	ret