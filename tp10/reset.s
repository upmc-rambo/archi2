#################################################################################
#	File : reset.s
#	Author : Alain Greiner
#	Date : 15/02/2011
#################################################################################
#	This is a boot code for a multi-processor architecture  supporting
#	multi-tasking. There is up to 4 tasks per processor, and
#	the number of processors cannot be larger than NB_PROCS.
#	Task(p,k) is identified by two indexes :
#	- p (between 0 and ND_PROCS-1) is the processor index
#	- k (between 0 and 3) is the task local index
#
#	The placement of tasks on the processors is statically defined
#	in the local tasks_entry_point[] array, that is indexed by m = 4*p + k.
#	Each entry contains a function pointer.
#
#	The actual number of tasks per processor can be smaller than 4.
#	It is defined in the _task_number_array[], indexed by the pid.
#	Each entry is an usigned char.
#
#	The ICU mask value for each processor is defined in the local
#	icu_masks_array[] indexed by the pid, with the following layout:
#	- IRQ_IN [0]		IOC
#	- IRQ_IN [8:11]		DMA_0 to DMA_3
#	- IRQ_IN [12:15]	TIMER_O to TIMER_3
#	- IRQ_IN [16:31]	TTY_0 to TTY_15
#
#	The _task_context_array[] layout is statically defined:
#	A single task context requires 256 bytes (64 words).
#	As there is at most 4 tasks per processor, and 4 processors,
#	the total memory space to store the contexts is 16*256 = 4K bytes.
#
#	This boot code :
#	- statically allocate tasks to processors, initialising
#	  the _tasks_entry_point[] and _task_number_array[].
#	- initializes the _task_context_array[] for tasks T(n,k) with k> 0
#	  (defines the future values for registers SR, EPC, SP, RA)
#	- It initializes the interrupt vector (TTYs, TIMERs, DMAs, IOC)
#	  and initializes the icu_mask_array[] for all processors.
#	- It defines the TICK value, and lauch the timer corresponding
#	  to the processor index.
#	- It initializes the EPC, SP, SR registers for tasks T(n,0), 
#	  and jumps to the task entry point in user mode.
#################################################################################
		
	.section .reset,"ax",@progbits

	.extern	seg_stack_base
	.extern	seg_data_base
	.extern	seg_icu_base
	.extern seg_timer_base
        .extern _isr_switch
        .extern _isr_tty_get_task0
        .extern _isr_tty_get_task1
        .extern _isr_tty_get_task2
        .extern _isr_tty_get_task3
        .extern _isr_dma
        .extern _isr_ioc
        .extern _interrupt_vector
	.extern	_task_context_array
	.extern	_task_number_array

	.ent	reset
	.align	2

