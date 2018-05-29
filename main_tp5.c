#include "config.h"
#include "stdio.h"

#define NPIXEL 256
#define NLINE  256

//////////////////////////////////////////////////////////////////////
//	build function
//////////////////////////////////////////////////////////////////////
unsigned char build(unsigned int x, unsigned int y, unsigned int step)
{
    if( ((x>>step & 0x1) && !(y>>step & 0x1)) ||
        (!(x>>step & 0x1) && (y>>step & 0x1)) ) 	return 0xFF; 
    else 						return 0; 
} // end build

//////////////////////////////////////////////////////////////////////
//	main function
/////////////////////////////////////////////////////////////////////
__attribute__ ((constructor)) void main() 
{
    unsigned char 	buf[NPIXEL];
    int 		n = procid();
    int			nprocs = NB_PROCS;
    unsigned int 	line;
    unsigned int 	pixel;

    for(line = 0 ; line < NLINE ; line++) 
    {   
	    if( n == line % nprocs) {
            for(pixel = 0 ; pixel < NPIXEL ; pixel++)
            {
                buf[pixel] = build(pixel, line, 5);
            }
            if ( fb_sync_write(line*NPIXEL, buf, NPIXEL) ) /*TO BE COMPLETED*/
                tty_printf(" !!! wrong transfer to frame buffer for line %d\n", line); 
            else
                tty_printf(" - building line %d, %d\n", line,n);
	    }
    }

    tty_printf("\ncycles = %d\n", proctime() );
    exit(); 

} // end main
