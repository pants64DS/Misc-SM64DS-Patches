
@ make the shell slow down on non-water surfaces
nsub_020cc3cc:
    str   r2,[r13]
    push  {r0-r3, r12}
    mov   r0, r6
    mov   r1, r4
    bl    GetShellSpeedLimit
    mov   r6, r0
    pop   {r0-r3, r12}
    b     0x020cc3d0

@ stop the shell from rotating
nsub_0214d0ac:
    ldrsh r1,[r3, #0x32]
    b     0x0214d0b0