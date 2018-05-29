#include "stdio.h"

// Nombre de pixels par ligne
#define NB_PIXELS 128

// Nombre de lignes par image
#define NB_LINES  128

// Nombre de bytes par bloc du contrôleur de disque
#define BLOCK_SIZE 512

// Valeur du seuil
#define THRESHOLD 200

__attribute__ ((constructor)) void main()
{
    char	buf_in[NB_LINES*NB_PIXELS];
    char	buf_out[NB_LINES*NB_PIXELS];
    int		i;
    char	byte;
    int		base = 0;
    int		image = 0;
    int		pid = procid();
    int		nprocs = procnumber();
    int		npixels;		// number of pixels per processor		
    int		nblocks; 		// number of blocks per processor

    if ( (nprocs != 1) &&
         (nprocs != 2) &&
         (nprocs != 4) )
    {
        tty_printf("Le nombre de processeurs doit être egal a  1, 2, 4\n");
        exit();
    }

    if ( pid >= nprocs )
    {
        tty_printf("L'index du processeur %d est plus grand que le nombre de processeurs\n", pid);
    }

    npixels = NB_PIXELS*NB_LINES/nprocs;
    nblocks = npixels/BLOCK_SIZE;

    // main loop
    while(image < 20)
    {
        tty_printf("\n *** image %d au cycle : %d *** \n", image, proctime());

        /* Phase 1 : lecture image sur le disque et transfert vers buf_in */
        if ( ioc_read(base+pid*nblocks, buf_in+pid*npixels, nblocks /*TO BE COMPLETED*/) )
        {
            tty_printf("\n!!! echec ioc_read au cycle : %d !!!\n", proctime()); 
            exit();
        }
        if ( ioc_completed() )
        {
            tty_printf("\n!!! echec ioc_completed au cycle : %d !!!\n", proctime());
            exit();
        }
        tty_printf("- image chargee au cycle = %d \n",proctime());

        /* Phase 2 : transfert de buf_in vers buf_out avec seuillage */
        for(i=pid*npixels ; i<(pid+1)*npixels ; i++) 
        {
            if( buf_in[i] > THRESHOLD ) 	buf_out[i] = 255;
            else				buf_out[i] = buf_in[i];
        } 
        tty_printf("- filtrage termine au cycle = %d \n",proctime());

        /* Phase 3 : transfertcd  de buf_out vers le frame buffer */
        if ( fb_sync_write(0+pid*npixels, buf_out+pid*npixels, npixels/*TO BE COMPLETED*/) )
        { 
            tty_printf("\n!!! echec fb_write au cycle : %d !!!\n", proctime()); 
            exit();
        }
        tty_printf("- image affichee au cycle = %d \n",proctime());

        base  = base + nblocks*nprocs;// number of blocks every image
        image = image + 1;

        tty_getc_irq(&byte);
    
    } // end while

    exit();

} // end main
