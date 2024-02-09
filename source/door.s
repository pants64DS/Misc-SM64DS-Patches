@ Prevent the main door model from rendering when a star requirement or keyhole model is rendered on top of it
nsub_0214550c_ov_64:
	ldr    r3, [r4, #0x138]
	cmp    r3, #0
	ldreq  r2, [r2, #0x14] @ Call ModelAnim::Render normally if the door only has one model
	ldrne  r2, [r2, #0xc]  @ Only call ModelAnim::UpdateVerts if the door has a second model
	b      0x02145510