reset:
       
	# Check the processor ID
	# If the processor ID is larger than 4, an error is reported
        mfc0    $10,    $15,    1               
	andi	$10,	$10,	0xFF		# $10 <= pid
        # define a stack for possible error signaling
        addi	$17,	$10, 	1		# $17 <= pid+1
	sll	$17,	$17,	16		# $17 <= (pid+1)*64K
	la	$18,	seg_stack_base
	addu	$29,	$18,	$17		# SP <= seg_stack_base + (pid+1)*64K 
	# actual check
	slti	$11,	$10,	4		# check pid < 4
        beqz    $11,    pid_stop

        # tasks_entry_point[] array initialization (done by all processors)
        # we define task_entry_point[p,k] <= main[x] for all T(n,k) tasks
        # that must be launched on the architecture 
        # The number of entries is 4 procs * 4 tasks
        # It is indexed by task_id = pid*4 + k
		la	$7,	seg_data_base
        la	$8,	tasks_entry_point
		
		#proc[0]
        lw	$9,	0($7)           # TO BE COMPLETED			# seg_data[0] #pgcd
        sw	$9,	0($8)			# set task_entry_point[0]
        lw	$9,	4($7)           # TO BE COMPLETED			# seg_data[1] #display conflit with dma
        sw	$9,	4($8)			# set task_entry_point[1]
        lw	$9,	12($7)           # TO BE COMPLETED			# seg_data[2] #prime
        sw	$9,	8($8)			# set task_entry_point[2]
        lw	$9,	0($7)           # TO BE COMPLETED			# seg_data[3] #pgcd
        sw	$9,	12($8)			# set task_entry_point[3]
		#proc[1]
        lw	$9,	0($7)                                       #pgcd
        sw	$9,	16($8)			# set task_entry_point[4]   
        lw	$9,	16($7) 
        sw	$9,	20($8)			# set task_entry_point[5]   #producer
        lw	$9,	20($7) 
        sw	$9,	24($8)			# set task_entry_point[6]   #consommer
        lw	$9,	24($7)
        sw	$9,	28($8)			# set task_entry_point[7]   #router
		#proc[2]
        lw	$9,	0($7)
        sw	$9,	32($8)			# set task_entry_point[8]   
        lw	$9,	24($7)
        sw	$9,	36($8)			# set task_entry_point[9]   #router
        lw	$9,	24($7)
        sw	$9,	40($8)			# set task_entry_point[10]  #router
        lw	$9,	0($7)
        sw	$9,	44($8)			# set task_entry_point[11]  
		#proc[3]
        lw	$9,	24($7)
        sw	$9,	48($8)			# set task_entry_point[12]  #router
        lw	$9,	12($7)
        sw	$9,	52($8)			# set task_entry_point[13]  #prime
        lw	$9,	12($7)
        sw	$9,	56($8)			# set task_entry_point[14]  #prime
        lw	$9,	12($7)
        sw	$9,	60($8)			# set task_entry_point[15]  #prime

        # _task_number_array[] initialization (done by all processors)
        # we must define the actual number of tasks assigned to each processor
        # this must be consistent with the task_entry_point array above.
        la	$8,	_task_number_array
        li	$9,	4               # TO BE COMPLETED
        sb	$9,	0($8)			# set_ task_number_array[0]
        li	$9,	4
        sb	$9,	1($8)			# set _task_number_array[1]
        li	$9,	4
        sb	$9,	2($8)			# set _task_number_array[2]
        li	$9,	4
        sb	$9,	3($8)			# set _task_number_array[3]

	# Each processor checks the number of tasks for himself 
	# If _task_number_array[p] larger than 4, an error is reported
	addu	$16,	$8,	$10		# $16 <= &_task_number_array[pid]
	lb	$17,	0($16)			# $17 <= task number
        addi    $20,	$17,	-1		# $20 <= kmax (highest local task index)
        # define a stack for possible error signaling
        addi	$17,	$10, 	1		# $17 <= pid+1
	sll	$17,	$17,	16		# $17 <= (pid+1)*64K
	la	$18,	seg_stack_base
	addu	$29,	$18,	$17		# SP <= seg_stack_base + (pid+1)*64K 
        # actual check
        slti    $19,    $20,	4		# check kmax < 4
	beqz	$19,	task_id_stop

	# Initializes the contexts for all tasks T(p,k) with k > 0 
	# future values of registers SR, EPC, SP, RA are defined.
ctx_loop:
	blez	$20,	ctx_loop_done		# exit loop if k <= 0
	sll	$30,	$10,	2		# $30 <= pid*4
	addu	$30,	$30,	$20		# $30 <= task_id = pid*4 + k
	sll	$19,	$30,	8		# $19 <= 256 * task_id
        la	$23,	_task_context_array
	addu	$23,	$23,	$19		# $23 <= &_task_context_array[task_id]

       	li	$16,	0x0000FF13	
        sw	$16,	0($23)			# context[n][k][SR] <= 0x0000FF13

	sll	$16,	$30,	2		# $16 <= 4*task_id
	la	$15,	tasks_entry_point
	add	$15,	$15,	$16		# $15 <= &tasks_entry_point[task_id]
	lw	$16,	0($15)			# $16 <= task_entry_point[task_id]
        sw	$16,	128($23)		# context[n][k][EPC] <= task_entry_point[task_id]

		addi	$15,	$30,	1		# $15 <= task_id + 1
		sll		$15,	$15,	16		# $15 <= (task_id + 1) * 64K
        la	$16,	seg_stack_base	
        add	$16,	$16,	$15		# $16 <= stack top address for T(n,k)
        sw	$16,	116($23)		# context[n][k][SP] <= T(n,k) stack pointer

	la	$16,	to_user
        sw	$16,	124($23)		# context[n][k][RA] <= ERET address 

	addi	$20,	$20,	-1		# k <= k-1
	j	ctx_loop
