[BITS 64]
[ORG 0xffffffffffc03000]

isr_task_region		equ 0xffffffffffc02000
isr_save_region		equ 0xffffffffffc02e00
transfer_control	equ 0xffffffffffe00000
task_state_region	equ 0xffffffffffe01000

isr_table:
	dq int_isr_0
	dq int_isr_1
	dq int_isr_2
	dq int_isr_3
	dq int_isr_4
	dq int_isr_5
	dq int_isr_6
	dq int_isr_7
	dq int_isr_8
	dq int_isr_9
	dq int_isr_10
	dq int_isr_11
	dq int_isr_12
	dq int_isr_13
	dq int_isr_14
	dq int_isr_15
	dq int_isr_16
	dq int_isr_17
	dq int_isr_18
	dq int_isr_19
	dq int_isr_20
	dq int_isr_21
	dq int_isr_22
	dq int_isr_23
	dq int_isr_24
	dq int_isr_25
	dq int_isr_26
	dq int_isr_27
	dq int_isr_28
	dq int_isr_29
	dq int_isr_30
	dq int_isr_31
	dq int_isr_32
	dq int_isr_33
	dq int_isr_34
	dq int_isr_35
	dq int_isr_36
	dq int_isr_37
	dq int_isr_38
	dq int_isr_39
	dq int_isr_40
	dq int_isr_41
	dq int_isr_42
	dq int_isr_43
	dq int_isr_44
	dq int_isr_45
	dq int_isr_46
	dq int_isr_47
	dq int_isr_48
	dq int_isr_49
	dq int_isr_50
	dq int_isr_51
	dq int_isr_52
	dq int_isr_53
	dq int_isr_54
	dq int_isr_55
	dq int_isr_56
	dq int_isr_57
	dq int_isr_58
	dq int_isr_59
	dq int_isr_60
	dq int_isr_61
	dq int_isr_62
	dq int_isr_63
	dq int_isr_64
	dq int_isr_65
	dq int_isr_66
	dq int_isr_67
	dq int_isr_68
	dq int_isr_69
	dq int_isr_70
	dq int_isr_71
	dq int_isr_72
	dq int_isr_73
	dq int_isr_74
	dq int_isr_75
	dq int_isr_76
	dq int_isr_77
	dq int_isr_78
	dq int_isr_79
	dq int_isr_80
	dq int_isr_81
	dq int_isr_82
	dq int_isr_83
	dq int_isr_84
	dq int_isr_85
	dq int_isr_86
	dq int_isr_87
	dq int_isr_88
	dq int_isr_89
	dq int_isr_90
	dq int_isr_91
	dq int_isr_92
	dq int_isr_93
	dq int_isr_94
	dq int_isr_95
	dq int_isr_96
	dq int_isr_97
	dq int_isr_98
	dq int_isr_99
	dq int_isr_100
	dq int_isr_101
	dq int_isr_102
	dq int_isr_103
	dq int_isr_104
	dq int_isr_105
	dq int_isr_106
	dq int_isr_107
	dq int_isr_108
	dq int_isr_109
	dq int_isr_110
	dq int_isr_111
	dq int_isr_112
	dq int_isr_113
	dq int_isr_114
	dq int_isr_115
	dq int_isr_116
	dq int_isr_117
	dq int_isr_118
	dq int_isr_119
	dq int_isr_120
	dq int_isr_121
	dq int_isr_122
	dq int_isr_123
	dq int_isr_124
	dq int_isr_125
	dq int_isr_126
	dq int_isr_127
	dq int_isr_128
	dq int_isr_129
	dq int_isr_130
	dq int_isr_131
	dq int_isr_132
	dq int_isr_133
	dq int_isr_134
	dq int_isr_135
	dq int_isr_136
	dq int_isr_137
	dq int_isr_138
	dq int_isr_139
	dq int_isr_140
	dq int_isr_141
	dq int_isr_142
	dq int_isr_143
	dq int_isr_144
	dq int_isr_145
	dq int_isr_146
	dq int_isr_147
	dq int_isr_148
	dq int_isr_149
	dq int_isr_150
	dq int_isr_151
	dq int_isr_152
	dq int_isr_153
	dq int_isr_154
	dq int_isr_155
	dq int_isr_156
	dq int_isr_157
	dq int_isr_158
	dq int_isr_159
	dq int_isr_160
	dq int_isr_161
	dq int_isr_162
	dq int_isr_163
	dq int_isr_164
	dq int_isr_165
	dq int_isr_166
	dq int_isr_167
	dq int_isr_168
	dq int_isr_169
	dq int_isr_170
	dq int_isr_171
	dq int_isr_172
	dq int_isr_173
	dq int_isr_174
	dq int_isr_175
	dq int_isr_176
	dq int_isr_177
	dq int_isr_178
	dq int_isr_179
	dq int_isr_180
	dq int_isr_181
	dq int_isr_182
	dq int_isr_183
	dq int_isr_184
	dq int_isr_185
	dq int_isr_186
	dq int_isr_187
	dq int_isr_188
	dq int_isr_189
	dq int_isr_190
	dq int_isr_191
	dq int_isr_192
	dq int_isr_193
	dq int_isr_194
	dq int_isr_195
	dq int_isr_196
	dq int_isr_197
	dq int_isr_198
	dq int_isr_199
	dq int_isr_200
	dq int_isr_201
	dq int_isr_202
	dq int_isr_203
	dq int_isr_204
	dq int_isr_205
	dq int_isr_206
	dq int_isr_207
	dq int_isr_208
	dq int_isr_209
	dq int_isr_210
	dq int_isr_211
	dq int_isr_212
	dq int_isr_213
	dq int_isr_214
	dq int_isr_215
	dq int_isr_216
	dq int_isr_217
	dq int_isr_218
	dq int_isr_219
	dq int_isr_220
	dq int_isr_221
	dq int_isr_222
	dq int_isr_223
	dq int_isr_224
	dq int_isr_225
	dq int_isr_226
	dq int_isr_227
	dq int_isr_228
	dq int_isr_229
	dq int_isr_230
	dq int_isr_231
	dq int_isr_232
	dq int_isr_233
	dq int_isr_234
	dq int_isr_235
	dq int_isr_236
	dq int_isr_237
	dq int_isr_238
	dq int_isr_239
	dq int_isr_240
	dq int_isr_241
	dq int_isr_242
	dq int_isr_243
	dq int_isr_244
	dq int_isr_245
	dq int_isr_246
	dq int_isr_247
	dq int_isr_248
	dq int_isr_249
	dq int_isr_250
	dq int_isr_251
	dq int_isr_252
	dq int_isr_253
	dq int_isr_254
	dq int_isr_255

