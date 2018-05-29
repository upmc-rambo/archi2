#include "stdio.h"

#define DEPTH		4
#define NMAX 		100

typedef struct fifo {
    int	buf[DEPTH];
    int	ptr;
    int	ptw;
    int	sts;
    int	lock;
    int depth;
} fifo_t;

volatile fifo_t fifo = { {},0,0,0,0,DEPTH };

/***********************************/
inline void lock_acquire(int* lock)
{
    unsigned int delay = rand()>>8;
    asm volatile (
            "lock_try:              \n"
            "ll   $2,    0(%0)      \n" /* $2 <= lock current value */
            "bnez $2,    lock_delay \n" /* retry if lock already taken */
            "li   $3,    1          \n" /* $3 <= argument for sc */
            "sc   $3,    0(%0)      \n" /* try to get lock */
            "bnez $3,    lock_ok    \n" /* exit if atomic */
            "lock_delay:            \n"
            "move $4,    %1         \n" /* $4 <= delay */
            "lock_loop:             \n"
            "addi $4,    $4,  -1    \n" /* $4 <= $4 - 1 */
            "beqz $4,    lock_loop  \n" /* test end delay */
            "nop                    \n"
            "j           lock_try   \n" /* retry if not atomic */
            "nop                    \n"
            "lock_ok:               \n"
            :
            :"r"(lock), "r"(delay)
            :"$2", "$3", "$4");
}

/***********************************/
inline void lock_release(int* lock)
{
    *lock = 0;
}

/*******************************************/
inline void fifo_write(fifo_t* fifo, int val)
{
    int done = 0;
    while(done == 0)
    {
        lock_acquire((int*)&fifo->lock);
        if(fifo->sts == fifo->depth) 
        {
            lock_release((int*)&fifo->lock);
        }
        else
        {
            fifo->buf[fifo->ptw] = val;
            fifo->ptw = (fifo->ptw+1)%fifo->depth;
            fifo->sts = fifo->sts+1;
            lock_release((int*)&fifo->lock);
            done = 1;
        }
    }
}
/*******************************************/
inline void fifo_read(fifo_t* fifo, int* val)
{
    int done = 0;
    while(done == 0)
    {
        lock_acquire((int*)&fifo->lock);
        if(fifo->sts == 0) 
        {
            lock_release((int*)&fifo->lock);
        }
        else
        {
            *val = fifo->buf[fifo->ptr];
            fifo->ptr = (fifo->ptr+1)%fifo->depth;
            fifo->sts = fifo->sts-1;
            lock_release((int*)&fifo->lock);
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
        fifo_write((fifo_t*)&fifo, val);
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