ctx_loop_done:

	# initializes interrupt vector (This is done by all processors)
	# This mut be consistent with the icu_mask_array[] below...
        la      $26,    _interrupt_vector       # interrupt vector address

        la      $27,    _isr_ioc 
        sw      $27,    0($26)                  # _interrupt_vector[0]  <= irq_ioc

        la	$27,   _isr_dma                  # TO BE COMPLETED
        sw	$27,   32($26)			# _interrupt_vector[8]  <= irq_dma[0]
        sw	$27,   36($26)			# _interrupt_vector[9]  <= irq_dma[1]
        sw	$27,   40($26)			# _interrupt_vector[10] <= irq_dma[2]
        sw	$27,   44($26)			# _interrupt_vector[11] <= irq_dma[3]

        la      $27,   _isr_switch           # TO BE COMPLETED
        sw      $27,   48($26)                  # _interrupt_vector[12] <= irq_timer[0]
        sw      $27,   52($26)                  # _interrupt_vector[13] <= irq_timer[1]
        sw      $27,   56($26)                  # _interrupt_vector[14] <= irq_timer[2]
        sw      $27,   60($26)                  # _interrupt_vector[15] <= irq_timer[3]

        la      $27,	_isr_tty_get_task0 
        sw      $27,	64($26)                  # _interrupt_vector[16] <= irq_tty[0,0]
        la      $27,	_isr_tty_get_task1 
        sw      $27,	68($26)                  # _interrupt_vector[17] <= irq_tty[0,1]
        la      $27,    _isr_tty_get_task2
        sw      $27,	72($26)                  # _interrupt_vector[18] <= irq_tty[0,2]
        la      $27,    _isr_tty_get_task3
        sw      $27,	76($26)                  # _interrupt_vector[19] <= irq_tty[0,3]
        la      $27,	_isr_tty_get_task0       #TO BE COMPLETED    
        sw      $27,	80($26)                  # _interrupt_vector[20] <= irq_tty[1,0]
        la      $27,	_isr_tty_get_task1       #TO BE COMPLETED   
        sw      $27,	84($26)                  # _interrupt_vector[21] <= irq_tty[1,1]
        la      $27,	_isr_tty_get_task2       #TO BE COMPLETED     
        sw      $27,    88($26)                  # _interrupt_vector[22] <= irq_tty[1,2]
        la      $27,	_isr_tty_get_task3       #TO BE COMPLETED     
        sw      $27,	92($26)                  # _interrupt_vector[23] <= irq_tty[1,3]
        la      $27,	_isr_tty_get_task0       #TO BE COMPLETED     
        sw      $27,	96($26)                  # _interrupt_vector[24] <= irq_tty[2,0]
        la      $27,	_isr_tty_get_task1       #TO BE COMPLETED     
        sw      $27,	100($26)                  # _interrupt_vector[25] <= irq_tty[2,1]
        la      $27,	_isr_tty_get_task2       #TO BE COMPLETED     
        sw      $27,    104($26)                  # _interrupt_vector[26] <= irq_tty[2,2]
        la      $27,    _isr_tty_get_task3       #TO BE COMPLETED     
        sw      $27,    108($26)                  # _interrupt_vector[27] <= irq_tty[2,3]
        la      $27,    _isr_tty_get_task0       #TO BE COMPLETED     
        sw      $27,    112($26)                  # _interrupt_vector[28] <= irq_tty[3,0]
        la      $27,    _isr_tty_get_task1       #TO BE COMPLETED    
        sw      $27,    116($26)                  # _interrupt_vector[29] <= irq_tty[3,1]
        la      $27,    _isr_tty_get_task2       #TO BE COMPLETED     
        sw      $27,    120($26)                  # _interrupt_vector[30] <= irq_tty[3,2]
        la      $27,    _isr_tty_get_task3       #TO BE COMPLETED    
        sw      $27,    124($26)                  # _interrupt_vector[31] <= irq_tty[3,3]

	# initializes ICU_MASK[pid] using the values defined in icu_mask_array[]
        sll	$17,	$10,	5		# $17 <= pid * 32
        la	$16,	seg_icu_base
	addu	$16,	$16,	$17		# $16 <= seg_icu_base + 32*pid
        sll	$18,	$10,	2		# $18 <= pid * 4
	la	$17,	icu_masks_array		
        addu	$17,	$17,	$18		# $17 <= &icu_masks_array[pid]
	lw	$17,	0($17)			# $17 <= icu_masks_array[pid]
	sw	$17,	8($16)			# MASK[pid] <= icu_masks_array[pid]

	# configure and start the timer for the context switch
        sll	$17,	$10,	4		# $17 <= pid*16
        la	$16,	seg_timer_base
	addu	$16,	$16,	$17		# $16 <= seg_timer_base + 16*pid
	li	$17,	10000           # TO BE COMPLETED
	sw	$17,	8($16)			# period <= 10000
	li	$17,	1
	sw	$17,	4($16)			# TIMER[pid] start

	# Initializes SR, SP & EPC for task T(p,0) 
	sll	$16,	$10,	2		# $16 <= taxk_id = pid*4
        addi	$17,	$16,	1		# $17 <= task_id + 1
	sll	$17,	$17,	16 		# $17 <= (task_id + 1)*64K
	la	$18,	seg_stack_base
	addu	$29,	$18,	$17		# SP <= seg_stack_base + (4*pid + 1)*64K 

	sll	$17,	$16,	2		# $17 <= 4*task_id 
	la	$18,	tasks_entry_point 
        addu	$18,	$18,	$17		# $18 <= &task_entry_point[task_id] 
	lw	$16,	0($18)			# $16 <= task_entry_point[task_id]
	mtc0	$16,	$14			# EPC <= task_entry[task_id] 

	li	$16,	0x0000FF13		
	mtc0	$16,	$12			# SR <= 0x0000FF13

	# Jump in user mode to address contained in EPC
