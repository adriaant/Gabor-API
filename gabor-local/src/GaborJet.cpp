/*
	Description:	Implementation for GaborJet class
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-3 Adriaan Tijsseling. All rights reserved.
*/

#include "GaborJet.h"

// default constructor just sets everything to default
GaborJet::GaborJet()
{
	mHeight 	= 512;
	mWidth 		= 512;
	mX			= 128;
	mY			= 128;
	mShowFilter = false;
	mFilters 	= NULL;
	mFiducials	= NULL;
}

// destructor: free up memory
GaborJet::~GaborJet()
{
	if ( mFilters != NULL )
	{
		for ( int i = 0; i < mAngles; i++ ) delete[] mFilters[i];
		delete[] mFilters;
	}
	if ( mFiducials != NULL ) delete[] mFiducials;	
}


// set up the filter
void GaborJet::Initialize( int y, int x, int x0, int y0, int r, 
						   float s, int f, float maxF, float minF, int a, bool save )
{
	int		i, j;
	float	angle, freq;
	
// set internal variables
	mHeight 	= y;
	mWidth 		= x;
	mX			= x0;
	mY			= y0;
	mSigma		= s * M_PI * M_PI;
	mAngles 	= a;
	mFreqs 		= f;
	mRadius		= r;
	mMinFreq 	= minF;
	mMaxFreq 	= maxF;
	mShowFilter = save;
	mFiducials = new float[mAngles * mFreqs];
	
// allocate memory for filters (angles * freqs = total filters)
	mFilters = new GaborFilter * [mAngles];
	for ( i = 0; i < mAngles; i++ )
	{
	// calculate angle
		angle = (float)i * M_PI / (float)mAngles;
		
	// allocate filters for this angle
		mFilters[i] = new GaborFilter[mFreqs];	
		
	// initialize each one	
		for ( j = 0; j < mFreqs; j++ )
		{
		// calculate frequency
			freq = minF + ( j * ( maxF - minF ) ) / (float)mFreqs;
			
		// initialize filter
			mFilters[i][j].Initialize( mRadius, angle, freq, mSigma );
			if ( mShowFilter ) mFilters[i][j].Save( mFile, i, j );
		}
	}	
}


// process an image
void GaborJet::Filter( float** image, int* len )
{	
	int			x, y;		// iterating over location
	int			gx, gy;		// iterating over filters
	int			a, f;		// iterating over angles and frequencies
	int			h, i, j;	// iterating over filter field
	float		sumI, sumR;	// sum of imaginary and of real parts
	
	cerr << "convoluting..." << endl;

// convolve at center of filter location
	// collect responses over angles and frequencies
	h = 0;
	for ( a = 0; a < mAngles; a++ )
	{
		for ( f = 0; f < mFreqs; f++ )
		{
			sumR = 0.0;
			sumI = 0.0;

		// start from bottom-left corner of filter location
			y = mY - mRadius;
			for ( gy = y; gy < y + 2 * mRadius; gy++ )
			{
			// make sure we are not out of bounds
				if ( gy < 0 || gy >= mHeight ) break;
				
			// offset to local coordinates of filter
				i = gy - y;
				
				x = mX - mRadius;
				for ( gx = x; gx < x + 2 * mRadius; gx++ )
				{
				// make sure we are not out of bounds
					if ( gx < 0 || gx >= mWidth ) break;

				// offset to local coordinates of filter
					j = gx - x;

					sumR += image[gy][gx] * mFilters[a][f].GetReal(i,j);
					sumI += image[gy][gx] * mFilters[a][f].GetImaginary(i,j);
				}
			}
			mFiducials[h] = sqrt( sumR*sumR + sumI*sumI );
			h++;
		} // f
	} // a

	*len = mAngles * mFreqs;
}



