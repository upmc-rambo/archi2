#include "stdio.h"

#define DEPTH		8 
#define NMAX 		50

/******************/
typedef struct lock
{
    unsigned int current;    // current slot index ---------0<=current<=depth
    unsigned int free;       // next free slot index
} lock_t;

/******************/
typedef struct fifo 
{
    int	    buf[DEPTH];
    int	    ptr;
    int	    ptw;
    int	    sts;  //
    int     depth;
    lock_t  lock;
} fifo_t;

volatile fifo_t fifo = { {} , 0 , 0 , 0 , DEPTH };

/************************************************/
unsigned int atomic_increment( unsigned int*  ptr,	// return ptr+incr
                               unsigned int   increment )
{
    unsigned int value;

    asm volatile (
        "atomic_start:                    \n"
        "move  $10,     %1                \n"
        "move  $11,     %2                \n"
        "ll    $12,     0($10)            \n"
        "addu  $13,     $11,     $12      \n"
        "sc    $13,     0($10)            \n"
        "beqz  $13,     atomic_start      \n"
        "move  %0,      $12               \n"
        : "=r" (value)
        : "r" (ptr) , "r" (increment)
        : "$10" , "$11" , "$12" , "$13" , "memory" );

    return value; 
}

/*************LOCK******************/
void lock_acquire( lock_t* plock ) //	    while(free + 1 != current ){}
{
    // get next free slot index
    unsigned int  ticket = atomic_increment ( &plock->free , 1 );

    // get address of the current slot index
    unsigned int pcurrent = (unsigned int)(&plock->current);

    // poll current slot index until current == ticket
    asm volatile (
        "lock_try:                        \n"
        "lw    $10,     0(%0)             \n"
        "move  $11,     %1                \n"
        "bne   $10,     $11,     lock_try \n"
        :
        : "r" (pcurrent) , "r" (ticket )
        : "$10" , "$11" );
}

/*************UNLOCK******************/
void lock_release( lock_t* plock )
{
    asm volatile ( "sync" );

    plock->current = plock->current + 1;
}

/*************************************/
void fifo_write(fifo_t* fifo, int* val)
{
    int done = 0;

    while(done == 0)
    {
        lock_acquire( (lock_t*)(&fifo->lock) );

        if(fifo->sts == fifo->depth) 
        {
            lock_release( (lock_t*)(&fifo->lock) );  // fifo is full
        }
        else
        {
            //TO BE COMPLETED
            fifo->buf[fifo->ptw] = *val; // write val to fifo
			fifo->ptw = (fifo->ptw + 1) % DEPTH;
			fifo->sts++;
			lock_release( (lock_t*)(&fifo->lock) );
			done = 1;
        }
    }
}
/************************************/
void fifo_read(fifo_t* fifo, int* val)
{
    int done = 0;

    while(done == 0)
    {
        lock_acquire( (lock_t*)(&fifo->lock) );

        if(fifo->sts == 0) 
        {
            lock_release( (lock_t*)(&fifo->lock) ); // fifo is empty
        }
        else
        {
            //TO BE COMPLETED
			*val = fifo->buf[fifo->ptr]; // write val to fifo
			fifo->ptr = (fifo->ptr + 1) % DEPTH;
			fifo->sts--;
			lock_release( (lock_t*)(&fifo->lock) );
			done = 1;
        }
    }
}

/********************************************/
__attribute__ ((constructor)) void producer()
{
    int n;
    int x;
    int tempo = 0;
    int val;

    tty_printf("*** Starting task producer on processor %d ***\n\n", procid());

    for(n = 0 ; n < NMAX ; n++) 
    { 
        tempo = rand()>>6;
        val = n;
        fifo_write((fifo_t*)&fifo, &val);
        for(x = 0 ; x < tempo ; x++) asm volatile ("");
        tty_printf("transmitted value : %d      temporisation = %d\n", val, tempo);
    }

    tty_printf("\n*** Completing producer at cycle %d ***\n", proctime());
    exit();

} // end producer()

/*******************************************/
__attribute__ ((constructor)) void consumer()
{
    int n;
    int x;
    int tempo = 0;
    int val;

    tty_printf("*** Starting task consumer on processor %d ***\n\n", procid());

    for(n = 0 ; n < NMAX ; n++) 
    { 
        tempo = rand()>>6;
        fifo_read((fifo_t*)&fifo, &val);
        for(x = 0 ; x < tempo ; x++) asm volatile ("");
        tty_printf("received value : %d      temporisation = %d\n", val, tempo);
    }

    tty_printf("\n*** Completing consumer at cycle %d ***\n", proctime());
    exit();

} // end consumer()

