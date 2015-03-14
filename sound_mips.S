	.align 4
	.globl  DecodeBlockAsm
	.globl  DecodeBlockAsm2
	

.macro	SMP12CLIP16		
	BGT 	$8, 0xFFFF8000,1111f
	nop
	LI    $8, 0xFFFF8000
	J 		1112f
	nop
1111:
	BLT 	$8, 0x7FFF,1112f
	nop
	li 	$8, 0x7FFF
1112:	
	BGT 	$6, 0xFFFF8000,1121f	
	nop
	li 		$6, 0xFFFF8000
	J 		1122f
	nop
1121:
	BLT		$6, 0x7FFF,1122f
	nop
	li 	$6, 0x7FFF
1122:
.endm

	
	
/*
;Bit-Rate Expand Waveform
;
;Desc:
;   Decompresses a 9-byte bit-rate reduced block into 16 16-bit samples.
;   This procedure is designed to be recursively called to decompress a series of blocks.
;In:
;   $4=ESI-> Sample Block
;   $5=EDI   -> Output buffer
;   $6=EDX   =3D Last sample of previous block (32-bit)
;   $7=EBX   =3D Next to last sample (sign extended from 16-bits)
;Out:
;   $4=ESI -> Next Block
;   $5=EDI -> After last sample
;   $6=EDX =3D Last sample (32-bit)
;   $7=EBX =3D Next to last sample (16-bit)
;Destroys:
;   $8=EAX	
*/


	.align 4
DecodeBlockAsm:
		
//LDR		$6,[$6]	
	lw		$6,($6)
//LDR		$7,[$7]
	lw		$7,($7)
//BL		BREWave
	jal		BREWave
	nop
//LDMFD		SP!,{$4,$5}
//STR		$6,[$4]
	lw		$6,($4)
//STR		$7,[$5]
	lw		$7,($5)
		
//LDMFD		SP!,{$8,LR}
//MOV		PC,LR	
	jr		$ra
	nop

BREWave:	
//STMFD	SP!,{$9,$10,$11}
	//Mov	AL,[ESI]                           ;Get header byte
	//Inc	ESI
	
//LDRB	$8,[$4],#1
	lbu		$8,($4)
	addu	$4,$4,0x1

	//Mov	CL,0CFh
	//Sub	CL,AL	
	//SetC	AH
	//Dec	AH
	//And	CL,AH
	//ShR	CL,4                               ;Isolate range
//CMP	$8,#0xD0
	blt		$8,0xD0,label11
	nop
//MOVHS	$9,#0
	move	$9,$0
	j			label12
	nop
label11:	
//MOVLO	$9,#0xCF
//SUBLO	$9,$9,$8
//MOVLO	$9,$9,LSR #4	
	li		$9,0xCF
	sub		$9,$9,$8
	srl		$9,$9, 4
label12:				
		
	//Mov	CH,8                               ;Decompress 8 bytes (16 nybbles)
//MOV	$10,#8	
	li		$10,0x8
	//Test	AL,0Ch                             ;Does block use ADPCM compression?			
	//JZ 	@@Method0                    ;  No
//TST	$8,#0xC
//BEQ	Method0
	andi	$12,$8,0xC
	beqz	$12,Method0
	nop

	//Test	AL,8                               ;Does block use method 1?
	//JZ 	@@Method1                    ;  Yes
//TST	$8,#0x8
//BEQ	Method1
	andi	$12,$8,0x8
	beqz	$12,Method1
	nop
	
	//Test	AL,4                               ;Does block use method 2?
	//jnz 	@@Method3                          ;  Yes
	//jmp	@@Method2
//TST	$8,#0x4
//BEQ	Method2
	andi	$12,$8,0x4
	beqz	$12,Method2
	nop
//B	Method3			//;Must use method 3
	j Method3
	nop
/*
ALIGN	16
	;[Smp] ----------------------------------*/
Method0:
//ADD	$9,$9,#16
	addu	$9,$9, 16	
Method0loop:
	//XOr	EAX,EAX
	//XOr	EDX,EDX
	//Mov	AH,byte[ESI]                        ;Get byte
	//Mov	DH,AH
	//And	AH,0F0h                         ;AH = High nybble << 12
	//ShL	DH,4                            ;DH = Low nybble << 12