%macro basic_isr 1
int_isr_%1:
	; exception code on TOS
	; save rax/rbx/rdi/rsi
	push	rax
	push	rbx
	push	rdi
	push	rsi

	xor	rbx, rbx
	; determine if this has an exception code or not
	; uses assumption that RIP >= 0x1000
	; read the second element of the iret region
	; if it's CS, it'll be < 0x1000
	cmp	qword [rsp + 32 + 8], 0x1000
	jb	.skip_code

	; save exception code to rbx
	mov	rbx, qword [rsp + 32]

	; shift the stack
	lea	rdi, [rsp + 32]
	lea	rsi, [rsp + 24]
	cld
	movsq
	movsq
	movsq
	movsq
	add	rsp, 8

.skip_code:
	mov	rsi, [isr_task_region + %1*8]
	cmp	qword rsi, 0
	je	.skip_task

	mov	qword [rsi + 5*8], %1 ; rdi
	mov	qword [rsi + 4*8], rbx ; rsi
	mov	rdi, qword [task_state_region]
	mov	qword [rsi + 3*8], rdi ; rdx

	call	transfer_control
.skip_task:
	pop	rsi
	pop	rdi
	pop	rbx
	pop	rax

	iretq
%endmacro

%macro timer_isr 1
int_isr_%1:
	push	r9
	push	rax
	mov	r9, 0xffff900000000000 ; status page base
	; grab increment
	mov	eax, dword [r9 + 0xc00]
	; add increment
	add	dword [r9 + 0x000], eax
	pop	rax
	mov	r9, 0xffffc00000000000 + 0xfee00000 ; phy map base + apic base
	; signal end-of-interrupt
	mov	dword [r9 + 0xb0], 0
	pop	r9

	iretq
%endmacro

