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
	mSpacingY 	= 4;
	mSpacingX 	= 4;
	mFilters 	= NULL;
	mPixels		= NULL;
	mResponses	= NULL;
	mNormals	= NULL;
	saveFilter  = false;
}


// destructor: free up memory
GaborJet::~GaborJet()
{
	if ( mFilters != NULL )
	{
		for ( int i = 0; i < mAngles; i++ ) delete[] mFilters[i];
		delete[] mFilters;
	}
	
	if ( mResponses != NULL )
	{
	#if kAngleSeparation
		for ( int i = 0; i < mAngles; i++ )
		{
			for ( int j = 0; j < mFreqs; j++ )
			{
				for ( int k = 0; k < mRespY; k++ ) delete[] mResponses[i][j][k];
				delete[] mResponses[i][j];
			}
			delete[] mResponses[i];
		}
		delete[] mResponses;
	#else
		for ( int i = 0; i < mRespY; i++ ) delete[] mResponses[i];
		delete[] mResponses;	
	#endif
	}
	
	if ( mNormals != NULL ) delete[] mNormals;
}


// set up the filter
void GaborJet::Initialize( int y, int x, int ys, int xs, int ysp, int xsp, 
						float s, int f, float maxF, float minF, int a )
{
	int		i, j, k, l;
	float	angle, freq;
	
// set internal variables
	mHeight 	= y;
	mWidth 		= x;
	mSpacingY 	= ysp;
	mSpacingX 	= xsp;
	mSigma		= s * M_PI * M_PI;
	mAngles 	= a;
	mFreqs 		= f;
	mSizeY		= ys;
	mSizeX		= xs;
	mMinFreq 	= minF;
	mMaxFreq 	= maxF;
	
// allocate memory for filters
	mFilters = new GaborFilter*[mAngles]; // angles * freqs = total filters
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
			mFilters[i][j].Initialize( mSizeY, mSizeX, angle, freq, mSigma );
			if ( saveFilter ) mFilters[i][j].Save( mFile, i, j );
		}
	}
	
// allocate memory for the responses
	mRespY = ( mHeight - mSizeY ) / mSpacingY + 1;
	mRespX = ( mWidth - mSizeX ) / mSpacingX + 1;
#if kAngleSeparation
	mResponses = new float***[mAngles];
	for ( i = 0; i < mAngles; i++ )
	{
		mResponses[i] = new float**[mFreqs];
		for ( j = 0; j < mFreqs; j++ )
		{
			mResponses[i][j] = new float*[mRespY];
			for ( k = 0; k < mRespY; k++ )
			{
				mResponses[i][j][k] = new float[mRespX];
				for ( l = 0; l < mRespX; l++ ) mResponses[i][j][k][l] = 0.0;
			}
		}
	}	
	mNormals = new float[mAngles*mFreqs];
#else
	mResponses = new float*[mRespY];
	for ( i = 0; i < mRespY; i++ )
	{
		mResponses[i] = new float[mRespX];
		for ( j = 0; j < mRespX; j++ ) mResponses[i][j] = 0.0;
	}
	mNormals = new float[mRespX*mRespY];
#endif
}


