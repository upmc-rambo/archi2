#include "stdio.h"

#define TRUE	1
#define FALSE	0
#define MAX	1000

//////////
__attribute__ ((constructor)) void main_prime()
{
    unsigned    prime[MAX];
    unsigned    tested_value = 2;
    unsigned    next_empty_slot = 0;
    unsigned    is_prime;
    unsigned    i;

    tty_printf("*** Starting Prime Computation ***\n\n");

    while (1) 
    {
        is_prime = TRUE;
        for( i=0 ; i<next_empty_slot ; i++ ) 
        { 
            if( tested_value%prime[i] == 0 ) is_prime = FALSE; 
        }
        if ( is_prime ) 
        {
            prime[next_empty_slot] = tested_value;
            tty_printf("prime[%d] = %d\n", next_empty_slot, tested_value);
            next_empty_slot++;
            if( next_empty_slot == MAX) 
            {
                tty_printf("prime table full\n");
                exit();
            }
        }
        tested_value++;
    }
} // end main
