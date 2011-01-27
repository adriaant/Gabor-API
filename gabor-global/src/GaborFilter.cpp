/*
	Description:	Implementation for GaborFilter class
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-3 Adriaan Tijsseling. All rights reserved.
*/

#include "GaborFilter.h"

// default constructor just sets everything to default
GaborFilter::GaborFilter()
{
	mSizeY = 16;
	mSizeX = 16;
	mSigma = M_PI;
	mAngle = 0;
	mPhase = 0;
	mFrequency = 1.0;
	mReal = NULL;
	mImaginary = NULL;
}


// destructor
GaborFilter::~GaborFilter()
{
	int i;
	
	// free up memory
	if ( mReal != NULL )
	{
		for ( i = 0; i < mSizeY; i++ ) delete[] mReal[i];
		delete[] mReal;
	}
	if ( mImaginary != NULL )
	{
		for ( i = 0; i < mSizeY; i++ ) delete[] mImaginary[i];
		delete[] mImaginary;
	}
}


// set up the filter
void GaborFilter::Initialize( int sizey, int sizex, float a, float f, float s, float p )
{
	float x, y, exponential, sincos;
	
// set internal variables
	mSizeY = sizey;
	mSizeX = sizex;
	mSigma = s;
	mAngle = a;
	mPhase = p;
	mFrequency = f * M_PI / 2.0;

// find origin of filter
	mYO = mSizeY / 2;
	mXO = mSizeX / 2;
	
// allocate memory for filter
	mReal 	   = new float*[mSizeY];		// real part of filter
	mImaginary = new float*[mSizeY];		// imaginary part of filter

// initialize filter values
	for ( int i = 0; i < mSizeY; i++ )
	{
		mReal[i] 	  = new float[mSizeX];
		mImaginary[i] = new float[mSizeX];
		
		for ( int j = 0; j < mSizeX; j++ )
		{
		// offset from origin
			y = (float)( i - mYO );
			x = (float)( j - mXO );
		// calculate exponential part
			exponential = exp( - ( x*x + y*y ) / mSigma );
		// calculate sin-cos sum
			sincos = mFrequency * ( y * cos( mAngle ) - x * sin( mAngle ) );
			mReal[i][j] 	 = exponential * sin( sincos );
			mImaginary[i][j] = exponential * ( cos( sincos ) - exp((-1.0*M_PI*M_PI)/2.0) );
		}
	}
}


// save the filter image
void GaborFilter::Save( char* file, int angle, int freq )
{
	PGMImage	pgmImage;
	char		filename[256];
	char		suffix[32];
	
	strcpy( filename, file );
	sprintf( suffix, "-gf_i_%d_%d.pgm", angle, freq );
	strcat( filename, suffix );
	pgmImage.WriteScaled( filename, mImaginary, mSizeY, mSizeX );
	strcpy( filename, file );
	sprintf( suffix, "-gf_r_%d_%d.pgm", angle, freq );
	strcat( filename, suffix );
	pgmImage.WriteScaled( filename, mReal, mSizeY, mSizeX );
}