//LDRB	$8,[$4],#1
	lbu		$8,($4)
	addu	$4,$4,1
	
	 //$8 = 0x000000hl
//MOV	$6,$8,LSL #(28)
	sll		$6,$8, 28
	 //$6 = 0xl0000000
//MOV	$8,$8,LSL #(24)
	sll		$6,$8, 24
	 //$8 = 0xhl000000
//BIC	$8,$8,#0x0F000000
	andi	$8,$8,#0xF0FFFFFF
	
	 //$8 = 0xh0000000
	 	
	//SAR	AX,CL                           ;Reduce samples according to range
	//SAR	DX,CL
//MOV	$8,$8,ASR $9	
	sra		$8,$8,$9
//MOV	$6,$6,ASR $9
	sra		$6,$6,$9
		
	//Mov	word[EDI],AX
	//Mov	word[2+EDI],DX
//STRH	$8,[$5],#2
	sh		$8,($5)	
//STRH	$6,[$5],#2
	sh		$6,2($5)
	addu	$5,$5,4
	
	//Add	EDI,4
	//Inc	ESI
	//Dec	CH
	//JNZ	Short @@Method0
	//MovSX	EDX,DX
	//MovSX	EBX,AX
	//Ret	
//SUBS	$10,$10,#1
	sub	$10,$10,1
//BNE	Method0loop	
	bnez	$10,Method0loop
	nop
//MOV	$6,$6,LSL #16
	sll		$6,$6,16
//MOV	$6,$6,ASR #16
	sra		$6,$6,16
//MOV	$7,$8,LSL #16
	sll		$7,$8,16
//MOV	$7,$7,ASR #16		
	sra		$7,$7,16
//LDMFD	SP!,{$9,$10,$11}
//MOV	PC,LR
	jr		$ra
	nop
//ALIGN	16
//	;[Delta]+[Smp-1](15/16) -----------------
Method1:
//ADD	$11,$9,#16
	add		$11,$9,16
Method1loop:	
		//MovSX	EBX,byte[ESI]              ;Sign extend upper nybble into EBX
		//And	BL,0F0h
		//ShL	EBX,8
		//SAR	EBX,CL
//LDRSB   $7,[$4]
		lb		$7,($4)
//BIC	$7,$7,#0xF
		and		$7,$7,0xFFFFFFF0
//MOV	$7,$7,LSL #8
		sll		$7,$7,8
//MOV	$7,$7,ASR $9
		srav	$7,$7,$9

		//MovSX	EAX,DX
		//Add	EBX,EAX
		//SAR	EAX,4
		//Sub	EBX,EAX
//MOV	$8,$6,LSL #16		
		sll		$8,$6,16
//ADD	$7,$7,$8,ASR #16		
//SUB	$7,$7,$8,ASR #20
		sra		$12,$8,16
		sra		$13,$8,20
		add		$7,$12,$7
		sub		$7,$7,$13
		

		//Mov	word[EDI],BX
//STRH	$7,[$5],#2
		sh		$7,($5)		

		//Mov	DL,byte[ESI]
		//ShL	EDX,12
		//MovSX	EDX,DX
		//SAR	EDX,CL
//LDRSB   $6,[$4],#1		
		lb		$6,($4)
		addu		$4,$4,1
//MOV	$6,$6,LSL #(12+16)		
		sll		$6,$6,28
//MOV	$6,$6,ASR $11
		srav	$6,$6,$11

		//MovSX	EAX,BX
		//Add	EDX,EAX
		//SAR	EAX,4
		//Sub	EDX,EAX
//MOV	$8,$7,LSL #16		
		sll		$8,$7,16
//ADD	$6,$6,$8,ASR #16		
//SUB	$6,$6,$8,ASR #20
		sra		$12,$8,16
		sra		$13,$8,20
		add		$6,$6,$12
		sub		$6,$6,$13

		//Mov	word[2+EDI],DX
//STRH	$6,[$5],#2
		sh		$6, 2($5)
		//Add	EDI,4
		addu	$5,$5,4

		//Inc	ESI

	//Dec	CH
	//JNZ	Short @@Method1
	//MovSX	EBX,BX
	//Ret
//SUBS	$10,$10,#1
	sub		$10,$10,1
//BNE	Method1loop
	bnez	$10,Method1loop
	nop
	
//MOV	$7,$7,LSL #16
	sll		$7,$7,16