to_user:
	eret

# Exit with an error message if pid is too large
pid_stop:	
	la	$27,	_tty_write		# $27 <= adresse _tty_write
        ori	$4,     $0,     0       	# $4 <= 0 (tty_index = 0)
        la      $5,     pid_message		# $5 <= message address
        jalr	$27              		# print message
        ori     $6,     $0,	48             	# $6 <= message length
pid_dead:	
	j	pid_dead			# suicide

# Exit with an error message if task_id is illegal
task_id_stop:	
	la	$27,	_tty_write		# $27 <= adresse _tty_write
        ori	$4,     $0,     0       	# $4 <= 0 pid
        la      $5,     task_id_message		# $5 <= message address
        jalr	$27              		# print message
        ori     $6,     $0,	48             	# $6 <= message length
task_id_dead:	
	j	task_id_dead			# suicide

task_id_message:
	.asciiz	"\n!!! task number should be 1, 2, 3, or 4  !!!\n"
pid_message:
	.asciiz	"\n!!! processor id cannot be larger than 3 !!!\n"

	.align	2

	# Default initialization of the tasks_entry_point[] array 

tasks_entry_point:		# 16 tasks entry points
	.space	64

icu_masks_array:		# mask for the IRQ routing : indexed by pid
	.word	0b00000000000011110001000100000001	# ICU_MASK[0]
	.word	0b00000000111100000010001000000000  # ICU_MASK[1]
	.word	0b00001111000000000100010000000000  # ICU_MASK[2]
	.word	0b11110000000000001000100000000000  # ICU_MASK[3]

	.end	reset
