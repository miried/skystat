/*
* memory.c - memory and data management
*
* Copyright (C) 2012 Michael Rieder <mr@student.ethz.ch>
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or (at
* your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#include "skyplot.h"

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* VARIABLES
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

FILE		*fp;

// full catalogs
catalog_t	nvss_full;
catalog_t	sdss_full;

// pointers to the active buffers
catalog_t	*sdss_culled;
catalog_t	*nvss_culled;

// two buffers to switch
catalog_t	sdss_culled1;
catalog_t	nvss_culled1;

catalog_t	sdss_culled2;
catalog_t	nvss_culled2;

// selection for statistical test
catalog_t	sdss_A;
catalog_t	sdss_B;

catalog_t	nvss_A;
catalog_t	nvss_B;


/*
================
MEM_AllocCatData

Allocate memory for cat data
================
*/
catdata_t*	MEM_AllocCatData( int n ) {
	catdata_t *ptr;

	ptr = calloc( n, sizeof(catdata_t) );
	return ptr;
}

/*
================
MEM_CopyCat

Copy catalog
================
*/
void	MEM_CopyCat( catalog_t *from, catalog_t *to ) {
	int size;
	catdata_t *dat;

	// copy cat data
	size = from->number * sizeof(catdata_t);
	memcpy( to->data, from->data, size );
	// temporarily store data pointer
	dat = to->data;
	// copy whole catalog
	memcpy( to, from, sizeof(catalog_t) );
	// restore pointer
	to->data = dat;
}

/*
================
MEM_CloneCatA

clone catalog type and allocate buffer
================
*/
void	MEM_CloneCatA( catalog_t *old, catalog_t *new ) {

	new->data = MEM_AllocCatData( old->number );
	MEM_CopyCat( old, new );
}

/*
================
MEM_CloneCatB

clone catalog type and allocate buffer + copy data
================
*/
void	MEM_CloneCatB( catalog_t *old, catalog_t *new ) {

	new->data = MEM_AllocCatData( old->number );
	new->type = old->type;
}

/*
================
MEM_AppendCat

Append cat data from old catalog at offset to new catalog
================
*/
void	MEM_AppendCat( catalog_t *old, catalog_t *new, int offs ) {

	memcpy( new->data + new->number, old->data + offs, sizeof(catdata_t) );
	new->number++;
}

/*
================
MEM_SwitchSDSSBuffer

Switch SDSS buffers
================
*/
void	MEM_SwitchSDSSBuffer( void ) {

	if ( sdss_culled == &sdss_culled1 )
		sdss_culled = &sdss_culled2;
	else
		sdss_culled = &sdss_culled1;
}

/*
================
MEM_SwitchNVSSBuffer

Switch NVSS buffers
================
*/
void	MEM_SwitchNVSSBuffer( void ) {

	if ( nvss_culled == &nvss_culled1 )
		nvss_culled = &nvss_culled2;
	else
		nvss_culled = &nvss_culled1;
}

/*
================
MEM_GrabCatFile

Grab sky data from ASCII catalog and close fp
================
*/
void	MEM_GrabCatFile( catalog_t *cat ) {

	switch ( cat->type ) {
		case CT_SDSS:
		FIO_OpenDataFile( &fp, "./data/SDSS_galaxies.dat", cat );
		break;
		case CT_NVSS:
		FIO_OpenDataFile( &fp, "./data/RMCatalogue.txt", cat );
		break;
	}

	printf( "found %i sources.\n", cat->number );
	
	if ( cat->number == 0 )
		return;

	// allocate buffer for data;
	cat->data = MEM_AllocCatData( cat->number );
	if ( cat->data ) {
		printf( "allocated %i cat sets for data.\n", cat->number );
		FIO_ReadCatFile( fp, cat );
	}
	else
		printf("allocation failed for %i cat data sets.\n",cat->number);

	FIO_CloseFile( fp );

	printf( "--------------------------------\n" );
}

/*
================
MEM_ProcessCatData

Calculate Mollweide positions
================
*/
void	MEM_ProcessCatData( catalog_t *cat ) {
	int i;

	// Go through the catalog
	for ( i=0; i< cat->number; i++ ) {
		catdata_t	*cd;

		cd = cat->data + i;
		// calculate mollweide angle
		MAT_Mollweide( cd->dec, &cd->mollw_angle );
		// calculate cos(dec)
		cd->cosdec = cos(RAD*cd->dec);
	}
	// Mollweide angle in Galactic coordinates
	if ( cat->type == CT_NVSS ) {
		for ( i=0; i< cat->number; i++ ) {
			catdata_t	*cd;

			cd = cat->data + i;
			// calculate Galactic mollweide angle
			MAT_Mollweide( cd->latitude, &cd->mollw_angle_gal );
		}
	}
}