//MOV	$7,$7,ASR #16
	sra		$7,$7,16
	
//LDMFD	SP!,{$9,$10,$11}
//MOV	PC,LR
	jr		$ra
	nop
//ALIGN	16
	//;[Delta]+[Smp-1](61/32)-[Smp-2](30/32) --
Method2:

//ADD	$11,$9,#16
	add		$11,$9,16
Method2loop:
		//MovSX	EAX,Byte[ESI]              ;EAX = Delta
//LDRSB	$8,[$4]
	lb		$8,($4)
		//And	AL,0F0h
		//ShL	EAX,8
		//SAR	EAX,CL
//BIC	$8,$8,#0xF
	and		$8,$8,0xFFFFFFF0
//MOV	$8,$8,LSL #8
	sll		$8,$8,8
//MOV	$8,$8,ASR $9
	srav	$8,$8,$9

		//;Subtract 15/16 of second sample -----
		//Sub	EAX,EBX
		//SAR	EBX,4
		//Add	EAX,EBX
		//MovSX	EBX,DX
//SUB	$8,$8,$7
//ADD	$8,$8,$7,ASR #4
//MOV	$7,$6
	sra		$12,$7,4
	sub		$8,$8,$7
	add		$8,$8,$12
	move	$7,$6
		
		//;Add 61/32 of last sample ------------
		//And	DL,~3
		//Add	EAX,EDX
		//Add	EAX,EDX
		//SAR	EDX,4
		//Sub	EAX,EDX
		//SAR	EDX,1		
//BIC	$6,$6,#3
	and		$6,$6,0xFFFFFFFC
//ADD	$8,$8,$6,LSL #1			
//SUB	$8,$8,$6,ASR #4			
	sll		$12,$6,1			
	sra		$13,$6,4
	sra		$14,$6,5
	add		$8,$8,$12
	sub		$8,$8,$13
	
		//MovSX	EDX,DX
		//Sub	EAX,EDX		
//SUB	$8,$8,$6,ASR #5
	sub		$8,$8,$14
		//Mov	word[EDI],AX
//STRH	$8,[$5],#2
	sh		$8,($5)

		//Mov	DL,byte[ESI]
//LDRB	$6,[$4],#1
	lbu		$6,($4)
	addu	$4,$4,1
		//ShL	EDX,12
		//MovSX	EDX,DX
		//SAR	EDX,CL
//MOV	$6,$6,LSL #(12+16)		
	sll		$6,$6,28	
//MOV	$6,$6,ASR $11
	sra		$6,$6,$11

		//Sub	EDX,EBX
		//SAR	EBX,4
		//Add	EDX,EBX
		//MovSX	EBX,AX
//SUB	$6,$6,$7
//ADD	$6,$6,$7,ASR #4
	sra		$12,$7,4
	sub		$6,$6,$7
	add		$6,$6,$12
//MOV	$7,$8
	move	$7,$8
		
		//And	AL,~3
		//Add	EDX,EAX
		//Add	EDX,EAX
		//SAR	EAX,4
		//Sub	EDX,EAX
		//SAR	EAX,1
//BIC	$8,$8,#3
	and		$8,$8,0xFFFFFFFC
//ADD	$6,$6,$8,LSL #1		
//SUB	$6,$6,$8,ASR #4		
	sll		$12,$8,1
	sra		$13,$8,4
	sra		$14,$8,5
	add		$6,$6,$12
	sub		$6,$6,$13
		//MovSX	EAX,AX
		//Sub	EDX,EAX		
//SUB	$6,$6,$8,ASR #5		
	sub		$6,$6,$14

		//Mov	word[2+EDI],DX
//STRH	$6,[$5],#2
	sh		$6,2($5)	
		//Add	EDI,4
	addu	$5,$5,4

		//Inc	ESI

	//Dec	CH
	//JNZ	@@Method2
	//Ret
//SUBS	$10,$10,#1
	sub		$10,$10,1
//BNE	Method2loop
	bnez	$10,Method2loop
	nop
//LDMFD	SP!,{$9,$10,$11}
//MOV	PC,LR

	jr      $ra 
	nop
//ALIGN	16
	//;[Delta]+[Smp-1](115/64)-[Smp-2](52/64) -
Method3:

//ADD	$11,$9,#16
	add		$11,$9,16
