/*
* compute.c - data handling
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

pthread_mutex_t	cull_mutex = PTHREAD_MUTEX_INITIALIZER;
int		cull_completed;
int		numCulThreads = -1;
int		culThreadsFin = 0;
pthread_t	*cpThreads;
threadData_t	*cThreadData;


/*
================
CUL_CullbyCrit

Cull the catalog based on selection criteria
================
*/
void	CUL_CullbyCrit( cattype_t cType, const char *buf ) {
	int i;
	char type;
	double a, b;
	catalog_t *old;
	catalog_t *new;

	switch ( cType ) {
		default:
		case CT_SDSS:
		old = sdss_culled;
		MEM_SwitchSDSSBuffer();
		new = sdss_culled;
		sscanf( buf, "%c %lf %lf", &type, &a, &b );
		break;
		case CT_NVSS:
		old = nvss_culled;
		MEM_SwitchNVSSBuffer();
		new = nvss_culled;
		sscanf( buf, "%c %lf", &type, &a );
		break;
	}

	new->number = 0;

	switch (type) {
		case 'c':
		for ( i=0; i<old->number; i++ ) {
			if ( fabs((old->data+i)->rot_measure_mean) <= a ) {

				MEM_AppendCat( old, new, i );
			}
		}
		break;
		case 'n':
		for ( i=0; i<old->number; i++ ) {
			if ( fabs((old->data+i)->rot_measure_mean_nn) <= a ) {

				MEM_AppendCat( old, new, i );
			}
		}
		break;
		case 'l':
		for ( i=0; i<old->number; i++ ) {
			if ( fabs((old->data+i)->latitude) >= a ) {

				MEM_AppendCat( old, new, i );
			}
		}
		break;
		case 'd':
		for ( i=0; i<old->number; i++ ) {
			if ( (old->data+i)->dec >= a && (old->data+i)->dec <= b) {

				MEM_AppendCat( old, new, i );
			}
		}
		break;
		case 'z':
		for ( i=0; i<old->number; i++ ) {
			if ( (old->data+i)->z >= a && (old->data+i)->z <= b) {

				MEM_AppendCat( old, new, i );
			}
		}
		break;
		default:
		printf( "Did not recognize cull command '%c'\n", type );
		MEM_SwitchSDSSBuffer();
		return;
	}
	if (cType==CT_SDSS)
		printf( "Culled SDSS by %c from %lf to %lf (%i sources)\n",
						type, a, b, new->number );
	else
		printf( "Culled NVSS by %c from/to %lf (%i sources)\n",
						type, a, new->number );
	
}

/*
================
CUL_ThreadFinished

Worker thread finished
================
*/
void	CUL_ThreadFinished( void ) {

	pthread_mutex_lock( &cull_mutex );
	culThreadsFin++;
	pthread_mutex_unlock( &cull_mutex );

	if ( culThreadsFin < numCulThreads + 1 )
		return;

	// worker threads finished
	if ( scripted == 0 ) {
		// free only in interactive mode here
		free( cpThreads );
		free( cThreadData );
	}

	printf( "Got %i sources after culling.\n", nvss_culled->number );

	printf( "----------------------------------------" );
	printf( "----------------------------------------\n" );
}

/*
================
CUL_CullThread

Worker thread for culling NVSS
================
*/
void*	CUL_CullThread( void *arg ) {
	threadData_t	*threadData;
	int		i, k, j;
	catalog_t	*from, *to;
	catalog_t	*sdss;
	time_t		toc;
	int		start;
	int		end;
	double		threshold;
	time_t		tic;

	// thread may be canceled any time
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, &i );

	// get our worker info
	threadData = arg;

	sdss = sdss_culled;
	from = threadData->from;
	to = threadData->toA;
	start = threadData->start;
	end = threadData->end;
	threshold = threadData->threshold;
	tic = threadData->tic;

	j = 0;

	// iterate
	for ( i=start; i<end; i++ ){
		catdata_t	*cdn;

		// print out info how far we got
		if ( i%500 == 0 ) {
			double difft;

			pthread_mutex_lock( &cull_mutex );
			cull_completed += i - start;
			cull_completed -= j;
			j = i - start;
			pthread_mutex_unlock( &cull_mutex );

			time( &toc );

			difft = difftime(toc,tic);
			printf( "%i%% time: %.1lf / %.1lf minutes\n",
					100 * cull_completed / from->number,
					difft/60,
					difft*from->number/cull_completed/60 );
		}

		cdn = from->data + i;
		// calculate distance to nearest SDSS
		for ( k=0; k<sdss->number; k++ ) {
			catdata_t *cds;
			int result;

			cds = sdss->data + k;
			result = COM_ImpLThreshold( cdn,cds,threshold );

			if ( result == 1 ) {
				// found one nearby
				pthread_mutex_lock( &cull_mutex );
				MEM_AppendCat( from, to, i );
				pthread_mutex_unlock( &cull_mutex );
				break;
			}
		}
	}

	// Thread finished.
	CUL_ThreadFinished();

	return NULL;
}

