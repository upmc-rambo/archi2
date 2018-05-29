#include "stdio.h"

// Nombre de pixels par ligne
#define NPIXEL 128

// Nombre de lignes par image
#define NLINE  128

// Nombre de bytes par bloc du contrôleur de disque
#define BLOCK_SIZE 512

// Valeur du seuil
#define THRESHOLD 200

////////////////////////////////////////////
__attribute__ ((constructor)) void display()
{
    char	buf_in[NLINE*NPIXEL];
    char	buf_out[NLINE*NPIXEL];
    int		x;
    int		i;
    int		nblocks = NPIXEL*NLINE/BLOCK_SIZE;
    int		base = 0;

    /* boucle principale */
    while(base < 20*nblocks) {

    tty_printf("\n *** image %d at cycle : %d *** \n", base/nblocks, proctime());

    /* Phase 1 : lecture image sur le disque et transfert vers buf_in */
    x = ioc_read(base, buf_in, nblocks);
    if(x) tty_printf("\n!!! echec ioc_read !!!\n"); 
    x = ioc_completed();
    if(x) tty_printf("\n!!! echec ioc_completed !!!\n");
    tty_printf("image chargee au cycle = %d \n",proctime());

    /* Phase 2 : transfert de buf_in vers buf_out avec seuillage */
    for(i=0 ; i<NLINE*NPIXEL ; i++) 
    {
        if( buf_in[i] > THRESHOLD ) 	buf_out[i] = 255;
        else				buf_out[i] = buf_in[i];
    } 
    tty_printf("filtrage termine au cycle = %d \n",proctime());

    /* Phase 3 : transfert de buf_out vers le frame buffer */
    x = fb_write(0,buf_out,NLINE*NPIXEL); 
    if(x) tty_printf("\n!!! echec fb_write !!!\n");
    x = fb_completed();
    if(x) tty_printf("\n!!! echec fb_completed !!!\n");
    tty_printf("image affichee au cycle = %d \n",proctime());

    base = base + nblocks;
    
} // end while

    exit();

} // end display
