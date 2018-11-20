/* arpInit.c - arpInit */

#include <xinu.h>
#include "arp.h"

struct arpentry arptab[NARP];
int arplock;

/*------------------------------------------------------------------------
 * arpInit - initialize ARP structures
 *------------------------------------------------------------------------
 */
void arpInit(void)
{
	bzero((void *)arptab, NARP * sizeof(struct arpentry));
	arplock = semcreate(1);
}
