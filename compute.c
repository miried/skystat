/*
* compute.c - computations
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
================
COM_ImpLThreshold

decide if impact parameter below threshold
================
*/
int	COM_ImpLThreshold( catdata_t *cdn, catdata_t *cds, double threshold ) {
	double delta;
	double impact;
	double dra;
	double ddec;

	dra = RAD*(cdn->ra - cds->ra);
	ddec = RAD*(cdn->dec - cds->dec);
	if (fabs(ddec)*cds->angDiamD > threshold)
		return 0;

	// comoving impact parameter = AngDiamDist * GreatCircDist
	delta = MAT_GreatCircD( dra, ddec, cdn->cosdec, cds->cosdec );
	impact = delta * cds->angDiamD;

	if ( impact < threshold )
		return 1;
	else
		return 0;
}

/*
================
COM_AngLThreshold

decide if angular distance below threshold
================
*/
int	COM_AngLThreshold( catdata_t *a, catdata_t *b,
				double threshold, double *delta ) {

	double result;
	double dra;
	double ddec;

	dra = a->ra - b->ra;
	ddec = a->dec - b->dec;

	if (fabs(ddec) > threshold)
		return 0;

	result = DEG*MAT_GreatCircD( RAD*dra, RAD*ddec, a->cosdec, b->cosdec );

	if ( result < threshold ) {
		*delta = result;
		return 1;
	}
	else
		return 0;
}

/*
================
COM_MeanRM

Compute mean RM
================
*/
void	COM_MeanRM( const char *cmdLine ) {
	double			threshold;
	int			i;
	catalog_t		*cat;

	// Read parameters
	if ( sscanf( cmdLine, "%lf", &threshold ) != 1 ) {
		threshold	= 2.0;
	}
	printf( "Computing mean RM...\n" );
	printf( "Threshold is %lf deg\n", threshold );

	cat = nvss_culled;

	for (i=0;i<cat->number;i++) {
		int		k;
		catdata_t	*a;
		double		sum;
		int		sourcesFound;

		a = cat->data + i;
		sum = 0.0;
		sourcesFound = 0;

		if ( i%2000 == 0 ) {
			printf( "%i %%\n", (int)(100.0*i/cat->number) );
		}
		// find other sources near
		for ( k=0; k<cat->number; k++ ) {
			catdata_t	*b;
			int		result;
			double		delta;

			// skip the galaxy itself
			if ( k==i )
				continue;
			b = cat->data + k;
			result = COM_AngLThreshold( a,b,threshold,&delta );

			if ( result == 1 ) {
				// found one nearby
				sourcesFound++;
				sum += b->rot_measure;
			}
		}
		// mean
		a->rot_measure_mean = sum / sourcesFound;
		a->rot_measure_delta = a->rot_measure - a->rot_measure_mean;
		a->sourcesNum = sourcesFound;
	}
	printf( "\nDone.\n" );
}

/*
================
COM_SwapNN

swap Nearest Neighbors.
================
*/
void	COM_SwapNN( sortNN_t *a, sortNN_t *b ) {
	sortNN_t tmp;

	memcpy( &tmp,	a,	sizeof(sortNN_t) );
	memcpy( a,	b,	sizeof(sortNN_t) );
	memcpy( b,	&tmp,	sizeof(sortNN_t) );
}

/*
================
COM_SortNN

Sort Nearest Neighbors.
Algorithm: Optimized Bubblesort
================
*/
void	COM_SortNN( sortNN_t *neighbors, int num ) {
	int k;

	k = num;
	do {
		int i, l;

		l = 1;
		for (i=0; i<k-1; ++i) {
			if (neighbors[i].dist > neighbors[i+1].dist) {
				// swap
				COM_SwapNN( neighbors+i, neighbors+i+1 );
				l = i+1;
			}
		}
		k = l;
	} while ( k > 1 );
}

/*
================
COM_MeanRM_NN

Compute mean RM for Nearest Neighbor method
================
*/
void	COM_MeanRM_NN( const char *cmdLine ) {
	int			nn_number;
	int			i;
	catalog_t		*cat;
	double			threshold;
	sortNN_t		*neighbors;
	int			sortSize;

	// Read parameters
	if ( sscanf( cmdLine, "%i", &nn_number ) != 1 ) {
		nn_number	= 20;
	}
	printf( "Computing mean RM nearest neighbor...\n" );
	printf( "NN number is %i.\n", nn_number );

	// we need enough array members to collect all nearby galaxies
	sortSize = nn_number * 15;
	neighbors = malloc( sortSize * sizeof(sortNN_t) );
	cat = nvss_culled;
	// search inside a radius about large enough to find enough
	threshold = 1.5*sqrt( (double)nn_number );

	for (i=0;i<cat->number;i++) {
		int		k;
		catdata_t	*a;
		double		sum;
		int		sourcesFound;

		a = cat->data + i;
		sum = 0.0;
		sourcesFound = 0;

		if ( i%2000 == 0 ) {
			printf( "%i %%\n", (int)(100.0*i/cat->number) );
		}
		// collect all sources nearby
		for ( k=0; k<cat->number; k++ ) {
			catdata_t	*b;
			int		result;
			sortNN_t	*nn;
			double		delta;

			// skip the galaxy itself
			if ( k==i )
				continue;

			b = cat->data + k;
			result = COM_AngLThreshold( a,b,threshold,&delta );

			if ( result == 1 ) {
				// found one nearby
				if ( sourcesFound == sortSize ) {
					printf( "neighbor overrun.\n" );
					free(neighbors);
					return;
				}
				nn = neighbors + sourcesFound;
				nn->dist = delta;
				nn->rot_measure = b->rot_measure;
				sourcesFound++;
			}
		}
		// if found too little, error
		if ( sourcesFound < nn_number ) {
			printf("Not enough neighbors found in annulus. (%i).\n",
					sourcesFound );
			free( neighbors );
			return;
		}

		// Sort them
		COM_SortNN( neighbors, sourcesFound );

		// Get mean of them
		for ( k=0; k<nn_number; k++ )
			sum += neighbors[k].rot_measure;
		a->rot_measure_mean_nn = sum / nn_number;
		a->rot_measure_delta_nn = a->rot_measure-a->rot_measure_mean_nn;

		// median
		if ( nn_number%2 == 1 )
			a->rot_measure_median = neighbors[nn_number/2].rot_measure;
		else
			a->rot_measure_median = (neighbors[nn_number/2].rot_measure
						+neighbors[(nn_number/2)-1].rot_measure)/2;
		a->rot_measure_median_delta = a->rot_measure-a->rot_measure_median;

		// standard deviation
		sum = 0;
		for ( k=0; k<nn_number; k++ )
			sum += (neighbors[k].rot_measure-a->rot_measure_mean_nn)
				*(neighbors[k].rot_measure-a->rot_measure_mean_nn);
		a->rot_measure_sd_nn = sqrt( sum / (nn_number-1) );
	}

	free( neighbors );
	printf( "\nDone.\n" );
}

