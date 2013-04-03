/*
* statistics.c - statistics code
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
* MULTITHREADING STUFF
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

pthread_mutex_t	div_mutex = PTHREAD_MUTEX_INITIALIZER;
int		div_completed;
int		numDivThreads = -1;
int		divThreadsFin = 0;
pthread_t	*dpThreads;
threadData_t	*dThreadData;


/*
================
STAT_WriteToDisk

Write data to disk for stat. analysis
================
*/
void	STAT_WriteToDisk( const char *cmdLine ) {
	int		i;
	char		type;
	char		subType;
	char		dataType1;
	catalog_t	*cat;
	const char	*caption;
	double		*data;
	char		fileName[64];
	char		name[64];

	type	= *cmdLine;
	subType	= *(cmdLine+1);
	dataType1= *(cmdLine+2);
	strncpy( name, cmdLine+4, 64 );
	name[strlen(name)-1] = 0;

	// which catalog to take data from
	switch ( type ) {
		case 'n':
		if ( subType == 'a' ) {
			cat = &nvss_A;
			caption = "NVSSA";
		}
		else if ( subType == 'b' ) {
			cat = &nvss_B;
			caption = "NVSSB";
		}
		else {
			cat = nvss_culled;
			caption = "NVSS";
		}
		break;
		case 's':
		if ( subType == 'a' ) {
			cat = &sdss_A;
			caption = "SDSSA";
		}
		else if ( subType == 'b' ) {
			cat = &sdss_B;
			caption = "SDSSB";
		}
		else {
			cat = sdss_culled;
			caption = "SDSS";
		}
		break;
		default:
			printf( "Unrecognized cat type '%c'.\n", type );
			return;
	}

	data = malloc( cat->number * sizeof(double) );

	// what type of data
	switch ( dataType1 ) {
		case 'z':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->z;
		}
		break;
		case 'n':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->sourcesNum;
		}
		break;
		case 'd':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_delta;
		}
		break;
		case 'e':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_delta_nn;
		}
		break;
		case 'f':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_median_delta;
		}
		break;
		case 'm':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->stellar_mass;
		}
		break;
		case 'u':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->u_b_color;
		}
		break;
		case 'x':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->ra;
		}
		break;
		case 'y':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->dec;
		}
		break;
		case 'a':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->mollw_angle;
		}
		break;
		case 'b':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->mollw_angle_gal;
		}
		break;
		case 'c':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_mean;
		}
		break;
		case 'g':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_mean_nn;
		}
		break;
		case 'h':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure_sd_nn;
		}
		break;
		case 'k':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->longitude;
		}
		break;
		case 'l':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->latitude;
		}
		break;
		case 'r':
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			data[i] = cd->rot_measure;
		}
		break;
		default:
			printf( "Unrecognized data type '%c'.\n", type );
			free( data );
			return;
	}

	sprintf( fileName, "/dev/shm/skyplot/%s-%s.%c",
					name, caption, dataType1 );
	FIO_DataToFile( fileName, data, cat->number );

	printf( "Wrote %i lines of %c to %s.\n",
				cat->number, dataType1, fileName );

	free( data );
}

