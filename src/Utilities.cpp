/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002 Adriaan Tijsseling. All rights reserved.
	Description:	Generic utilities
*/

#include	<math.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	"Utilities.h"

long	gPrecision;
long	gWidth;

// for Permute
struct	tmp
{
	int p;		/* permutation		*/
	int r;		/* random number	*/
};


// Permutes an array
void Permute( int* array, size_t size )
{
	struct tmp 	*t;
	size_t 		i;
	
	t = new tmp[size];
	for( i = 0; i < size; i++ ) 	// load up struct with data
	{
		t[i].r = rand();
		t[i].p = array[i];
	}	
	qsort( t, size, sizeof(struct tmp), cmp );	// shuffle
	
	// data back to original array
	for( i = 0; i < size; i++ ) array[i] = t[i].p;
	
	delete[] t;
}

// use for permuted qsort
int cmp( const void *s1, const void *s2 )
{
	struct tmp *a1 = (struct tmp *)s1;
	struct tmp *a2 = (struct tmp *)s2;
	
	return((a1->r) - (a2->r));
}


float Heavyside( float a ) 
{
	// if a is larger than 0.5, return 1.0, else return 0.0
	return( ( a > 0.5 ) ? 1.0 : 0.0 );
}


float Sigmoid( float act )
{
	return ( 1.0 / ( 1.0 + exp( -1.0 * act ) ) );
}


// beta must be negative
// untested
float Sigmoid( float beta, float a_pot ) 
{
	return ( 1.0 / ( 1.0 + exp( beta * a_pot ) ) );
}

float Sigmoid( float beta, float a_pot, float thresh ) 
{
	return ( 1.0 / ( 1.0 + exp( beta * a_pot + thresh ) ) );
}

// Create a matrix and fill it with constant given in parameter
int **CreateMatrix( int val, int row, int col ) 
{
	int **matrix = new int*[row];
	
	for ( int i = 0; i < row; i++ ) 
	{
		matrix[i] = new int[col];		
		for ( int j = 0; j < col; j++ ) matrix[i][j] = val;
	}
	return matrix;
}

// Reset a matrix with new value val
void ResetMatrix( int ** matrix, int val, int row, int col ) 
{
	for ( int i = 0; i < row; i++ )
		for ( int j = 0; j < col; j++ )
			matrix[i][j] = val;
}

// Dispose allocated matrix
void DisposeMatrix( int** matrix, int row )
{
	for ( int i = 0; i < row; i++ ) delete[] matrix[i];
	delete[] matrix;
}


// Create a matrix and fill it with constant given in parameter
float **CreateMatrix( float val, int row, int col ) 
{
	float **matrix = new float*[row];
	
	for ( int i = 0; i < row; i++ ) 
	{
		matrix[i] = new float[col];		
		for ( int j = 0; j < col; j++ ) matrix[i][j] = val;
	}
	return matrix;
}

// Reset a matrix with new value val
void ResetMatrix( float ** matrix, float val, int row, int col ) 
{
	for ( int i = 0; i < row; i++ )
		for ( int j = 0; j < col; j++ )
			matrix[i][j] = val;
}

// Dispose allocated matrix
void DisposeMatrix( float** matrix, int row )
{
	for ( int i = 0; i < row; i++ ) delete[] matrix[i];
	delete[] matrix;
}


// Returns Euclidean distance between two vectors
float ReturnDistance( float *pat1, float *pat2, int size ) 
{
	float dist = 0.0;
	
	for ( int i = 0; i < size; i++ )
		dist += ( pat1[i] - pat2[i] ) * ( pat1[i] - pat2[i] );
	
	return (float)( sqrt( dist ) / sqrt( (float)size ) );
}

// For file reading purposes. Skips blanks and lines starting with #
void SkipComments( ifstream* infile )
{
	bool garbage = true;
	char c;
	
	while ( garbage )
	{
		// ignore any line feeds left in the stream
		while ( infile->peek() == '\n' || infile->peek() == ' ' || infile->peek() == '\t' ) 
			infile->get();	
		while ( infile->peek() == '#' )infile->ignore( 1000, '\n' );
		infile->get(c);
		if ( c == '\n' || c == '\t' || c == ' ' || c == '#' )
			garbage = true;
		else
			garbage = false;
		infile->putback(c);
	}
}


void FileCreateError( char* filename )
{
	char folder[FILENAME_MAX];
	
	getcwd( folder, FILENAME_MAX );		
	cerr << "Error: Could not create file " << filename << " in directory ";
	cerr << folder << endl;
}

void FileOpenError( char* filename )
{
	char folder[FILENAME_MAX];
	
	getcwd( folder, FILENAME_MAX );		
	cerr << "Error: Could not open file " << filename << " in directory ";
	cerr << folder << endl;
}


// cout, cerr and ostream formatting utilities

void GetStreamDefaults( void )
{
	gWidth = cout.width();
	gPrecision = cout.precision();
}

void AdjustStream( ostream &os, int precision, int width, int pos, bool trailers )
{
	os.precision( precision );
	os.width( width );
	os.fill( ' ' );
	if ( trailers )
		os.setf( ios::showpoint,  ios::showpoint );
	else
		os.unsetf( ios::showpoint );
	if ( pos == kLeft )
		os.setf( ios::left, ios::adjustfield );
	else
		os.setf( ios::right, ios::adjustfield );
}

void SetStreamDefaults( ostream &os )
{
	os.precision( gPrecision );
	os.width( gWidth );
	os.unsetf( ios::showpoint );
	os.setf( ios::left, ios::adjustfield );
}


// return absolute value

double SafeAbs( double val1, double val2 )
{
	double diff = val1 - val2;
	
	if ( diff < 0.0 ) return ( 0.0 - diff );
	else return diff;
}

float SafeAbs( float val1, float val2 )
{
	float diff = val1 - val2;
	
	if ( diff < 0.0 ) return ( 0.0 - diff );
	else return diff;
}

int SafeAbs( int val1, int val2 )
{
	int diff = val1 - val2;
	
	if ( diff < 0 ) return ( 0 - diff );
	else return diff;
}

double SafeAbs( double val )
{
	if ( val < 0.0 ) return ( 0.0 - val );
	else return val;
}

float SafeAbs( float val )
{
	if ( val < 0.0 ) return ( 0.0 - val );
	else return val;
}

int SafeAbs( int val )
{
	if ( val < 0 ) return ( 0 - val );
	else return val;
}