// process an image
void GaborJet::Filter( float** image, int* len )
{	
	int			rx, ry;		// iterating over mResponses
	int			x, y;		// iterating over location
	int			gx, gy;		// iterating over filters
	int			a, f;		// iterating over angles and frequencies
	int			i, j;		// iterating over filter field
	int			h = 0;		// iterates over normal vector
	float		sumI, sumR;	// sum of imaginary and of real parts
	float		local_sumI, 
				local_sumR;	// sum of imaginary and of real parts
	float		norm;		// for normalization
	
	mPixels = image;

#if kAngleSeparation

// start collecting responses
	h = 0;
	for ( a = 0; a < mAngles; a++ )
	{
		for ( f = 0; f < mFreqs; f++ )
		{
			sumI = 0.0;
			sumR = 0.0;

			y = 0;
			for ( ry = 0; ry < mRespY; ry++ )
			{
				x = 0;
				for ( rx = 0; rx < mRespX; rx++ )
				{
					local_sumI = 0.0;
					local_sumR = 0.0;
					
					for ( gy = y; gy < y + mSizeY; gy++ )
					{
						for ( gx = x; gx < x + mSizeX; gx++ )
						{
						// make sure we are not out of bounds
							if ( gx > mWidth || gy > mHeight ) break;
						// offset to local coordinates of filter
							i = gy - y;
							j = gx - x;
						// get real and imaginary products
							sumR += mPixels[gy][gx] * mFilters[a][f].GetReal(i,j);
							sumI += mPixels[gy][gx] * mFilters[a][f].GetImaginary(i,j);
							local_sumR += mPixels[gy][gx] * mFilters[a][f].GetReal(i,j);
							local_sumI += mPixels[gy][gx] * mFilters[a][f].GetImaginary(i,j);
						}
					}					
				// collect responses
					mResponses[a][f][ry][rx] = sqrt( local_sumR*local_sumR + local_sumI*local_sumI );
					
					x = x + mSpacingX;			
				}	// rx
				y = y + mSpacingY;
			}	// ry

			mNormals[h] = sqrt( sumR*sumR + sumI*sumI );
			h++;
		}	// f
	}	// a

	float max, min;
	max = min = mNormals[0];
	for ( h = 0; h < mAngles*mFreqs; h++ )
	{	
		if( mNormals[h] > max ) max = mNormals[h];
		if( mNormals[h] < min ) min = mNormals[h];
	}
	norm = max - min;
	for ( h = 0; h < mAngles*mFreqs; h++ )
		mNormals[h] = 1.0 * ( ( mNormals[h] - min ) / norm );

	*len = mAngles * mFreqs;

#else
	y = 0;
	for ( ry = 0; ry < mRespY; ry++ )
	{
		x = 0;
		for ( rx = 0; rx < mRespX; rx++ )
		{
		// start collecting responses
			sumI = 0.0;
			sumR = 0.0;
			for ( a = 0; a < mAngles; a++ )
			{
				for ( f = 0; f < mFreqs; f++ )
				{
					for ( gy = y; gy < y + mSizeY; gy++ )
					{
						for ( gx = x; gx < x + mSizeX; gx++ )
						{
						// make sure we are not out of bounds
							if ( gx > mWidth || gy > mHeight ) break;
						// offset to local coordinates of filter
							i = gy - y;
							j = gx - x;
						// get real and imaginary products
							sumR += mPixels[gy][gx] * mFilters[a][f].GetReal(i,j);
							sumI += mPixels[gy][gx] * mFilters[a][f].GetImaginary(i,j);
						}
					}					
				}	// f
			}	// a
			// collect responses
			x = x + mSpacingX;
			mResponses[ry][rx] = sqrt( sumR*sumR + sumI*sumI );
		}	// rx
		y = y + mSpacingY;
	}	// ry

	h = 0;
// normalize the responses
	
	float max, min;
		
	max = min = mResponses[0][0];
	for ( ry = 0; ry < mRespY; ry++ )
		for ( rx = 0; rx < mRespX; rx++ )
		{
			if( mResponses[ry][rx] > max ) max = mResponses[ry][rx];
			if( mResponses[ry][rx] < min ) min = mResponses[ry][rx];
		}

	norm = max - min;
	h = 0;
	for ( ry = 0; ry < mRespY; ry++ )
		for ( rx = 0; rx < mRespX; rx++ )
		{
			mNormals[h] = 1.0 * ( ( mResponses[ry][rx] - min ) / norm );
			h++;
		}

	*len = mRespX * mRespY;

#endif

// save normals and responses to file
	if ( saveFilter ) Save();
}

#if kAngleSeparation

// save gabor responses and normals to file
void GaborJet::Save( void )
{
	PGMImage	pgmImage;
	char		filename[256];
	char		suffix[32];
	ofstream	outfile;
	
	for ( int a = 0; a < mAngles; a++ )
		for ( int f = 0; f < mFreqs; f++ )
		{
			strcpy( filename, mFile );
			sprintf( suffix, "-response-%d-%d.pgm", a, f );
			strcat( filename, suffix );
			pgmImage.WriteScaled( filename, mResponses[a][f], mRespY, mRespX );
		}
}

#else

void GaborJet::Save( void )
{
	PGMImage	pgmImage;
	char		filename[256];
	ofstream	outfile;
	
	strcpy( filename, mFile );
	strcat( filename, "-response.pgm" );
	pgmImage.WriteScaled( filename, mResponses, mRespY, mRespX );
}

#endif