/*
================
STAT_DivideSDSS

Divide SDSS into A and B
================
*/
void	STAT_DivideSDSS( const char *cmdLine ) {
	int i;
	char type;
	double l, u;
	catalog_t *sdss, *a, *b;

	// Scan selection parameters
	sscanf( cmdLine, "%c %lf %lf", &type, &l, &u );

	sdss = sdss_culled;
	a = &sdss_A;
	b = &sdss_B;

	// clear buffers
	a->number = 0;
	b->number = 0;

	switch (type) {
		case 't':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;

			cd = sdss->data+i;
			if ( cd->u_b_color >= 0.8 && cd->u_b_color <= 1.05
			&& cd->stellar_mass >= 10 && cd->stellar_mass <= 11 )
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		case 'z':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;

			cd = sdss->data+i;
			if ( cd->z >= l && cd->z <= u)
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		case 'c':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;

			cd = sdss->data+i;
			if ( cd->u_b_color >= l && cd->u_b_color <= u)
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		case 'm':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;

			cd = sdss->data+i;
			if ( cd->stellar_mass >= l && cd->stellar_mass <= u)
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		case 'f':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;
			double ratio;

			cd = sdss->data+i;
			ratio = cd->u_b_color / cd->stellar_mass;
			if ( ratio >= l && ratio <= u)
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		case 'r':
		for ( i=0; i<sdss->number; i++ ) {
			catdata_t *cd;

			cd = sdss->data+i;
			if ( cd->ra >= l && cd->ra <= u)
				MEM_AppendCat( sdss, a, i );
			else
				MEM_AppendCat( sdss, b, i );
		}
		break;
		default:
		printf( "Did not recognize NVSS divide command '%c'.\n", type );
		return;
	}
	printf( "Divided SDSS by %c from %lf to %lf\n", type, l, u );
	printf( "Bin A: %i sources.\nBin B: %i sources.\n",a->number,b->number);
}

/*
================
STAT_ThreadFinished

Worker thread finished
================
*/
void	STAT_ThreadFinished( void ) {

	pthread_mutex_lock( &div_mutex );
	divThreadsFin++;
	pthread_mutex_unlock( &div_mutex );

	if ( divThreadsFin < numDivThreads + 1 )
		return;

	// worker threads finished
	if ( scripted == 0 ) {
		// free only in interactive mode here
		free( dpThreads );
		free( dThreadData );
	}

	printf( "Sorted %i sources to A and %i sources to B.\n",
					nvss_A.number, nvss_B.number );

	printf( "----------------------------------------" );
	printf( "----------------------------------------\n" );
}

/*
================
STAT_DivThread

Worker thread for dividing NVSS
================
*/
void*	STAT_DivThread( void *arg ) {
	threadData_t	*threadData;
	int		i, k, j;
	catalog_t	*from, *toA, *toB;
	catalog_t	*sdss;
	int		start;
	int		end;
	double		threshold;
	time_t		tic;
	time_t		toc;

	// thread may be canceled any time
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, &i );

	// get our worker info
	threadData = arg;

	sdss = &sdss_A;
	from = threadData->from;
	toA = threadData->toA;
	toB = threadData->toB;
	start = threadData->start;
	end = threadData->end;
	threshold = threadData->threshold;
	tic = threadData->tic;

	j = 0;

	// iterate
	for ( i=start; i<end; i++ ){
		catdata_t	*cdn;
		int		foundA = 0;

		// print out info how far we got
		if ( i%500 == 0 ) {
			double difft;

			pthread_mutex_lock( &div_mutex );
			div_completed += i - start;
			div_completed -= j;
			j = i - start;
			pthread_mutex_unlock( &div_mutex );

			time( &toc );

			difft = difftime(toc,tic);
			printf( "%i%% time: %.1lf / %.1lf minutes\n",
					100 * div_completed / from->number,
					difft/60,
					difft * from->number/div_completed/60 );
		}

		cdn = from->data + i;
		// calculate distance to nearest SDSS
		for ( k=0; k<sdss->number; k++ ) {
			catdata_t *cds;
			int result;

			cds = sdss->data + k;
			result = COM_ImpLThreshold( cdn, cds, threshold );

			if ( result == 1 ) {
				// found one nearby
				pthread_mutex_lock( &div_mutex );
				MEM_AppendCat( from, toA, i );
				pthread_mutex_unlock( &div_mutex );
				foundA = 1;
				break;
			}
		}

		if ( foundA == 0 ) {
			pthread_mutex_lock( &div_mutex );
			MEM_AppendCat( from, toB, i );
			pthread_mutex_unlock( &div_mutex );
		}
	}

	// Thread finished.
	STAT_ThreadFinished();

	return NULL;
}

