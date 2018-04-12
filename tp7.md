
# TP7 : Contrôleur DMA[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#TP7:Contr%C3%B4leurDMA "Link to this section")
Songlin Li : 3770906 Hongbo Jiang: 3602103
## B) Contrôleur DMA[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#BContr%C3%B4leurDMA "Link to this section")

Le composant  _PibusDma_  est capable d’adresser directement la mémoire (en lecture et en écriture), pour déplacer des données d’une zone de la mémoire vers une autre. Le coprocessseur DMA se comporte donc comme une cible, puisqu'il doit être configuré par le programme qui souhaite effectuer le transfert, mais il se conduit également comme un maître, puisqu’il est capable d'initier des transactions sur le bus pour lire ou écrire en mémoire.

![](https://www-asim.lip6.fr/trac/sesi-multi/raw-attachment/wiki/MultiCourseTP7/tp7_topcell_dma.png)

On aura donc deux maîtres pouvant travailler en parallèle dans cette architecture : le processeur MIPS32 et le coprocesseur DMA. Nous souhaitons analyser les mécanismes - matériels et logiciels - permettant la coopération entre les deux processus parallèles que sont le programme qui s'exécute sur le processeur, et le coprocesseur DMA qui effectue le transfert. Le mécanisme général est le suivant :

-   Le programme qui s'exécute sur le processeur MIPS32 configure le coprocesseur DMA et lance le transfert en écrivant dans différents registres « mappés en mémoire » les paramètres du transfert : l’adresse de base du tampon source, l’adresse de base du tampon destination, et enfin le nombre de mots à transférer. Ceci fait, le programme qui a commandé le transfert continue son exécution.
-   Le coprocesseur DMA effectue le transfert en construisant des rafales de longueur fixe. Il exécute autant de paires de transactions que nécessaire: chaque paire est constituée par une lecture rafale d'un paquet dans le tampon source, suivie par une écriture rafale de ce paquet vers le tampon destination. La longueur de la rafale est définie par la capacité de stockage interne du coprocesseur DMA. On parle de coprocesseur, car le contrôleur DMA et le processeur travaillent en parallèle durant ce tranfert.
-   La durée du transfert est très variable, car elle dépend à la fois de la charge du bus et du nombre d'octets à transférer. Lorsque le transfert est terminé, le contrôleur DMA le signale au système d'explotation en activant une interruption. cette signalisation de fin de transfert est indispensable, pour permettre au programme ayant déclenché le transfert de re-utiliser les tampons mémoire concernés (source et destination).

Lisez la spécification fonctionnelle du composant  _PibusDma_  que vous trouverez dans l’en-tête du fichier  **pibus_dma.h**  pour répondre aux questions suivantes.

**Question B1 :**  Quels sont les registres adressables du contrôleur DMA, et quel est l'effet d'une lectures ou d'une écriture dans chacun de ces registres? Pourquoi l’adresse de base du segment associé au contrôleur DMA en tant que cible doit-elle être alignée sur une frontière de bloc de 32 octets ?

This component is a DMA controler with a PIBUS interface, acting both as a master and a target on the system bus.
As a target the DMA controler contains  5 memory mapped registers, taking 32 bytes in the address space. (only the 5 less significant bits of the VCI address are decoded)
- SOURCE  (0x00)  Read/Write  Source buffer base address
- DEST  (0x04)  Read/Write  Destination buffer base address
- LENGTH/STATUS  (0x08)  Read/Write  Transfer length (bytes) / Status
- RESET  (0x0C)  Write Only  Software reset & IRQ acknowledge -------r_stop
- IRQ_DISABLED  (0x10)  Read/Write  IRQ disabled when non zeo

A write access to register LENGTH/STATUS starts the DMA transfer,
with the addresses contained in registers SOURCE and DEST,
A read access to register LENGTH/STATUS will return the DMA status,
that is actually the master FSM state.
The relevant values for the status are :
\- DMA_SUCCESS (0)
\- DMA\_READ\_ERROR (1)
\- DMA\_WRITE\_ERROR (2)
\- DMA_IDLE (3)
\- DMA_RUNNING  (>3)

The source and destination addresses must be word aligned,
and the transfer length must be a multiple of 4 bytes.
A write access to the SOURCE, DEST or NWORDS registers is
ignored if the DMA master FSM is not IDLE.
Writing in register RESET stops gracefully an ongoing transfer
and forces the master FSM in IDLE state.
The DMA controller asserts an IRQ when the transfer is completed
(states SUCCESS, READ\_ERROR, WRITE\_ERROR). The IRQ is not asserted
if the IRQ_DISABLED register contains a non-zero value.
Writing in the RESET register is the normal way to acknowledge IRQ.
The initiator FSM uses an internal buffer to store a burst.

**Question B2 :**  Quelle est la signification de l'argument  _burst_  du constructeur du composant  _PibusDma_  ?
This component has 4 "constructor" parameters :
- sc\_module\_name name  : instance name
- unsigned int  tgtid  : target index
- PibusSegmentTable segtab  : segment table
- unsigned int  burst  : **max number of words per burst**


**Question B3 :**  Pourquoi faut-il deux automates (MASTER\_FSM et TARGET\_FSM) pour contrôler le coprocessseur DMA?

Parce que le DMA peut être un Target de CPU et aussi un Master de memoire **en parallèle** (e.g.  le DMA  reçu une nouvelle commande quand il est en train de travailler).



**Question B4 :**  Ce composant matériel contient évidemment d'autres registres que les 5 registres adressables. En analysant le modèle SystemC contenu dans le fichier  **pibus_dma.cpp**, décrivez précisément la fonction de la bascule  _r_stop_.

Soft Reset : After each burst (read or write), the master FSM test the r_stop flip-flop to stop the ongoing transfer if requested. It goes to the DMA_SUCCESS state when the tranfer is isuccessfully completed, to assert the IRQ signaling the completion. In case of bus error, it goes to the DMA\_WRITE\_ERROR or DMA\_READ\_ERROR state to assert the IRQ signaling the completion.

Target FSM controls


| r\_target\_fsm | r\_source |r\_dest|r\_nwords|r\_stop|
|--|--|--|--|--|
|  | addressable |addressable|addressable|addressable|||



The master FSM controls



|r\_master\_fsm | r\_read\_ptr | r\_write\_ptr | r\_index | r\_count|
|--|--|--|--|--|
| |  |  | | |



others


|r\_irq\_disable| r_max |
|--|--|
| addressable |  |





**Question B5 :**  Complétez le graphe ci-dessous représentant la fonction de transition de l'automate MASTER_FSM du composant  _PibusDma_.

response à la dernière page.

## C) Architecture matérielle[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#CArchitecturemat%C3%A9rielle "Link to this section")

**Question C1:**  Quelle est la longueur par défaut d'une rafale en nombre de mots de 32 bits) ?

16

Quel est l'avantage d'utiliser des grosses rafales?

Diminuer les cycles de mort entre transactions de BUS.

Quelle est la conséquence sur le matériel d'une augmentation de la longueur de la rafale?

Le tempon doit avoir la même taille que le burst. (-DMABURST)

**Question C2:**  Quelle est l'adresse de base du segment associé au périphérique DMA?
0x93000000

Quel est son  _numéro de cible_  pour le composant BCU ? Le périphérique DMA étant aussi un maître sur le bus, il est connecté au composant BCU par les signaux REQ\_DMA et GNT\_DMA.

DMA_INDEX = 6 ( as a target )

Quel est son  _numéro de maître_  pour le BCU?

NPROCS ? ( as a master )

Sur quel port d'entrée du composant ICU est connecté la ligne d'interruption IRQ contrôlée par le DMA?

IRQ_IN[**0**] (signal_irq_dma )

## D) Application logicielle[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#DApplicationlogicielle "Link to this section")

**Question D1 :**  L'appel système  _fb\_sync\_write()_  n'utilise pas le coprocesseur DMA. Quel composant matériel effectue-t-il le transfert des pixels de l'image entre le tampon mémoire dans l'espace utilisateur et la mémoire video (frame buffer) ?

The function implement the transaction byte by byte using a pair of Read/Write.
This blocking function use a memory copy strategy to transfer data from a user buffer to the frame buffer device in kernel space.

Expliquez pour quoi cet appel système est bloquant. La réponse se trouve dans les fichier  **stdio.c**  et  **drivers.c**.

Blocked till ( = Return when ) the transaction is finished.

**Question D2 :**  Compilez et exécutez sur le prototype virtuel cette première application logicielle n'utilisant pas le contrôleur DMA.
Quelle est la durée de construction d'une image (temps de remplissage du buffer) ?

-  1 build  OK at cycle 714252
-  2 build  OK at cycle 1551578

Quelle est la durée d'affichage?

-  1 display  OK at cycle 834694
-  2 display OK at cycle 1671857

1st display 120442.

On utilise généralement le contrôleur DMA lorsqu'on a besoin de transférer de gros volumes de données, comme la copie d'une image d'un tampon mémoire (dans l'espace utilisateur) vers la mémoire graphique (située dans l'espace protégé réservé au système). On utilise les appels système  _fb_write()_  et  _fb_completed()_.

**Question D3 :**  Quelle est la différence entre l'appel système  _fb\_sync\_write()_  et l'appel système  _fb_write()_  ?

The '\_fb\_sync\_write' and '\_fb\_sync\_read' functions use a memcpy strategy to implement the transfer between a data buffer (user space) and the frame buffer (kernel space). They are blocking until completion of the transfer. The '\_fb\_write()', '\_fb\_read()' and '\_fb\_completed()' functions use the DMA coprocessor to transfer data between the user buffer and the frame buffer.

Quite similarly to the block device, these three functions use a polling policy to test the global variables \_dma\_busy\[i\] and detect the transfer completion.  As each processor has its private DMA, there is up to NB_PROCS\_dma\_busy locks, that are indexed by the proc_id. A \_dma\_busy variable is reset by the ISR associated to the DMA device IRQ.

Quelle est l'utilité de l'appel système  _fb_completed()_  ?
This function checks completion of a DMA transfer to or fom the frame buffer.
As it is a blocking call, the processor is stalled until the next interrupt.
Returns 0 if success, > 0 if error.

**Question D4 :**  Quelle est la durée d'affichage d'une image avec le DMA?

-  1 build  OK at cycle 714242
-  2 build  OK at cycle 1444653

Quelle est la durée d'affichage?

-  1 display  OK at cycle 728530
-  2 display OK at cycle 1458651

durée de 1ére display 14288
durée de 1ére display 13998


**Question D5 :**  Quel défaut observez-vous sur le bord gauche de l'image affichée? Expliquez précisément la cause de ce dysfonctionnement.

constructor : left => right.


**Question D6 :**  Comment cette variable est-elle utilisée par les deux appels sytème  _fb_write()_  et  _fb_completed()_? Dans quelle fonction trouve-t-on le code de mise à 1 de la variable  _\_dma\_busy_  ? Dans quelle fonction trouve-t-on le code de mise à 0 ? Dans quel segment est stockée cette variable ?

Quite similarly to the block device, these three functions use a polling policy to test the global variables \_dma\_busy\[i\] and detect the transfer completion.  As each processor has its private DMA, there is up to NB_PROCS\_dma\_busy locks, that are indexed by the proc_id. A \_dma\_busy variable is reset by the ISR associated to the DMA device IRQ.

## E) Pipe-Line logiciel[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#EPipe-Linelogiciel "Link to this section")

-   Durant les périodes impaires (2i+1) :
1.  Construction de l'image (2i+1) dans  BUF1
2.  Affichage de l'image (2i) stockée dans  BUF2  (si i>0)

-   Durant les périodes paires (2i) :
1.  Construction de l'image (2i) dans  BUF2, (si i<3)
2.  Affichage de l'image (2i-1) stockée dans  BUF1

Grâce à l'utilisation alternative des tampons mémoires pairs et impairs, à aucun moment on ne lit et on n'écrit simultanément dans le même tampon.

Re-écrivez un programme  **main_pipe.c**  qui réalise le pipe-line logiciel ci-dessus. Ce programme devra être organisé en trois phases:

-   chargement initial du pipe-line, appelée prologue (période 0)
-   traversée du pipe-line (periodes 1,2,3,4)
-   vidage du pipe-line, appelée épilogue (période 5)

**Question E1**: Il faut synchroniser le pipe-line. Quelle condition doit être testées par le logiciel pour passer de la période (n) à la période (n+1) ?

Durant une période (n), le processeur construit l'image (n) et le DMA affiche l'image (n-1).
Pour passer à la période (n+1) il faut que les deux activités parallèles de la période (n) soient terminées.
Il suffit donc que le logiciel qui construit l'image (n) teste à la fin de la construction
que l'affichage de l'image (n-1)  est terminé, en utilisant la fonction bloquante fb_completed().

**Question E2**: Quel est le gain (en nombre de cycles) apporté par le parallélisme pipe-line, par rapport a une execution séquentielle?

La durée de construction est plus longue donc le coût de afficage est faible.

Comment interprétez-vous ce résultat?


## F) Traitement des erreurs[](https://www-asim.lip6.fr/trac/sesi-multi/wiki/MultiCourseTP7#FTraitementdeserreurs "Link to this section")

On s'intéresse maintenant au traitement et à la signalisation des erreurs.

Le contrôleur DMA est un maître capable d'adresser directement la mémoire (en lecture et en écriture), mais ce maître ne prend aucune initiative. Il ne fait qu'exécuter les transfert qui ont été définis par un programme utilisateur, et le programmeur peut faire des erreurs sur la valeur des adresses des tampons source ou destination.

**Question F1 :**  Pourquoi le système d'exploitation interdit-il que l'adresse du tampon source (dans le cas de l'appel système  _fb_write()_) ou l'adresse du tampon destination (dans le cas de l'appel système  _fb_read()_) appartienne à la zone protégée de l'espace adressable ?
Pourquoi ce type d'erreur doit-il absolument être détecté avant que le contrôleur DMA commence à effectuer le transfert?

Parce que la reason de sécurité. L'OS doit protéger les codes et donées de kernel.

Une autre cause d'erreur est l'utilisation d'adresses qui ne correspondent à aucun segment défini. Cette erreur est détectée au moment où le contrôleur DMA effectue le transfert, et reçoit de la part du contrôleur mémoire une réponse de type  _bus error_, en réponse à sa commande de lecture ou d'écriture. Un processeur programmable tel que le MIPS32 qui reçoit une réponse  _bus error_  se branche au gestionnaire d'exception pour signaler l'erreur et permettre le debug, mais le contrôleur DMA est un automate cablé qui n'a évidemment pas de  _gestionnaire d'exception logiciel_...

**Question F2 :**  Quel est le mécanisme qui permet au contrôleur DMA de signaler ce type erreur au programme utilisateur? Pour répondre à cette question, il faut analyser toute la chaîne de signalisation :

-   comportement du contrôleur DMA qui reçoit une réponse  _bus error_  : dans le fichier  **pibus_dma.cpp**:
- W_ERROR/R_ERROR  (MASTER_FSM)
-  register STATUS => ioc_status ( by ISR ) => analyse by fb_completed() => return Error Number
-
-   code de la routine d'interruption  _\_isr\_dma_  associée au DMA : dans le fichier  **irq_handler.c**
- ISR report the error

-   code de la fonction système  _\_fb\_completed()_  : dans le fichier  **drivers.c**
-  register STATUS => ioc_status ( by ISR ) => analyse by fb_completed() => return Error Number

Pour tester ces mécanismes de signalisation d'erreur, modifiez le programme contenu dans le fichier  **main_dma.c**  pour introduire volontairement une erreur sur l'adresse du buffer passée en argument à l'appel système  _fb_write()_.

**Question F3 :**  Quel appel système signale l'erreur si l'adresse erronée est non définie (par exemple adresse 0x0, qui ne correspond à aucun segment défini)?

Detecter **après** la transaction, signé par _fb_completed()_.

Quel appel système signale l'erreur si l'adresse erronée appartient à la zone protégée (par exemple adresse 0x80000000)?

Detecter **avant** la transaction, signé par _fb_write()_.
