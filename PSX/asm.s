
	opt at-
	opt m-
	opt c+

	SECTION .text
	
	xdef transformVertexListA

	cnop 0,4
transformVertexListA
	; args are VERT *, long numverts, long *transverts, long *transdepths

	sll		a1,a1,2		; multiply by four
	addu	a1,a3

	lw		t0,(a0)		; load xy
	lw		t1,4(a0)	; load z
	lw		t2,8(a0)	; load xy
	lw		t3,12(a0)	; load z
	lw		t4,16(a0)	; load xy
	lw		t5,20(a0)	; load z

@transVertexLoop
	mtc2	t0, r0		; load gte registers
	mtc2	t1, r1	
	mtc2	t2, r2	
	mtc2	t3, r3	
	mtc2	t4, r4	
	mtc2	t5, r5

	lw		t0,24(a0)	; load xy (using delay slots)	
	lw		t1,28(a0)	; load z

	RTPT				; equiv. gte_rtpt_b

	lw		t2,32(a0)
	lw		t3,36(a0)	
	lw		t4,40(a0)	
	lw		t5,44(a0)


	swc2	r12,(a2)	; store screen verts
	swc2	r13,4(a2)
	swc2	r14,8(a2)

	mfc2	t6,r17
	nop
	srl		t6,t6,2
	sw		t6,(a3)		; store screen depths

	mfc2	t7,r18
	nop
	srl		t7,t7,2
	sw		t7,4(a3)	; store screen depths

	mfc2	t6,r19
	nop
	srl		t6,t6,2
	sw		t6,8(a3)	; store screen depths

	addu	a2,12
	addu	a3,12

	slt		at,a3,a1
	bne		at,zero,@transVertexLoop
	addu	a0,24		; he he! branch slot not going to waste here :)

	j	ra
	nop

