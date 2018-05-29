#include "stdio.h"

/////////////////////////////////////////
__attribute__ ((constructor)) void pgcd()
{
    unsigned int opx;
    unsigned int opy;

    tty_printf(" Interactive PGCD \n");

    while (1) 
    {
        tty_printf("\n*******************\n");
        tty_printf("operand X = ");
        tty_getw_irq(&opx);
        tty_printf("\n");
        tty_printf("operand Y = ");
        tty_getw_irq(&opy);
        tty_printf("\n");
        if( (opx == 0) || (opy == 0) ) 
        {
            tty_printf("operands must be larger than 0\n");
        } 
        else 
        {
            while (opx != opy) 
            {
                if(opx > opy)   opx = opx - opy;
                else            opy = opy - opx;
            }
            tty_printf("pgcd      = %d\n", opx);
        }
    }
} // end pgcd

