/*
	Description:	Class definition for a Gabor Jet
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-3 Adriaan Tijsseling. All rights reserved.
*/

#ifndef __GABORJET__
#define __GABORJET__

#define kAngleSeparation 0

#include "GaborGlobal.h"
#include "GaborFilter.h"


class GaborJet
{
public:

	GaborJet();
	~GaborJet();
	
	void	Initialize( int y, int x, int ys, int xs, int ysp, int xsp, 
						float s = 2.0, int f = 2, float maxF = 2, float minF = 1, int a = 8 );
	void	Filter( float** image, int* len );
	float	GetResponse( int idx ) { return mNormals[idx]; }

	void	Save( void );

	inline void		SetFileName( char* file ) { strcpy( mFile, file ); saveFilter = true; }
	
protected:

	int				mHeight;	// vertical size of image
	int				mWidth;		// horizontal size of image
	int				mSpacingY;	// vertical amount of pixels between subsequent GFs
	int				mSpacingX;	// horizontal amount of pixels between subsequent GFs
	float			mSigma;		// modulator for standard deviation sigma
	int				mAngles;	// number of orientations
	int				mFreqs;		// number of frequencies
	int				mSizeY;		// vertical size of filter
	int				mSizeX;		// horizontal size of filter
	int				mRespY;		// height of response matrix
	int				mRespX;		// width of response matrix
	float			mMinFreq;	// minimum frequency
	float			mMaxFreq;	// maximum frequency
	GaborFilter**	mFilters;	// set of filters in use
	float**			mPixels;	// the pixel matrix to filter
#if kAngleSeparation
	float****		mResponses;	// the gabor filtered image
#else
	float**			mResponses;	// the gabor filtered image
#endif
	float*			mNormals;	// normalized responses (for NN)
	char			mFile[256];	// filename
	bool			saveFilter;
};

#endif
