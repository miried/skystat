/*
* fileio.c - file input/output code
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

FILE	*script_file;

/*
================
FIO_OpenDataFile

Open file handle and count number of lines
================
*/
int	FIO_OpenDataFile( FILE **fp, const char *fs, catalog_t *cat ) {
	int c;
	int lines;

	// Open File
	*fp = fopen( fs, "rt" );

	if ( *fp == NULL ) {
		printf( "File \"%s\" could not be opened.\n", fs );
		cat->number = 0;
		return 0;
	}
	// Go to Start of File
	rewind( *fp );
	printf( "File \"%s\" opened.\n", fs );

	// Count number of input lines
	lines = 0;
	do {
		c = fgetc( *fp );
		if (c == '\n') lines++;
	} while (c != EOF);

	if ( cat  != NULL )
		cat->number = lines;

	return lines;
}

/*
================
FIO_ReadDistFile

read ASCII distances
================
*/
void	FIO_ReadDistFile( FILE *fp, catalog_t *cat ) {
	int i;
	double cD;
	catdata_t *current;

	rewind( fp );
	for ( i=0; i<cat->number; i++ ) {
		current = cat->data + i;

		if (fscanf( fp, "%lf", &cD ) != 1)
			printf( "comovD read fail.\n" );
		else {
			// TODO: calculate list in Kiloparsec
			cD *= 1000; // from GPc to MPc
			cD *= 1000; // from Mpc to Kpc
			current->comovD = cD;
			current->angDiamD = MAT_AngDiamD(current->z,cD);
		}
	}
}

/*
================
FIO_ReadCatFile

read catalog ASCII data file
================
*/
int	FIO_ReadCatFile( FILE *fp, catalog_t *cat ) {
	int i;
	catdata_t *current;

	rewind(fp);
	switch (cat->type ) {
		char linebuffer[NVSS_LINE_LEN];

		case CT_SDSS:
		for ( i=0; i<cat->number; i++ ) {
			current = cat->data + i;

			if (fscanf( fp, "%lf %lf %lf %lf %lf %lf",&current->ra,
						&current->dec,
						&current->z,
						&current->abs_petro_r_mag,
						&current->u_b_color,
						&current->stellar_mass ) !=6 )
				{
					printf("error reading SDSS data, line %i.\n",
									i);
					return 0;
				}
		}
		break;

		case CT_NVSS:
		for ( i=0; i<cat->number; i++ ) {
			double	hour, min, sec;

			current = cat->data + i;

			if (fread( linebuffer, sizeof(linebuffer), 1, fp )==0) {
				printf( "error reading NVSS data.\n" );
				return 0;
			}
			// Read and convert RA to degs
			sscanf( linebuffer + 0, "%lf %lf %lf",
								&hour,
								&min,
								&sec );

			current->ra = (360/24)*(hour + min/60.0 + sec/3600.0);
			// same for DEC
			sscanf( linebuffer + 21, "%lf %lf %lf",
								&hour,
								&min,
								&sec );
			current->dec = hour + min/60.0 + sec/3600.0;

			// Galactic coordinates
			sscanf( linebuffer + 44, "%lf %lf",
							&current->longitude,
							&current->latitude );
			// rotation measure
			sscanf( linebuffer + 125, "%lf",
							&current->rot_measure );
		}
		break;
	}
	return 1;
}

/*
================
FIO_CloseFile

Close file handles
================
*/
void	FIO_CloseFile( FILE *fp ) {

	fclose( fp );
}

/*
================
FIO_FileToMemory

Read data precompiled data array from file
================
*/
int	FIO_FileToMemory( const char *name, catalog_t *cat ) {
	FILE	*fp;

	fp = fopen( name, "rb" );

	if ( fp == NULL ) {
		printf( "%s not found.\n", name );
		return 0;
	}

	if (fread( cat, sizeof(catalog_t), 1, fp )==0) {
		printf( "error reading %s.\n", name );
		return 0;
	}
	// allocate data buffer
	cat->data = malloc( cat->number * sizeof(catdata_t) );
	if (fread( cat->data, sizeof(catdata_t)*cat->number, 1, fp )==0) {
		printf("error reading data of %s. n is %i\n",name,cat->number);
		perror( "skyplot" );
		return 0;
	}

	printf( "loaded %i sources from %s.\n", cat->number, name );
	fclose( fp );

	return 1;
}

/*
================
FIO_MemoryToFile

Save data array to a file
================
*/
void	FIO_MemoryToFile( const char *name, catalog_t *cat ) {
	FILE	*fp;

	fp = fopen( name, "wb" );

	if ( fp == NULL ) {
		printf( "%s not savable.\n", name );
		return;
	}

	fwrite( cat, sizeof(catalog_t), 1, fp );
	fwrite( cat->data, sizeof(catdata_t)*cat->number, 1, fp );

	fclose( fp );
}

/*
================
FIO_MemoryToFile

Save data array to a file
================
*/
void	FIO_DataToFile( const char *name, double *data, int number ) {
	FILE	*fp;
	int	i;

	fp = fopen( name, "wt" );

	if ( fp == NULL ) {
		printf( "%s not savable.\n", name );
		return;
	}

	for ( i=0 ; i<number; i++ ) {
		fprintf( fp, "%lf\n", *(data+i) );
	}

	fclose( fp );
}

/*
================
FIO_MemoryToFile

Save data array to a file
================
*/
int	FIO_OpenScriptFile( const char *name ) {

	script_file = fopen( name, "rt" );

	if ( script_file == NULL ) {
		printf( "script file %s could not be opened.\n", name );
		return 0;
	}

	return 1;
}

/*
================
FIO_ReadScript

Save data array to a file
================
*/
int	FIO_ReadScript( char *cmdLine, int length ) {

	if ( fgets(cmdLine,length,script_file) == NULL )
		return 0;
	if ( cmdLine[0] == '\n' )
		return 2;

	return 1;
}

/*
================
FIO_CloseScriptFile

Close script fp
================
*/
void	FIO_CloseScriptFile( void ) {

	FIO_CloseFile( script_file );
}

