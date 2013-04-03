/*
* skyplot.h - skyplot header
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gnuplot_i.h"

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* GLOBAL CONSTANTS
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
// Program-specific
#define	PROJECTNAME		"skyplot"

// Calculation constants
#define	MOLLWEIDE_ERROR		0.0000001

// Mathematical Constants
#define	RAD			M_PI/180.0
#define	DEG			180.0/M_PI

// Data input
#define	NVSS_LINE_LEN		145

// Cosmological parameters
#define	OMEGA_M			0.272
#define	OMEGA_L			0.734
#define	HUBBLE_0		71.0

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* STRUCTURES, ENUMS
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
typedef enum cattype_s {
	CT_SDSS,
	CT_NVSS
} cattype_t;

typedef struct catdata_s {
	double	ra;
	double	dec;
	double	z;
	double	abs_petro_r_mag;
	double	u_b_color;
	double	stellar_mass;

	// Galactic coordinates for NVSS
	double	longitude;
	double	latitude;

	// Rotation Measure
	double	rot_measure;
	double	rot_measure_mean;	// mean RM in neighbourhood
	double	rot_measure_delta;	// delta of RM to mean RM
	int	sourcesNum;		// number of sources inside annulus
	double	rot_measure_mean_nn;	// mean for Nearest neighbor
	double	rot_measure_delta_nn;	// delta for NN
	double	rot_measure_sd_nn;	// standard deviation with NN
	double	rot_measure_median;
	double	rot_measure_median_delta;

	double	comovD;		// in GPc
	double	angDiamD;

	double	mollw_angle;
	double	mollw_angle_gal;
	double	cosdec;			// cos(dec)
} catdata_t;

typedef struct catalog_s {
	cattype_t	type;
	int		number;
	double		threshold; // for selected NVSS
	catdata_t	*data;	// pointer size may vary (32/64 bits)
} catalog_t;

typedef struct cullthread_s {
	catalog_t	*from;
	catalog_t	*toA;
	catalog_t	*toB;

	int		start;
	int		end;
	double		threshold;
	time_t		tic;
} threadData_t;

typedef struct sortNN_s {
	double		dist;
	double		rot_measure;
} sortNN_t;

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* EXTERNAL VARIABLES
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

extern int		scripted;

// full catalogs
extern catalog_t	nvss_full;
extern catalog_t	sdss_full;

// pointers to the active buffers
extern catalog_t	*sdss_culled;
extern catalog_t	*nvss_culled;

extern catalog_t	sdss_A;
extern catalog_t	sdss_B;

extern catalog_t	nvss_A;
extern catalog_t	nvss_B;

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* FUNCTION DECLARATIONS
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
// skyplot.c
int		main( int argc, char **argv );
void		SKY_ScriptLoop( const char *fileName );
void		SKY_CmdLoop( void );

// compute.c
int		COM_ImpLThreshold( catdata_t *cdn, catdata_t *cds,
							double threshold );
void		COM_MeanRM( const char *cmdLine );
void		COM_MeanRM_NN( const char *cmdLine );

// culling.c
int		CUL_CullCancel( const char *cmdLine );

// fileio.c
int		FIO_OpenDataFile( FILE **fp, const char *fs, catalog_t *cat );
int		FIO_ReadCatFile( FILE *fp, catalog_t *cat );
void		FIO_CloseFile( FILE *fp );
void		FIO_ReadDistFile( FILE *fp, catalog_t *cat );

int		FIO_FileToMemory( const char *name, catalog_t *cat );
void		FIO_MemoryToFile( const char *name, catalog_t *cat );
void		FIO_DataToFile( const char *name, double *data, int number );

int		FIO_OpenScriptFile( const char *name );
int		FIO_ReadScript( char *cmdLine, int length );
void		FIO_CloseScriptFile( void );

// math.c
double		MAT_GreatCircD(double dra, double ddec,
				double cosdec1, double cosdec2);
double		MAT_AngDiamD( double z, double comovD );
void		MAT_Mollweide( double dec, double *result );

// memory.c
void		MEM_AppendCat( catalog_t *old, catalog_t *new, int offs );
void		MEM_SwitchSDSSBuffer( void );
void		MEM_SwitchNVSSBuffer( void );
void		MEM_LoadCulled( void );
void		MEM_SaveCulled( void );
void		MEM_ResetCulled( void );
void		MEM_Init( void );
void		MEM_FreeAllBuffers( void );

// statistics.c
int		STAT_DivideCancel( const char *cmdLine );
void		STAT_WriteToDisk( const char *cmdLine );

// visual.c
void		VIS_DrawPlot( const char *buf );