Method3loop:
	//MovSX	EAX,Byte[ESI]
//LDRSB	$8,[$4]
	lb		$8,($4)
		//And	AL,0F0h
		//ShL	EAX,8
		//SAR	EAX,CL
//BIC	$8,$8,#0xF
	and		$8,$8,0xFFFFFFF0
//MOV	$8,$8,LSL #8
	sll		$8,$8,8
//MOV	$8,$8,ASR $9
	srav	$8,$8,$9

		//;Subtract 13/16 of second sample -----
		//Sub	EAX,EBX
		//SAR	EBX,3
		//Add	EAX,EBX
		//SAR	EBX,1
		//Add	EAX,EBX
		//MovSX	EBX,DX
//SUB	$8,$8,$7	
//ADD	$8,$8,$7,ASR #3
//ADD	$8,$8,$7,ASR #4
	sra		$12,$7,3
	sra		$13,$7,4
	sub		$8,$8,$7
	add		$8,$8,$12
	add		$8,$8,$13
//MOV	$7,$6
	move	$7,$6

		//;Add 115/64 of last sample -----------
		//And	DL, ~3
		//Add	EAX,EDX
		//Add	EAX,EDX
		//SAR	EDX,3
		//Sub	EAX,EDX
		//SAR	EDX,1
		//Sub	EAX,EDX
		//SAR	EDX,2
		//Sub	EAX,EDX
//BIC	$6,$6,#3
	and		$6,$6,0xFFFFFFFC
//ADD	$8,$8,$6,LSL #1
//SUB	$8,$8,$6,ASR #3
//SUB	$8,$8,$6,ASR #4
//SUB	$8,$8,$6,ASR #6
	sll		$12,$6,1
	sra		$13,$6,3
	sra		$14,$6,4
	sra		$15,$6,6
	add		$8,$8,$12
	sub		$8,$8,$13
	sub		$8,$8,$14
	sub		$8,$8,$15

		//Mov	word[EDI],AX
//STRH	$8,[$5],#2
	sh		$8,($5)

		//Mov	DL,byte[ESI]
//LDRB	$6,[$4],#1
	lbu		$6,($4)
	addu	$4,$4,1

		//ShL	EDX,12
		//MovSX	EDX,DX
		//SAR	EDX,CL
//MOV	$6,$6,LSL #(12+16)
	sll		$6,$6,28
//MOV	$6,$6,ASR $11
	srav		$6,$6,$11

		//Sub	EDX,EBX
		//SAR	EBX,3
		//Add	EDX,EBX
		//SAR	EBX,1
		//Add	EDX,EBX
		//MovSX	EBX,AX
//SUB	$6,$6,$7
//ADD	$6,$6,$7,ASR #3
//ADD	$6,$6,$7,ASR #4
	sra		$12,$7,3
	sra		$13,$7,4
	sub		$6,$6,$7
	add		$6,$6,$12
	add		$6,$6,$13
//MOV	$7,$8
	move	$7,$8

		//And	AL, ~3
		//Add	EDX,EAX
		//Add	EDX,EAX
		//SAR	EAX,3
		//Sub	EDX,EAX
		//SAR	EAX,1
		//Sub	EDX,EAX
		//SAR	EAX,2
		//Sub	EDX,EAX
//BIC	$8,$8,#3
	and		$8,$8,0xFFFFFFFC
//ADD	$6,$6,$8,LSL #1
//SUB	$6,$6,$8,ASR #3
//SUB	$6,$6,$8,ASR #4
//SUB	$6,$6,$8,ASR #6
	sll		$12,$8,1
	sra		$13,$8,3
	sra		$14,$8,4
	sra		$15,$8,6
	add		$6,$6,$12
	sub		$6,$6,$13
	sub		$6,$6,$14
	sub		$6,$6,$15

		//Mov	word[2+EDI],DX
//STRH	$8,[$5],#2
	sh		$8,2($5)
	addu	$5,$5,4
		//Add	EDI,4

		//Inc	ESI

	//Dec	CH
	//JNZ	@@Method3
	//Ret
//SUBS	$10,$10,#1
	sub		$10,$10,1
//BNE	Method3loop
	bnez	$10,Method3loop
	nop
//LDMFD	SP!,{$9,$10,$11}

	jr      $ra 
	nop

DecodeBlockAsm2:	
	jr      $ra 
	nop
	