basic_isr 0
basic_isr 1
basic_isr 2
basic_isr 3
basic_isr 4
basic_isr 5
basic_isr 6
basic_isr 7
basic_isr 8
basic_isr 9
basic_isr 10
basic_isr 11
basic_isr 12
basic_isr 13
basic_isr 14
basic_isr 15
basic_isr 16
basic_isr 17
basic_isr 18
basic_isr 19
basic_isr 20
basic_isr 21
basic_isr 22
basic_isr 23
basic_isr 24
basic_isr 25
basic_isr 26
basic_isr 27
basic_isr 28
basic_isr 29
basic_isr 30
basic_isr 31
basic_isr 32
basic_isr 33
basic_isr 34
basic_isr 35
basic_isr 36
basic_isr 37
basic_isr 38
basic_isr 39
basic_isr 40
basic_isr 41
basic_isr 42
basic_isr 43
basic_isr 44
basic_isr 45
basic_isr 46
basic_isr 47
basic_isr 48
basic_isr 49
basic_isr 50
basic_isr 51
basic_isr 52
basic_isr 53
basic_isr 54
basic_isr 55
basic_isr 56
basic_isr 57
basic_isr 58
basic_isr 59
basic_isr 60
basic_isr 61
basic_isr 62
basic_isr 63
basic_isr 64
basic_isr 65
basic_isr 66
basic_isr 67
basic_isr 68
basic_isr 69
basic_isr 70
basic_isr 71
basic_isr 72
basic_isr 73
basic_isr 74
basic_isr 75
basic_isr 76
basic_isr 77
basic_isr 78
basic_isr 79
basic_isr 80
basic_isr 81
basic_isr 82
basic_isr 83
basic_isr 84
basic_isr 85
basic_isr 86
basic_isr 87
basic_isr 88
basic_isr 89
basic_isr 90
basic_isr 91
basic_isr 92
basic_isr 93
basic_isr 94
basic_isr 95
;basic_isr 96
timer_isr 96
basic_isr 97
basic_isr 98
basic_isr 99
basic_isr 100
basic_isr 101
basic_isr 102
basic_isr 103
basic_isr 104
basic_isr 105
basic_isr 106
basic_isr 107
basic_isr 108
basic_isr 109
basic_isr 110
basic_isr 111
basic_isr 112
basic_isr 113
basic_isr 114
basic_isr 115
basic_isr 116
basic_isr 117
basic_isr 118
basic_isr 119
basic_isr 120
basic_isr 121
basic_isr 122
basic_isr 123
basic_isr 124
basic_isr 125
basic_isr 126
basic_isr 127
basic_isr 128
basic_isr 129
basic_isr 130
basic_isr 131
basic_isr 132
basic_isr 133
basic_isr 134
basic_isr 135
basic_isr 136
basic_isr 137
basic_isr 138
basic_isr 139
basic_isr 140
basic_isr 141
basic_isr 142
basic_isr 143
basic_isr 144
basic_isr 145
basic_isr 146
basic_isr 147
basic_isr 148
basic_isr 149
basic_isr 150
basic_isr 151
basic_isr 152
basic_isr 153
basic_isr 154
basic_isr 155
basic_isr 156
basic_isr 157
basic_isr 158
basic_isr 159
basic_isr 160
basic_isr 161
basic_isr 162
basic_isr 163
basic_isr 164
basic_isr 165
basic_isr 166
basic_isr 167
basic_isr 168
basic_isr 169
basic_isr 170
basic_isr 171
basic_isr 172
basic_isr 173
basic_isr 174
basic_isr 175
basic_isr 176
basic_isr 177
basic_isr 178
basic_isr 179
basic_isr 180
basic_isr 181
basic_isr 182
basic_isr 183
basic_isr 184
basic_isr 185
basic_isr 186
basic_isr 187
basic_isr 188
basic_isr 189
basic_isr 190
basic_isr 191
basic_isr 192
basic_isr 193
basic_isr 194
basic_isr 195
basic_isr 196
basic_isr 197
basic_isr 198
basic_isr 199
basic_isr 200
basic_isr 201
basic_isr 202
basic_isr 203
basic_isr 204
basic_isr 205
basic_isr 206
basic_isr 207
basic_isr 208
basic_isr 209
basic_isr 210
basic_isr 211
basic_isr 212
basic_isr 213
basic_isr 214
basic_isr 215
basic_isr 216
basic_isr 217
basic_isr 218
basic_isr 219
basic_isr 220
basic_isr 221
basic_isr 222
basic_isr 223
basic_isr 224
basic_isr 225
basic_isr 226
basic_isr 227
basic_isr 228
basic_isr 229
basic_isr 230
basic_isr 231
basic_isr 232
basic_isr 233
basic_isr 234
basic_isr 235
basic_isr 236
basic_isr 237
basic_isr 238
basic_isr 239
basic_isr 240
basic_isr 241
basic_isr 242
basic_isr 243
basic_isr 244
basic_isr 245
basic_isr 246
basic_isr 247
basic_isr 248
basic_isr 249
basic_isr 250
basic_isr 251
basic_isr 252
basic_isr 253
basic_isr 254
basic_isr 255
