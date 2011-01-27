/*
	Description:	Gabor API usage
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-3 Adriaan Tijsseling. All rights reserved.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "GaborGlobal.h"	// contains project-wide defines, constants, and globals
#include "GaborJet.h"
#include "ContrastFilter.h"
#include "LogPolar.h"
#include "PGMImage.h"
#include "Utilities.h"

// use these defines to toggle the use of log-polar and contrastfilter
#define kUseLogPolar	0
#define kUseContrast	1
// set to use color or grayscale
#define	kUsingColor		0

// GLOBAL 
bool		kSaveFilter = true;	// in case of multiple files, we save GFs only once
bool		kVerbosity = true; 	// whether to output any messages or not

// LOCAL GLOBALS
// These are the settings for the Gabor filter jet. These are of course modifiable.
int			gx = 32;	//	-X	: horizontal size of filter
int			gy = 32;	//	-Y	: vertical size of filter
int			sx = 28;	//	-x	: horizontal spacing of filter, i.e. overlap is gx - sx pixels
int			sy = 28;	//	-y	: vertical spacing of filter
float		s = 2.0;	//	-s	: sigma modulator
int			a = 8;		//	-a	: number of angles
int			f = 1;		//	-f	: number of frequencies
float		l = 1;		//	-l	: lower bound of frequency
float		u = 2;		//	-u	: upper bound of frequency


// PROTOTYPES
float*		ProcessFile( char*, int***, int, int, float*, int* );
float* 		ProcessChannel( float**, int, int, float*, int*, int, char* );
void		Usage( void );


// process command line arguments and filter selected files
int main( int argc, char *argv[] )
{
	int		arg;
	char	file[256];

	cout << "# ";	
	if ( argc > 1 )
	{
	// run down each argument
		arg = 1;
		while( arg < argc )
		{
		// check if -h is called
			if( strcmp( argv[arg], "-h") == 0 )
			{
				arg++;
				Usage();
			}
			if( strcmp( argv[arg], "-X") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				gx = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-Y") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				gy = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-x") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				sx = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-y") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				sy = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-s") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				s = atof( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-a") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				a = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-f") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				f = atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-l") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				l = atof( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-u") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				u = atof( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-v") == 0 )
			{
				arg++;
				if ( argv[arg] == NULL ) Usage();
				kVerbosity = (bool)atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-S") == 0 )
			{
				arg++;
				if ( argv[arg] == NULL ) Usage();
				kSaveFilter = (bool)atoi( argv[arg] );
				goto loop;
			}
			if( argv[arg][0] != '-' ) break;
loop:
			arg++;
		}
	}
	cout << endl;

// better to pass some file to process!
	if ( arg >= argc )
	{
		Usage();
		return 0;
	}
	
	for( int i = arg; i < argc; i++ )
	{
	// copy argument to filename
		strcpy( file, argv[i] );
		if ( kVerbosity ) cerr << "Processing file \"" << argv[i] << "\"..." << endl;

	// load the image (only PGM and PPM are supported)
		int		 imgHeight, imgWidth;
		int***	 image = NULL;		// allocated and freed by PGMImage
		PGMImage pgmImage( file );	
		imgHeight = pgmImage.GetHeight();
		imgWidth  = pgmImage.GetWidth();
		image 	  = pgmImage.GetRGBPixels();

	// filter this image (assumes image is in RGB color)
		float *response = NULL;
		int	  len = 0;
		response = ProcessFile( file, image, imgHeight, imgWidth, response, &len );

	// write the filter response to console	
		cout << "# " << argv[i] << " " << len << endl;
		for ( int j = 0; j < len; j++ ) cout << response[j] << " ";
		cout << endl;

	// clean up	
		if ( response != NULL ) delete[] response;
	}
		
	return 0;
}


/* 
pass the rgb pixel data for filtering; the response is returned in the response vector. 
This vector will be initialized by the Gabor API, but MUST disposed of it by the user.
respLen is the address of an int holding the length of the response vector. 
*/
float* ProcessFile( char* file, int*** rgb, int h, int w, float* response, int* respLen )
{
	char 	basename[256];
	char	dirStr[256];
	char*	fileStr;
	int		i, j, len;

// extract directory path from filename
	strcpy( dirStr, file );
	fileStr = strrchr( dirStr, '/' );
	if ( fileStr != NULL )
	{
		strcpy( basename, fileStr );				// save part after last slash
		dirStr[fileStr-dirStr+1] = '\0';				// save directory string
		strcat( dirStr, basename );
		strcat( dirStr, "-globalGF" );
		basename[strlen(basename)-4] = '\0';			// remove extension
		for ( i = 0; i < strlen( basename ); i++ )		// remove intial slash
			basename[i] = basename[i+1];
		basename[i] = '\0';
	}
	else	// we are in working directory
	{
		strcpy( basename, file );
		basename[strlen(basename)-4] = '\0';			// remove extension
		strcpy( dirStr, basename );
		strcat( dirStr, "-globalGF" );
	}
	
// create img directory in extracted path
	if ( kSaveFilter == 1 )
	{
		mkdir( dirStr, S_IRWXU | S_IRWXG );
		if ( kVerbosity ) cerr << "Created directory: \"" << dirStr << "\"" << endl;
		strcat( dirStr, "/" );
	}

#if kUsingColor	// USING COLOR

// allocate pixels for rgb matrix
	float*** pixels = new float**[3];
	for ( int i = 0; i < 3; i++ ) pixels[i] = CreateMatrix( (float)255.0, h, w );
	for ( i = 0; i < h; i++ )
	{
		for ( j = 0; j < w; j++ )
		{
			pixels[0][i][j] = (float)rgb[0][i][j];
			pixels[1][i][j] = (float)rgb[1][i][j];
			pixels[2][i][j] = (float)rgb[2][i][j];
		}
	}

// save channels to file
	if ( kSaveFilter == 1 )
	{	
		PGMImage channelImg;
		strcpy( basename, dirStr );
		strcat( basename, "red.ppm" );
		channelImg.Write( basename, pixels[0], h, w, 0 );
		strcpy( basename, dirStr );
		strcat( basename, "green.ppm" );
		channelImg.Write( basename, pixels[1], h, w, 1 );
		strcpy( basename, dirStr );
		strcat( basename, "blue.ppm" );
		channelImg.Write( basename, pixels[2], h, w, 2 );
	}
	
// process red channel and get gabor filter response length to determine
// the full length of all responses and allocate the to be returned vector
	len = 0;
	strcpy( basename, dirStr );
	strcat( basename, "red" );
	response = ProcessChannel( pixels[0], h, w, response, &len, 0, basename );
	*respLen = 3 * len;	
// process green channel
	strcpy( basename, dirStr );
	strcat( basename, "green" );
	ProcessChannel( pixels[1], h, w, response, &len, len, basename );
// process blue channel
	strcpy( basename, dirStr );
	strcat( basename, "blue" );
	ProcessChannel( pixels[2], h, w, response, &len, len+len, basename );

// dispose of pixel storage	
	for ( i = 0; i < 3; i++ )
		DisposeMatrix( pixels[i], h );
	delete[] pixels;

#else	// USING GRAYSCALE

// allocate pixels for rgb matrix
	float** pixels = CreateMatrix( (float)255.0, h, w );

//  convert rgb info to grayscale
	for ( i = 0; i < h; i++ )
	{
		for ( j = 0; j < w; j++ )
		{
			pixels[i][j] = sqrt( (float)( rgb[0][i][j]*rgb[0][i][j] +
										  rgb[1][i][j]*rgb[1][i][j] + 
										  rgb[2][i][j]*rgb[2][i][j] ) ) / sqrt( 3.0 );
		}
	}

// process grayscale pixels, get gabor filter response length and allocate the return vector
	len = 0;
	strcpy( basename, dirStr );
	strcat( basename, "gf" );
	response = ProcessChannel( pixels, h, w, response, &len, 0, basename );
	*respLen = len;

// dispose of pixel storage	
	DisposeMatrix( pixels, h );

#endif
	
	return response;
}


