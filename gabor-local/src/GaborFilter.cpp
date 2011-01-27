/*
Description:	Implementation for GaborFilter class
Author:			Adriaan Tijsseling (AGT)
Copyright: 		(c) Copyright 2002 Adriaan Tijsseling. All rights reserved.
Change History (most recent first):
	18/04/2002 - AGT - initial version
*/

#include "GaborFilter.h"

// default constructor just sets everything to default
GaborFilter::GaborFilter()
{
	mRadius = 16;
	mSigma = M_PI;
	mAngle = 0;
	mPhase = 0;
	mFrequency = 1.0;
	mReal = NULL;
	mImaginary = NULL;
}

// destructor: free up memory
GaborFilter::~GaborFilter()
{
	int i;
	
	if ( mReal != NULL )
	{
		for ( i = 0; i < mRadius; i++ ) delete[] mReal[i];
		delete[] mReal;
	}
	if ( mImaginary != NULL )
	{
		for ( i = 0; i < mRadius; i++ ) delete[] mImaginary[i];
		delete[] mImaginary;
	}
}


// set up the filter
void GaborFilter::Initialize( int radius, float a, float f, float s, float p )
{
	float x, y, exponential, sincos;
	
// set internal variables
	mRadius = 2 * radius;
	mXYO = radius;	// origin of filter
	mSigma = s;
	mAngle = a;
	mPhase = p;
	mFrequency = f * M_PI / 2.0;
	
// allocate memory for this filter
	mReal 		= new float*[mRadius];		// real part of filter
	mImaginary 	= new float*[mRadius];		// imaginary part of filter

// initialize values of filter
	for ( int i = 0; i < mRadius; i++ )
	{
		mReal[i] 	  = new float[mRadius];
		mImaginary[i] = new float[mRadius];
		
		for ( int j = 0; j < mRadius; j++ )
		{
		// offset from origin
			y = (float)( i - mXYO );
			x = (float)( j - mXYO );
			
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
	sprintf( suffix, "gf_i_%d_%d.pgm", angle, freq );
	strcat( filename, suffix );
	pgmImage.WriteScaled( filename, mImaginary, mRadius, mRadius );
	strcpy( filename, file );
	sprintf( suffix, "gf_r_%d_%d.pgm", angle, freq );
	strcat( filename, suffix );
	pgmImage.WriteScaled( filename, mReal, mRadius, mRadius );
}

