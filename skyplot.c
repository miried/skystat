/*
* skyplot.c - main code
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
* GLOBAL VARIABLES
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
int	scripted;

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* GENERAL PURPOSE CODE
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

/*
================
SKY_PrintHelp

print help dialog
================
*/
void	SKY_PrintHelp( void ) {
	printf( "TODO.\n" );
}

/*
================
SKY_MeanRM

mean RM
================
*/
void	SKY_MeanRM( const char *cmdLine ) {
	char subcommand;

	subcommand = cmdLine[0];
	switch ( subcommand ) {
		case 'a':
		COM_MeanRM( cmdLine+1 );
		break;
		case 'n':
		COM_MeanRM_NN( cmdLine+1 );
		break;
		default:
		printf( "Unknown RM method %c.\n", subcommand );
		return;
	}
}

/*
================
SKY_ExecCmd

execute command
================
*/
int	SKY_ExecCmd( const char *line ){
	char command;

	command = line[0];

	switch ( command ) {
	case '\n':
	case '#':
		// Ignore this cmd
		break;
	case 'm':
		// Compute mean RM
		SKY_MeanRM( line+1 );
		break;
	case 'p':
		// Draw the Plot
		VIS_DrawPlot( line+1 );
		break;
	case 'l':
		// load previous work
		MEM_LoadCulled();
		break;
	case 's':
		// load previous work
		MEM_SaveCulled();
		break;
	case 'r':
		// reset culling
		MEM_ResetCulled();
		break;
	case 'c':
		// cull
		if ( CUL_CullCancel(line+1) == 1 )
			return 1;
		break;
	case 'd':
		// Divide into A and B
		if ( STAT_DivideCancel(line+1) == 1 )
			return 1;
		break;
	case 'w':
		// Write data to disk for stat. analysis
		STAT_WriteToDisk( line+1 );
		break;
	case 'h':
		// Print Help message
		SKY_PrintHelp();
		break;
	case 'e':
		// execute script
		if ( scripted == 0 && strlen(line) > 2 ) {
			char fname[64];
			scripted = 1;
			strncpy( fname, line+2, 64 );
			fname[strlen(fname)-1] = 0;
			SKY_ScriptLoop( fname );
		}
		break;
	case 'q':
		// Quit program
		return 0;
	default:
		// Unrecognized Command
		printf("Command '%c' not recognized.\n",line[0]);
		break;
	}

	return 1;
}

/*
================
SKY_ScriptLoop

loop to read script cmds
================
*/
void	SKY_ScriptLoop( const char *fileName ) {
	char	cmdBuffer[64];
	int	n;
	int	fin;

	printf( "Executing script %s\n", fileName );
	if ( FIO_OpenScriptFile(fileName) == 0 )
		return;

	n = 0;
	fin = 1;
	while ( FIO_ReadScript( cmdBuffer, sizeof(cmdBuffer) ) != 0 ) {
		time_t		tic;
		struct tm 	*ltime;
		char		clocktime[16];

		n++;
		if ( cmdBuffer[0] == '#' || cmdBuffer[0] == '\n' )
			continue;

		// get command time
		time( &tic );
		ltime = localtime( &tic );
		strftime( clocktime, 16, "%X", ltime );
		// print command
		printf( "| Script line %i (%s): %s", n, clocktime, cmdBuffer );
		printf( "----------------------------------------" );
		printf( "----------------------------------------\n" );

		// Execute the command
		fin = SKY_ExecCmd(cmdBuffer);
		if ( fin == 0 )
			break;

		printf( "----------------------------------------" );
		printf( "----------------------------------------\n" );
	}

	FIO_CloseScriptFile();

	if ( fin == 1 ) {
		scripted = 0;
		SKY_CmdLoop();
	}
}

/*
================
SKY_CmdLoop

loop to listen for commands
================
*/
void	SKY_CmdLoop( void ) {
	char lineBuffer[128];

	printf( "Interactive mode.\n" );
	while ( 1 ) {

		printf( ": " );
		if ( fgets(lineBuffer,sizeof(lineBuffer),stdin) == 0 )
			printf( "read stdin error.\n" );

		if ( SKY_ExecCmd(lineBuffer) == 0 )
			return;

		////////////////////////////////////////////////////////////////
		printf( "----------------------------------------" );
		printf( "----------------------------------------\n" );
	}
}

/*
================
main

skyplot main loop waiting for commands on the terminal
================
*/
int	main( int argc, char **argv ) {

	printf( "========================================" );
	printf( "========================================\n" );
	printf( "|| Skyplot started.\n" );
	printf( "|| (C) 2012 Michael Rieder <mr@student.ethz.ch>\n" );
	printf( "========================================" );
	printf( "========================================\n" );

	// Initialize Program
	MEM_Init();

	// Decide whether Scripted or Interactive
	scripted = 0;
	if ( argc == 2 )
//		if ( strncmp(argv[1],"script",6) == 0 )
			scripted = 1;

	if ( scripted == 1 )
		// read script file
		SKY_ScriptLoop( argv[1] );
	else
		// loop to receive commands on cmd line
		SKY_CmdLoop();

	// Wait for things to finish...
	
	// Free Buffers and Close
	printf( "Quit.\n" );
	MEM_FreeAllBuffers();
	printf( "========================================" );
	printf( "========================================\n" );
	return EXIT_SUCCESS;
}