// process a single color or grayscale channel
float* ProcessChannel( float** image, int h, int w, float* response, int* len, int offset, char* file )
{
	LogPolar*		logPolar = NULL;
	ContrastFilter*	contrastFilter = NULL;
	int				height = h;
	int				width = w;
	float** 		pixels;
	int				gflen;

// copy pointer
	pixels = image;

#if kUseContrast
// apply contrast filter to image
	contrastFilter = new ContrastFilter( image, height, width );
	if ( kSaveFilter == 1 )
	{
		contrastFilter->SetFileName( file );	
		contrastFilter->Save();					// save contrast image
	}
	pixels = contrastFilter->GetContrast();		// get contrast map
	width  = contrastFilter->GetWidth();		// obtain contrast dimensions
	height = contrastFilter->GetHeight();
#endif
	
#if kUseLogPolar
// apply log-polar filter to image
	int	minHW;
	if ( height <= width )
		minHW = height;
	else
		minHW = width;
	logPolar = new LogPolar( pixels, height, width, minHW, height/2, width/3 );
	if ( kSaveFilter == 1 )
	{
		logPolar->SetFileName( file );
		logPolar->Save(kSaveFilter);				// save contrast image
	}
	pixels = logPolar->GetPolars();		// get contrast map
	width  = logPolar->GetWidth();		// obtain polar dimensions
	height = logPolar->GetHeight();
#endif

// initialize gabor jet
	GaborJet gaborJet;
	if ( kSaveFilter == 1 ) gaborJet.SetFileName( file );
	gaborJet.Initialize( height, width, gy, gx, sy, sx, s, f, u, l, a );
	
// filter image
	// response vector is initialized here, but needs to be disposed by user
	gaborJet.Filter( pixels, &gflen );
	if ( *len == 0 ) 
	{
		*len = gflen;
	#if kUsingColor	// USING COLOR
		response = new float[(*len)*3]; // make room for R,G, and B channels
	#else
		response = new float[(*len)];
	#endif
	}
	for ( int i = 0; i < *len; i++ ) response[i+offset] = gaborJet.GetResponse(i);

	delete logPolar;
	delete contrastFilter;

	return response;
}


// give the user a clue
void Usage( void )
{
    cerr << "Usage: gabor (-OPTIONS) <image files>" << endl;
    cerr << "    -h = display this help and exit" << endl;
    cerr << "    -X = horizontal size of filter" << endl;
    cerr << "    -Y = vertical size of filter" << endl;
    cerr << "    -x = horizontal spacing of filter" << endl;
    cerr << "    -y = vertical spacing of filter" << endl;
    cerr << "    -s = sigma modulator" << endl;
    cerr << "    -a = number of orientations" << endl;
    cerr << "    -f = number of frequencies" << endl;
    cerr << "    -l = minimum frequency value" << endl;
    cerr << "    -u = maximum frequency value" << endl;
    cerr << "    -v = turn on/off verbosity" << endl;
    cerr << "    -S = save intermediate files" << endl;    
	exit(0);
}

