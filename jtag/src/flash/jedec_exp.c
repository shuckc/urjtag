#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <flash/cfi.h>
#include <flash/intel.h>
#include <unistd.h>

#include <flash.h>
#include <bus.h>
#include <bitmask.h>

void jedec_exp_read_id(bus_t *bus, uint32_t adr, uint32_t dmask, 
	uint32_t pata, uint32_t patb, uint32_t dcmd, 
	int det_addroffset, int det_dataoffset, uint32_t det_addrpat)
{
	int locofs;

	det_addrpat <<= det_addroffset;
	printf("     trying with address pattern base %08x:", det_addrpat);
	bus_write(bus, adr +  det_addrpat,     pata);
	bus_write(bus, adr + (det_addrpat>>1), patb);
	bus_write(bus, adr +  det_addrpat,     dcmd);

	for(locofs = 0; locofs <= 2; locofs++)
	{
		printf(" %08x", (dmask & bus_read(bus, adr + (locofs<<det_addroffset))) >> det_dataoffset);
	}

	printf("\n");
}

int
jedec_exp_detect( bus_t *bus, uint32_t adr, cfi_array_t **cfi_array )
{
	/* Temporary containers for manufacturer and device id while
	   probing with different Autoselect methods. */
	int ba, bw;
	int det_buswidth;
	bus_area_t area;
	
	*cfi_array = calloc( 1, sizeof (cfi_array_t) );
	if (!*cfi_array)
		return -2;              /* out of memory */
	
	(*cfi_array)->bus = bus;
	(*cfi_array)->address = adr;
	if (bus_area( bus, adr, &area ) != 0)
		return -8;		/* bus width detection failed */
	bw = area.width;

	if (bw == 0) bw = 32; // autodetection!

	if (bw != 8 && bw != 16 && bw != 32)
		return -3;              /* invalid bus width */
	(*cfi_array)->bus_width = ba = bw / 8;

	(*cfi_array)->cfi_chips = calloc( 1, sizeof (cfi_chip_t *) );
	if (!(*cfi_array)->cfi_chips)
		return -2;              /* out of memory */
	
	(*cfi_array)->cfi_chips[0] = calloc( 1, sizeof (cfi_chip_t) );
	if (!(*cfi_array)->cfi_chips[0])
		return -2;              /* out of memory */

	printf("=== experimental extensive JEDEC brute-force autodetection ===\n");
	for(det_buswidth = bw; det_buswidth >= 8; det_buswidth >>= 1)
	{
		int det_datawidth;
		printf("- trying with cpu buswidth %d\n", det_buswidth);
		for(det_datawidth = det_buswidth; det_datawidth >= 8; det_datawidth >>= 1)
		{
			int det_dataoffset;
  			printf("-- trying with flash datawidth %d\n", det_datawidth);
			for(det_dataoffset = 0; det_dataoffset + det_datawidth <= det_buswidth; det_dataoffset += 8)
			{
				int det_addroffset;
				uint32_t dmask = bits(det_dataoffset, det_datawidth+det_dataoffset-1);
				uint32_t pata = ~dmask | ( 0xAA << det_dataoffset );
				uint32_t patb = ~dmask | ( 0x55 << det_dataoffset );
				uint32_t dcmd = ~dmask | ( 0x90 << det_dataoffset );

	  			printf("--- trying with flash dataoffset %d", det_dataoffset);
				printf(" (using %08X, %08X and %08X)\n", pata, patb, dcmd);

				for(det_addroffset = 0; det_addroffset <= 2; det_addroffset++)
				{
					jedec_exp_read_id(bus,adr,dmask,pata,patb,dcmd,det_addroffset,det_dataoffset,0x5555);
					jedec_exp_read_id(bus,adr,dmask,pata,patb,dcmd,det_addroffset,det_dataoffset,0x0555);
				}
			}
		}
	}
	printf("=== end of experimental extensive JEDEC brute-force autodetection ===\n");
	
	return 1;
}

