
@ spawn a feather instead of a Power Flower
nsub_02149ab0_ov_66:
	mov	 r4, #0
	b    0x02149ac4

@ don't despawn the feather even if the player isn't Mario
nsub_020b2ed0_ov_02:
	b    0x020b2ed8

@ give the player wings if the feather is collected
nsub_020e03e4_ov_02:
	b    0x020e03e8