/*
================
STAT_DivideNVSS

Start threads to divide NVSS into A and B
================
*/
void	STAT_DivideNVSS( const char *cmdLine ) {
	double			threshold;
	int			i;
	catalog_t		*nvss;
	catalog_t		*a, *b;

	// Read division parameters
	if ( sscanf( cmdLine, "%lf %i", &threshold, &numDivThreads ) != 2 ) {
		threshold	= 20.0;
		numDivThreads	= 2;
	}
	printf( "Dividing NVSS data (%i threads)...\n", numDivThreads );
	printf( "Threshold is %lf Kiloparsecs\n", threshold );

	// a,b
	nvss = nvss_culled;
	a = &nvss_A;
	b = &nvss_B;

	// reset everything
	a->threshold = threshold;
	b->threshold = threshold;
	a->number = 0;
	b->number = 0;

	div_completed = 0;
	divThreadsFin = 0;

	// allocate memory for thread management
	dpThreads	= malloc( (numDivThreads+1) * sizeof(pthread_t) );
	dThreadData	= malloc( (numDivThreads+1) * sizeof(threadData_t) );

	// distribute the computation work among first n workers
	for ( i=0; i<numDivThreads; i++ ){
		(dThreadData+i)->start = i*(nvss->number/numDivThreads);
		(dThreadData+i)->end = (i+1)*(nvss->number/numDivThreads);
		(dThreadData+i)->threshold = threshold;
		(dThreadData+i)->from = nvss;
		(dThreadData+i)->toA = a;
		(dThreadData+i)->toB = b;
	}
	// last worker gets the remainder
	(dThreadData+numDivThreads)->start = nvss->number-(nvss->number%numDivThreads);
	(dThreadData+numDivThreads)->end = nvss->number;
	(dThreadData+numDivThreads)->threshold = threshold;
	(dThreadData+numDivThreads)->from = nvss;
	(dThreadData+numDivThreads)->toA = a;
	(dThreadData+numDivThreads)->toB = b;

	// let the workers begin
	for ( i=0; i<=numDivThreads; i++ ){
		int err;

		// add a timestamp to measure working time
		time(&(dThreadData+i)->tic);

		err = pthread_create( dpThreads+i, NULL,
					STAT_DivThread, dThreadData+i );

		if ( err != 0 )
			printf( "Error creating thread %i of %i.\n",
					i, numDivThreads );
	}
}

/*
================
STAT_CancelNVSS

Stop NVSS culling
================
*/
void	STAT_CancelNVSS( void ) {
	int i;

	if ( divThreadsFin == numDivThreads + 1 ) {
		printf( "Threads already finished.\n" );
		return;
	}

	printf( "Stopping threads...\n" );
	for ( i=0; i<numDivThreads; i++ ){
		int err;

		err = pthread_cancel( *(dpThreads+i) );
		if ( err != 0 )
			printf( "Error stopping thread %i of %i.\n",
					i+1, numDivThreads );
	}
	// wait for the last thread
	pthread_join( *(dpThreads+numDivThreads), NULL );

	printf( "done.\n" );
}

/*
================
STAT_WaitForThreads

Wait for the dividing threads to finish
================
*/
void	STAT_WaitForThreads( void ) {
	int i;

	// wait for all threads to finish
	for ( i=0; i<numDivThreads+1; i++ ){
		if ( pthread_join( *(dpThreads+i), NULL ) != 0 )
			printf( "Error joining thread %i of %i.\n",
							i, numDivThreads );
	}
}

/*
================
STAT_DivideCancel

Process Divide/ Cancel divide command
================
*/
int	STAT_DivideCancel( const char *cmdLine ) {
	char subcommand;

	subcommand = cmdLine[0];

	if ( subcommand == 's' )
		// cull SDSS
		STAT_DivideSDSS( cmdLine+1 );
	else if ( subcommand == 'n' ) {
		// Cull the NVSS
		STAT_DivideNVSS( cmdLine+1 );
		if ( scripted == 0 )
			// no separating line...
			return 1;
		else {
			STAT_WaitForThreads();
			free( dpThreads );
			free( dThreadData );
		}
	}
	else if ( subcommand == 'c' )
		// Cancel NVSS culling
		STAT_CancelNVSS();

	return 0;
}

