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
	sub rsp, 31
	mov byte[rbp - 17], 1
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
	lea rsi, [rbp - 10]; get address of integer to store for a;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 10];
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
	lea rsi, [rbp - 12]; get address of integer to store for b;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 12];
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
	lea rsi, [rbp - 16]; get address of integer to store for d;
	call _scanf
	mov rsp, rbx; restore rsp

	mov rbx, rsp ; save rsp
	and rsp, -16; align rsp to 16 bit boundary
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 16];
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

	;get value array
	mov byte[rel array_message + 0], 'I' ;
	mov byte[rel array_message + 1], 'n' ;
	mov byte[rel array_message + 2], 'p' ;
	mov byte[rel array_message + 3], 'u' ;
	mov byte[rel array_message + 4], 't' ;
	mov byte[rel array_message + 5], ':' ;
	mov byte[rel array_message + 6], ' ' ;
	mov byte[rel array_message + 7], 'E' ;
	mov byte[rel array_message + 8], 'n' ;
	mov byte[rel array_message + 9], 't' ;
	mov byte[rel array_message + 10], 'e' ;
	mov byte[rel array_message + 11], 'r' ;
	mov byte[rel array_message + 12], ' ' ;
	mov byte[rel array_message + 13], '6' ;
	mov byte[rel array_message + 14], ' ' ;
	mov byte[rel array_message + 15], 'a' ;
	mov byte[rel array_message + 16], 'r' ;
	mov byte[rel array_message + 17], 'r' ;
	mov byte[rel array_message + 18], 'a' ;
	mov byte[rel array_message + 19], 'y' ;
	mov byte[rel array_message + 20], ' ' ;
	mov byte[rel array_message + 21], 'e' ;
	mov byte[rel array_message + 22], 'l' ;
	mov byte[rel array_message + 23], 'e' ;
	mov byte[rel array_message + 24], 'm' ;
	mov byte[rel array_message + 25], 'e' ;
	mov byte[rel array_message + 26], 'n' ;
	mov byte[rel array_message + 27], 't' ;
	mov byte[rel array_message + 28], 's' ;
	mov byte[rel array_message + 29], ' ' ;
	mov byte[rel array_message + 30], 'o' ;
	mov byte[rel array_message + 31], 'f' ;
	mov byte[rel array_message + 32], ' ' ;
	mov byte[rel array_message + 33], 'i' ;
	mov byte[rel array_message + 34], 'n' ;
	mov byte[rel array_message + 35], 't' ;
	mov byte[rel array_message + 36], 'e' ;
	mov byte[rel array_message + 37], 'g' ;
	mov byte[rel array_message + 38], 'e' ;
	mov byte[rel array_message + 39], 'r' ;
	mov byte[rel array_message + 40], ' ' ;
	mov byte[rel array_message + 41], 't' ;
	mov byte[rel array_message + 42], 'y' ;
	mov byte[rel array_message + 43], 'p' ;
	mov byte[rel array_message + 44], 'e' ;
	mov byte[rel array_message + 45], ' ' ;
	mov byte[rel array_message + 46], 'f' ;
	mov byte[rel array_message + 47], 'o' ;
	mov byte[rel array_message + 48], 'r' ;
	mov byte[rel array_message + 49], ' ' ;
	mov byte[rel array_message + 50], 'r' ;
	mov byte[rel array_message + 51], 'a' ;
	mov byte[rel array_message + 52], 'n' ;
	mov byte[rel array_message + 53], 'g' ;
	mov byte[rel array_message + 54], 'e' ;
	mov byte[rel array_message + 55], ' ' ;
	mov byte[rel array_message + 56], '1' ;
	mov byte[rel array_message + 57], '0' ;
	mov byte[rel array_message + 58], ' ' ;
	mov byte[rel array_message + 59], 't' ;
	mov byte[rel array_message + 60], 'o' ;
	mov byte[rel array_message + 61], ' ' ;
	mov byte[rel array_message + 62], '1' ;
	mov byte[rel array_message + 63], '5' ;
	mov byte[rel array_message + 64], 0 ;
	lea rax, [rel array_message]
	call _print_message ; first argument is rax
	; get value array; base at rbp - 19
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 21]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 21];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 23]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 23];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 25]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 25];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 27]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 27];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 29]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 29];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 31]; get address of integer to store for A;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 31];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 21];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 23];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 25];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 27];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 29];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 31];
	call _printf
	;restore stack
	mov rsp, rbx
	lea rdi, [rel new_line_character_string];
	call _printf
	;get value array
	mov byte[rel array_message + 0], 'I' ;
	mov byte[rel array_message + 1], 'n' ;
	mov byte[rel array_message + 2], 'p' ;
	mov byte[rel array_message + 3], 'u' ;
	mov byte[rel array_message + 4], 't' ;
	mov byte[rel array_message + 5], ':' ;
	mov byte[rel array_message + 6], ' ' ;
	mov byte[rel array_message + 7], 'E' ;
	mov byte[rel array_message + 8], 'n' ;
	mov byte[rel array_message + 9], 't' ;
	mov byte[rel array_message + 10], 'e' ;
	mov byte[rel array_message + 11], 'r' ;
	mov byte[rel array_message + 12], ' ' ;
	mov byte[rel array_message + 13], '4' ;
	mov byte[rel array_message + 14], ' ' ;
	mov byte[rel array_message + 15], 'a' ;
	mov byte[rel array_message + 16], 'r' ;
	mov byte[rel array_message + 17], 'r' ;
	mov byte[rel array_message + 18], 'a' ;
	mov byte[rel array_message + 19], 'y' ;
	mov byte[rel array_message + 20], ' ' ;
	mov byte[rel array_message + 21], 'e' ;
	mov byte[rel array_message + 22], 'l' ;
	mov byte[rel array_message + 23], 'e' ;
	mov byte[rel array_message + 24], 'm' ;
	mov byte[rel array_message + 25], 'e' ;
	mov byte[rel array_message + 26], 'n' ;
	mov byte[rel array_message + 27], 't' ;
	mov byte[rel array_message + 28], 's' ;
	mov byte[rel array_message + 29], ' ' ;
	mov byte[rel array_message + 30], 'o' ;
	mov byte[rel array_message + 31], 'f' ;
	mov byte[rel array_message + 32], ' ' ;
	mov byte[rel array_message + 33], 'i' ;
	mov byte[rel array_message + 34], 'n' ;
	mov byte[rel array_message + 35], 't' ;
	mov byte[rel array_message + 36], 'e' ;
	mov byte[rel array_message + 37], 'g' ;
	mov byte[rel array_message + 38], 'e' ;
	mov byte[rel array_message + 39], 'r' ;
	mov byte[rel array_message + 40], ' ' ;
	mov byte[rel array_message + 41], 't' ;
	mov byte[rel array_message + 42], 'y' ;
	mov byte[rel array_message + 43], 'p' ;
	mov byte[rel array_message + 44], 'e' ;
	mov byte[rel array_message + 45], ' ' ;
	mov byte[rel array_message + 46], 'f' ;
	mov byte[rel array_message + 47], 'o' ;
	mov byte[rel array_message + 48], 'r' ;
	mov byte[rel array_message + 49], ' ' ;
	mov byte[rel array_message + 50], 'r' ;
	mov byte[rel array_message + 51], 'a' ;
	mov byte[rel array_message + 52], 'n' ;
	mov byte[rel array_message + 53], 'g' ;
	mov byte[rel array_message + 54], 'e' ;
	mov byte[rel array_message + 55], ' ' ;
	mov byte[rel array_message + 56], '-' ;
	mov byte[rel array_message + 57], '3' ;
	mov byte[rel array_message + 58], ' ' ;
	mov byte[rel array_message + 59], 't' ;
	mov byte[rel array_message + 60], 'o' ;
	mov byte[rel array_message + 61], ' ' ;
	mov byte[rel array_message + 62], '0' ;
	mov byte[rel array_message + 63], 0 ;
	lea rax, [rel array_message]
	call _print_message ; first argument is rax
	; get value array; base at rbp - 32
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 34]; get address of integer to store for B;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 34];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 36]; get address of integer to store for B;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 36];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 38]; get address of integer to store for B;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 38];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 40]; get address of integer to store for B;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 40];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 34];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 36];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 38];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 40];
	call _printf
	;restore stack
	mov rsp, rbx
	lea rdi, [rel new_line_character_string];
	call _printf
	;get value array
	mov byte[rel array_message + 0], 'I' ;
	mov byte[rel array_message + 1], 'n' ;
	mov byte[rel array_message + 2], 'p' ;
	mov byte[rel array_message + 3], 'u' ;
	mov byte[rel array_message + 4], 't' ;
	mov byte[rel array_message + 5], ':' ;
	mov byte[rel array_message + 6], ' ' ;
	mov byte[rel array_message + 7], 'E' ;
	mov byte[rel array_message + 8], 'n' ;
	mov byte[rel array_message + 9], 't' ;
	mov byte[rel array_message + 10], 'e' ;
	mov byte[rel array_message + 11], 'r' ;
	mov byte[rel array_message + 12], ' ' ;
	mov byte[rel array_message + 13], '1' ;
	mov byte[rel array_message + 14], ' ' ;
	mov byte[rel array_message + 15], 'a' ;
	mov byte[rel array_message + 16], 'r' ;
	mov byte[rel array_message + 17], 'r' ;
	mov byte[rel array_message + 18], 'a' ;
	mov byte[rel array_message + 19], 'y' ;
	mov byte[rel array_message + 20], ' ' ;
	mov byte[rel array_message + 21], 'e' ;
	mov byte[rel array_message + 22], 'l' ;
	mov byte[rel array_message + 23], 'e' ;
	mov byte[rel array_message + 24], 'm' ;
	mov byte[rel array_message + 25], 'e' ;
	mov byte[rel array_message + 26], 'n' ;
	mov byte[rel array_message + 27], 't' ;
	mov byte[rel array_message + 28], 's' ;
	mov byte[rel array_message + 29], ' ' ;
	mov byte[rel array_message + 30], 'o' ;
	mov byte[rel array_message + 31], 'f' ;
	mov byte[rel array_message + 32], ' ' ;
	mov byte[rel array_message + 33], 'i' ;
	mov byte[rel array_message + 34], 'n' ;
	mov byte[rel array_message + 35], 't' ;
	mov byte[rel array_message + 36], 'e' ;
	mov byte[rel array_message + 37], 'g' ;
	mov byte[rel array_message + 38], 'e' ;
	mov byte[rel array_message + 39], 'r' ;
	mov byte[rel array_message + 40], ' ' ;
	mov byte[rel array_message + 41], 't' ;
	mov byte[rel array_message + 42], 'y' ;
	mov byte[rel array_message + 43], 'p' ;
	mov byte[rel array_message + 44], 'e' ;
	mov byte[rel array_message + 45], ' ' ;
	mov byte[rel array_message + 46], 'f' ;
	mov byte[rel array_message + 47], 'o' ;
	mov byte[rel array_message + 48], 'r' ;
	mov byte[rel array_message + 49], ' ' ;
	mov byte[rel array_message + 50], 'r' ;
	mov byte[rel array_message + 51], 'a' ;
	mov byte[rel array_message + 52], 'n' ;
	mov byte[rel array_message + 53], 'g' ;
	mov byte[rel array_message + 54], 'e' ;
	mov byte[rel array_message + 55], ' ' ;
	mov byte[rel array_message + 56], '0' ;
	mov byte[rel array_message + 57], ' ' ;
	mov byte[rel array_message + 58], 't' ;
	mov byte[rel array_message + 59], 'o' ;
	mov byte[rel array_message + 60], ' ' ;
	mov byte[rel array_message + 61], '0' ;
	mov byte[rel array_message + 62], 0 ;
	lea rax, [rel array_message]
	call _print_message ; first argument is rax
	; get value array; base at rbp - 33
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rax, [rel int_message] ;load address of int_message to rax
	call _print_message ; first argument is rax
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	;scanf("%hd", &i) so rdi gets first arg then rsi gets second arg
	lea rdi, [rel int_input_format]
	lea rsi, [rbp - 35]; get address of integer to store for C;
	call _scanf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format];
	movzx rsi, word [rbp - 35];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 35];
	call _printf
	;restore stack
	mov rsp, rbx
	lea rdi, [rel new_line_character_string];
	call _printf
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 5
	mov word[rbp - 35], r8w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 2
	mov word[rbp - 37], r8w
	mov r8, 5
	mov r9, 2
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 39], r9w
	movsx r8, word[rbp - 39] ; load left for plus
	movsx r9, word[rbp - 10] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 41], r9w
	movsx r8, word[rbp - 41]; store i of a[i] in r8
	cmp r8, 10
	jl exit_array_error
	cmp r8, 15
	jg exit_array_error
	sub r8,10
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 19
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 43], r11w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 12
	mov word[rbp - 45], r8w
	mov r8, 12
	movsx r9, word[rbp - 12] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 47], r9w
	movsx r8, word[rbp - 47] ; load left for plus
	movsx r9, word[rbp - 16] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 49], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 3
	mov word[rbp - 51], r8w
	mov r8, 3
	movsx r9, word[rbp - 10] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 53], r9w
	movsx r8, word[rbp - 49] ; load left for plus
	movsx r9, word[rbp - 53] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 55], r9w
	movsx r8, word[rbp - 55]; store i of a[i] in r8
	cmp r8, -3
	jl exit_array_error
	cmp r8, 0
	jg exit_array_error
	add r8,3
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 32
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 57], r11w
	movsx r8, word[rbp - 43] ; load left for plus
	movsx r9, word[rbp - 57] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 59], r9w
	movsx r8, word[rbp - 59]
	mov word[rbp - 4], r8w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 0
	mov word[rbp - 61], r8w
	mov r8, 0; store i of a[i] in r8
	cmp r8, 0
	jl exit_array_error
	cmp r8, 0
	jg exit_array_error
	sub r8,0
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 33
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 63], r11w
	movsx r8, word[rbp - 4] ; load left for plus
	movsx r9, word[rbp - 63] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 65], r9w
	movsx r8, word[rbp - 2] ; load left for plus
	movsx r9, word[rbp - 65] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 67], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 5
	mov word[rbp - 69], r8w
	mov r8, 5
	movsx r9, word[rbp - 10] ; load right for plus
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 71], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 7
	mov word[rbp - 73], r8w
	movsx r8, word[rbp - 71] ; load left for plus
	mov r9, 7
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 75], r9w
	movsx r8, word[rbp - 75]; store i of a[i] in r8
	cmp r8, 0
	jl exit_array_error
	cmp r8, 0
	jg exit_array_error
	sub r8,0
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 33
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 77], r11w
	movsx r8, word[rbp - 67] ; load left for plus
	movsx r9, word[rbp - 77] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 79], r9w
	movsx r8, word[rbp - 79]
	neg r8 ; invert sign for unary minus
	sub rsp, 2 ; allocate space for temp
	 mov word[rbp - 81], r8w
	movsx r8, word[rbp - 81]
	mov word[rbp - 6], r8w
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 4]
	call _print_integer_id
	mov rsp, rbx
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 6]
	call _print_integer_id
	mov rsp, rbx
	; allocate space for 'for' construct
	 sub rsp, 4
	mov word[rbp - 35], 10; initialize counter with begin
	label0:
	cmp word[rbp - 35], 16;
	jz label1
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 12
	mov word[rbp - 39], r8w
	mov word[rbp - 37], 12
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 12
	mov word[rbp - 41], r8w
	movsx r8, word[rbp - 35] ; load left for plus
	mov r9, 12
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 43], r9w
	movsx r8, word[rbp - 43]; store i of a[i] in r8
	cmp r8, -3
	jl exit_array_error
	cmp r8, 0
	jg exit_array_error
	add r8,3
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 32
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 45], r11w
	movsx r8, word[rbp - 37] ; load left for plus
	movsx r9, word[rbp - 45] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 47], r9w
	movsx r8, word[rbp - 47]
	mov word[rbp - 6], r8w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 2
	mov word[rbp - 49], r8w
	movsx r8, word[rbp - 35] ; load left for plus
	mov r9, 2
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 51], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 25
	mov word[rbp - 53], r8w
	movsx r8, word[rbp - 51] ; load left for plus
	mov r9, 25
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 55], r9w
	;node integer literal, store it in temporary
	sub rsp, 2
	mov r8, 2
	mov word[rbp - 57], r8w
	movsx r8, word[rbp - 10] ; load left for plus
	mov r9, 2
	imul r9, r8
	sub rsp, 2; make space for multiplication temporary
	mov word[rbp - 59], r9w
	movsx r8, word[rbp - 55] ; load left for plus
	movsx r9, word[rbp - 59] ; load right for plus
	add r9, r8
	sub rsp, 2
	mov word[rbp - 61], r9w
	movsx r8, word[rbp - 61]; store i of a[i] in r8
	cmp r8, 0
	jl exit_array_error
	cmp r8, 0
	jg exit_array_error
	sub r8,0
	add r8, 1
; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)
	imul r8, 2
	add r8, 33
	; now r8 contains the index of the required element
	sub rsp, 2; allocate space a[i] in temporary
	mov r12, rbp
	sub r12, r8
	movsx r11, word[r12]
	mov word[rbp - 63], r11w
	movsx r8, word[rbp - 6] ; load left for plus
	movsx r9, word[rbp - 63] ; load right for plus
	sub r8, r9
	mov r9, r8
	sub rsp, 2
	mov word[rbp - 65], r9w
	movsx r8, word[rbp - 65]
	mov word[rbp - 4], r8w
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 35]
	call _print_integer_id
	mov rsp, rbx
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 6]
	call _print_integer_id
	mov rsp, rbx
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 4]
	call _print_integer_id
	mov rsp, rbx
	add word[rbp - 35], 1
	jmp label0
	label1:
	; deallocate space for 'for' construct
	 add rsp, 172
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 4]
	call _print_integer_id
	mov rsp, rbx
	mov rbx, rsp
	and rsp, -16
	movsx rax, word[rbp - 6]
	call _print_integer_id
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 21];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 23];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 25];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 27];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 29];
	call _printf
	;restore stack
	mov rsp, rbx
	;align stack
	mov rbx, rsp
	and rsp, -16
	lea rdi, [rel int_output_format_same_line];
	movzx rsi, word [rbp - 31];
	call _printf
	;restore stack
	mov rsp, rbx
	lea rdi, [rel new_line_character_string];
	call _printf
	;deallocate space from module
	add rsp, 79
	pop rbp
	ret
