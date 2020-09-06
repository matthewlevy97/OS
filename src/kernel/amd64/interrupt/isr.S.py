#!/usr/bin/env python3
import sys

num_isr = 256
isr_pushes_error = [8, 10, 11, 12, 13, 14, 17]

asm_file = '''
.intel_syntax noprefix

.extern isr_common
'''

# Create interrupts
asm_file += '''
// ISR Routines
'''
for i in range(num_isr):
	asm_file += '''isr{0}:
	cli{1}
	push {0}
	jmp isr_common
'''.format(i, '\n	push 0' if i not in isr_pushes_error else '')

# Create ISR Vector Table
asm_file += '''
.section .rodata

// ISR Vector Table
.global isr_table
isr_table:
'''
for i in range(num_isr):
	asm_file += '	.quad isr{}\n'.format(i)

sys.stdout.write(asm_file)