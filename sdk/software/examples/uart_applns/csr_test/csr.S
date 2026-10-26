_start:
	andi  t1,x0,0
	addi  t0,t1,0x8
	csrs  mstatus, t0
	csrr  a5,mstatus    
