@ dive infinitely (with A button)
repl_020dce18_ov_02:
    ldr   r0, =PLAYER_ARR
    ldr   r0, [r0]
    ldr   r0, [r0, #0x370]   @ currState
    ldr   r1, =#0x021105bc   @ ST_DIVE
    cmp   r1, r0
    ldrne r1, =#0x021101fc   @ ST_SLIDE_KICK_RECOVER
    bxne  lr
    ldr   r0, =#0x0209f49e   @ INPUT_1_FRAME
    ldrh  r0, [r0]
    and   r0, r0, #1         @ A button
    cmp   r0, #0
    ldreq r1, =#0x021101fc   @ ST_SLIDE_KICK_RECOVER
    bx    lr