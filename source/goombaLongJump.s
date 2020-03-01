
@ make the player continue long jumping after landing a long jump on a goomba
repl_0212a290_ov_54:
	push	{r2, r3}
	ldr		r2, [r0, #0x370]
	ldr		r3, =0x0211055c
	cmp		r2, r3
	pop		{r2, r3}
	bne		0x020d932c
	str 	r1, [r0, #0xa8]
	bx		lr