/*
================
MEM_LoadCulled

load previously culled catalogs
================
*/
void	MEM_LoadCulled( void ) {
	int err;

	// see if there is already a culled SDSS catalog
	err = FIO_FileToMemory("/dev/shm/skyplot/SDSS_culled.dat",sdss_culled);
	if ( err == 0 ) {
		printf( "loading culled SDSS failed.\n" );
		return;
	}

	// also see if there is already a culled NVSS catalog
	err = FIO_FileToMemory("/dev/shm/skyplot/NVSS_culled.dat",nvss_culled);
	if ( err == 0 ) {
		printf( "loading culled NVSS failed.\n" );
		return;
	}

	printf( "successfully loaded: %i NVSS, %i SDSS.\n",
				nvss_culled->number, sdss_culled->number );
	printf( "NVSS threshold is %lf Kiloparsecs.\n",nvss_culled->threshold );
}

/*
================
MEM_SaveCulled

save culled catalogs for later
================
*/
void	MEM_SaveCulled( void ) {

	FIO_MemoryToFile( "/dev/shm/skyplot/SDSS_culled.dat", sdss_culled );
	FIO_MemoryToFile( "/dev/shm/skyplot/NVSS_culled.dat", nvss_culled );

	printf( "saved.\n");
}

/*
================
MEM_ResetCulled

reset culled catalog
================
*/
void	MEM_ResetCulled( void ) {

	MEM_CopyCat( &sdss_full, sdss_culled );
	MEM_CopyCat( &nvss_full, nvss_culled );

	printf( "catalogs reset.\n" );
}

/*
================
MEM_Init

Initialize main data
================
*/
void	MEM_Init( void ) {

	mkdir( "/dev/shm/skyplot", 0777 );

	sdss_culled = &sdss_culled1;
	nvss_culled = &nvss_culled1;

	// Try to read previous data file from RAM
	if ( FIO_FileToMemory("/dev/shm/skyplot/NVSS.dat", &nvss_full) == 0 ) {
		// Read data from hard disk
		nvss_full.type = CT_NVSS;
		MEM_GrabCatFile( &nvss_full );
		MEM_ProcessCatData( &nvss_full );

		// Save data file to RAM
		FIO_MemoryToFile( "/dev/shm/skyplot/NVSS.dat", &nvss_full );
	}

	// Try to read previous data file from RAM
	if ( FIO_FileToMemory("/dev/shm/skyplot/SDSS.dat", &sdss_full) == 0 ) {
		int lines;
		// Read data from hard disk
		sdss_full.type = CT_SDSS;
		MEM_GrabCatFile( &sdss_full );
		MEM_ProcessCatData( &sdss_full );

		// workaround for separate distance data
		lines = FIO_OpenDataFile( &fp, "./data/comovD.dat", NULL );
		if ( lines != sdss_full.number )
			printf( "SDSS line mismatch.\n" );
		FIO_ReadDistFile( fp, &sdss_full );
		FIO_CloseFile( fp );

		// Save data file to RAM
		FIO_MemoryToFile( "/dev/shm/skyplot/SDSS.dat", &sdss_full );
	}

	// allocate space for primary buffer and copy full catalog
	MEM_CloneCatA( &sdss_full, &sdss_culled1 );
	MEM_CloneCatA( &nvss_full, &nvss_culled1 );

	// allocate space for seconday buffers too
	MEM_CloneCatB( &sdss_full, &sdss_culled2 );
	MEM_CloneCatB( &nvss_full, &nvss_culled2 );
	// and for statistical probes
	MEM_CloneCatB( &sdss_full, &sdss_A );
	MEM_CloneCatB( &sdss_full, &sdss_B );
	MEM_CloneCatB( &nvss_full, &nvss_A );
	MEM_CloneCatB( &nvss_full, &nvss_B );

	printf( "----------------------------------------" );
	printf( "----------------------------------------\n" );
}

/*
================
MEM_FreeDataBuffer

Free an allocated data buffer
================
*/
void	MEM_FreeDataBuffer( catalog_t *cat ) {

	free( cat->data );
}

/*
================
MEM_FreeAllBuffers

Free all allocated data buffers
================
*/
void	MEM_FreeAllBuffers( void ) {

	MEM_FreeDataBuffer( &nvss_full );
	MEM_FreeDataBuffer( &sdss_full );
	MEM_FreeDataBuffer( &nvss_culled1 );
	MEM_FreeDataBuffer( &sdss_culled1 );
	MEM_FreeDataBuffer( &nvss_culled2 );
	MEM_FreeDataBuffer( &sdss_culled2 );
	MEM_FreeDataBuffer( &sdss_A );
	MEM_FreeDataBuffer( &sdss_B );
	MEM_FreeDataBuffer( &nvss_A );
	MEM_FreeDataBuffer( &nvss_B );
}

