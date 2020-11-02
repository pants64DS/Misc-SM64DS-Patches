@ at the beginning of Actor::Spawn
nsub_02010e34:
	ldr   r5, =spawningActor
	strh  r0,[r5]
	mov   r5, r0
	b     0x02010e38

@ at the end of Actor::Spawn
nsub_02010e6c:
	ldr   r5, =spawningActor
	mov   r4, #0
	strh  r4,[r5]
	pop   {r4, r5, r15}

.global _Z18AllocateOnGameHeapj
_Z18AllocateOnGameHeapj:
	push  {r4, r5, r14}
	b     _ZN9ActorBasenwEj + 4