/*
================
CUL_CullNVSS

Start threads to cull the NVSS
================
*/
void	CUL_CullNVSS( const char *cmdLine ) {
	double			threshold;
	int			i;
	catalog_t		*from;
	catalog_t		*to;

	// Read culling parameters
	if ( sscanf( cmdLine, "%lf %i", &threshold, &numCulThreads ) != 2 ) {
		threshold	= 1000.0;
		numCulThreads	= 2;
	}
	printf( "Culling NVSS data (%i threads)...\n", numCulThreads );
	printf( "Threshold is %lf Kiloparsecs\n", threshold );

	from = nvss_culled;
	MEM_SwitchNVSSBuffer();
	to = nvss_culled;

	to->threshold = threshold;
	to->number = 0;
	cull_completed = 0;

	culThreadsFin = 0;

	// allocate memory for thread management
	cpThreads	= malloc( (numCulThreads+1) * sizeof(pthread_t) );
	cThreadData	= malloc( (numCulThreads+1) * sizeof(threadData_t) );

	// distribute the computation work among first n workers
	for ( i=0; i<numCulThreads; i++ ){
		(cThreadData+i)->start = i*(from->number/numCulThreads);
		(cThreadData+i)->end = (i+1)*(from->number/numCulThreads);
		(cThreadData+i)->threshold = threshold;
		(cThreadData+i)->from = from;
		(cThreadData+i)->toA = to;
	}
	// last worker gets the remainder
	(cThreadData+numCulThreads)->start = from->number-(from->number%numCulThreads);
	(cThreadData+numCulThreads)->end = from->number;
	(cThreadData+numCulThreads)->threshold = threshold;
	(cThreadData+numCulThreads)->from = from;
	(cThreadData+numCulThreads)->toA = to;

	// let the workers begin
	for ( i=0; i<=numCulThreads; i++ ){
		int err;

		// add a timestamp to measure working time
		time(&(cThreadData+i)->tic);

		err = pthread_create(cpThreads+i,NULL,
					CUL_CullThread,cThreadData+i);

		if ( err != 0 )
			printf( "Error creating thread %i of %i.\n",
					i, numCulThreads );
	}
}

/*
================
CUL_CancelNVSS

Stop NVSS culling
================
*/
void	CUL_CancelNVSS( void ) {
	int i;

	if ( culThreadsFin == numCulThreads + 1 ) {
		printf( "Threads already finished.\n" );
		return;
	}

	printf( "Stopping threads...\n" );
	for ( i=0; i<numCulThreads; i++ ){
		int err;

		err = pthread_cancel( *(cpThreads+i) );
		if ( err != 0 )
			printf( "Error stopping thread %i of %i.\n",
					i+1, numCulThreads );
	}
	// wait for the last thread
	pthread_join( *(cpThreads+numCulThreads), NULL );

	printf( "done.\n" );
}

/*
================
CUL_WaitForThreads

Wait for the culling threads to finish
================
*/
void	CUL_WaitForThreads( void ) {
	int i;

	// wait for all threads to finish
	for ( i=0; i<numCulThreads+1; i++ ){
		if ( pthread_join( *(cpThreads+i), NULL ) != 0 )
			printf( "Error joining thread %i of %i.\n",
							i, numCulThreads );
	}
}

/*
================
CUL_CullCancel

Process Cull or Cancel command
================
*/
int	CUL_CullCancel( const char *cmdLine ) {
	char subcommand;
	char subsub;

	subcommand = cmdLine[0];
	subsub = cmdLine[1];

	if ( subcommand == 's' )
		// cull SDSS
		CUL_CullbyCrit( CT_SDSS, cmdLine+1 );
	else if ( subcommand == 'n' ) {
		if ( subsub == ' ' ) {
			// Cull the NVSS
			CUL_CullNVSS( cmdLine+1 );
			if ( scripted == 0 )
				// no separating line...
				return 1;
			else {
				CUL_WaitForThreads();
				free( cpThreads );
				free( cThreadData );
			}
		}
		else CUL_CullbyCrit( CT_NVSS, cmdLine+1 );
	}
	else if ( subcommand == 'c' )
		// Cancel NVSS culling
		CUL_CancelNVSS();

	return 0;
}

