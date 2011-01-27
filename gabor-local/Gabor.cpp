/*
	Description:	Gabor API usage
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-3 Adriaan Tijsseling. All rights reserved.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "GaborGlobal.h" // contains project-wide defines, constants, and globals
#include "GaborJet.h"
#include "ContrastFilter.h"
#include "PGMImage.h"
#include "Utilities.h"

#define kUseContrast	1

// GLOBAL GLOBALS
bool		kSaveFilter = true;	// in case of multiple files, we save GFs only once
bool		kVerbosity  = true; // whether to output any messages or not

// LOCAL GLOBALS
// These are the default settings for the Gabor filter jet
int			gRadius = 32;			//	-r	: radius of filter
float		gS = 10.0;				//	-s	: sigma modulator
int			gA = 4;					//	-a	: number of angles
int			gF = 2;					//	-f	: number of frequencies
float		gL = 0.2;				//	-l	: lower bound of frequency
float		gU = 1.0;				//	-u	: upper bound of frequency
int			gNumLocs = 0;			// number of fiducials
int			**gLocations = NULL;	// coordinates of fiducials
char		gLocationsFile[256];

// PROTOTYPES
float*		ProcessFile( char* file, int*** rgb, int h, int w, float* response, int* respLen );
float* 		ProcessChannel( float** image, int h, int w, float* response, int* len, char* file );
bool 		ReadLocations( void );
void		Usage( void );


// process command line arguments and filter selected files
int main( int argc, char *argv[] )
{
	int		arg;
	char	file[256];
	bool	ok = false;
	
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
			if( strcmp( argv[arg], "-r") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gRadius = atoi( argv[arg] );
				cout << "gRadius" << " " << gRadius << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-s") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gS = atof( argv[arg] );
				cout << "gS" << " " << gS << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-a") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gA = atoi( argv[arg] );
				cout << "gA" << " " << gA << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-f") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gF = atoi( argv[arg] );
				cout << "gF" << " " << gF << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-l") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gL = atof( argv[arg] );
				cout << "gL" << " " << gL << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-u") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				gU = atof( argv[arg] );
				cout << "gU" << " " << gU << endl;
				goto loop;
			}
			if( strcmp( argv[arg], "-F") == 0 )
			{
				arg++;
				if ( argv[arg] == NULL ) Usage();
				strcpy( gLocationsFile, argv[arg] );
				ok = true;
				goto loop;
			}
			if( strcmp( argv[arg], "-v") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
				kVerbosity = (bool)atoi( argv[arg] );
				goto loop;
			}
			if( strcmp( argv[arg], "-S") == 0 )
			{
				cout << argv[arg] << " ";
				arg++;
				if ( argv[arg] == NULL ) Usage();
				cout << argv[arg] << " ";
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
	if ( arg >= argc || ok == false )
	{
		Usage();
		return 0;
	}

	if ( ! ReadLocations() ) return 0;
	
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

	// filter this image
		float* response = NULL;
		int	len = 0;
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
pass the rgb pixel vector for filtering; the response is returned in the response vector. 
This vector will be initialized by the Gabor API, but MUST disposed of it by the user.
respLen is the address of an int holding the length of the response vector. 
*/
float* ProcessFile( char* file, int*** rgb, int h, int w, float* response, int* respLen )
{
	char 	basename[256];
	char	dirStr[256];
	char*	fileStr;
	int		i, j, len;
	float	norm, max, min;

// extract directory path from filename
	strcpy( dirStr, file );
	fileStr = strrchr( dirStr, '/' );
	if ( fileStr != NULL )
	{
		strcpy( basename, fileStr );				// save part after last slash
		dirStr[fileStr-dirStr+1] = '\0';				// save directory string
		strcat( dirStr, basename );
		strcat( dirStr, "-localGF" );
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
		strcat( dirStr, "-localGF" );
	}

// create img directory in extracted path
	if ( kSaveFilter == 1 )
	{
		mkdir( dirStr, S_IRWXU | S_IRWXG );
		if ( kVerbosity ) cerr << "Created directory: \"" << dirStr << "\"" << endl;
		strcat( dirStr, "/" );
	}

// allocate pixels for rgb matrix
	float** pixels = CreateMatrix( (float)255.0, h, w );

//  convert rgb info to grayscale
	for ( i = 0; i < h; i++ )
	{
		for ( j = 0; j < w; j++ )
		{
			if (rgb[0][i][j] == 0) {
				pixels[i][j] = 1;
			} else {
				pixels[i][j] = 0;
			}
//			pixels[i][j] = sqrt( (float)( rgb[0][i][j]*rgb[0][i][j] +
//										  rgb[1][i][j]*rgb[1][i][j] + 
//										  rgb[2][i][j]*rgb[2][i][j] ) ) / sqrt( 3.0 );
			cout << pixels[i][j];
		}
		cout << endl;
	}

// process grayscale pixels, get gabor filter response length and allocate the return vector
	len = 0;
	strcpy( basename, dirStr );

	cout << endl;
	cout << endl;
	for ( i = 0; i < h; i++ )
	{
		for ( j = 0; j < w; j++ )
		{
			cout << pixels[i][j];
		}
		cout << endl;
	}
	response = ProcessChannel( pixels, h, w, response, &len, basename );
	*respLen = len;

// original float values scaled to [0,1]
	if ( kVerbosity ) cerr << "scaling..." << endl;
	max = min = response[0];
	cerr << endl;
	for ( i = 0; i < len; i++ )
	{
		if( response[i] > max ) max = response[i];
		if( response[i] < min ) min = response[i];
	}
	cerr << endl;
	norm = max - min;
	for ( i = 0; i < len; i++ ) {
		response[i] = 1.0 * ( ( response[i] - min ) / norm );
		cerr << response[i] << " ";
	}
	
// dispose of pixel storage	
	if ( pixels != NULL ) DisposeMatrix( pixels, h );
	
	return response;
}


float* ProcessChannel( float** image, int h, int w, float* response, int* len, char* file )
{
	ContrastFilter*	contrastFilter = NULL;
	GaborJet*		gaborJet = NULL;
	int				height = h;
	int				width = w;
	float** 		pixels;
	int				gflen, dummy;
	int				i, j, offset = 0;
	char			filename[256], suffix[5];

// copy pointer
	pixels = image;

#if kUseContrast
// apply contrast filter to image
	contrastFilter = new ContrastFilter( image, height, width );
// set filename if intermediate files should be saved
	if ( kSaveFilter == 1 )
	{
		contrastFilter->SetFileName( file );	
		contrastFilter->Save();					// save contrast image
	}
	pixels = contrastFilter->GetContrast();		// get contrast map
	width = contrastFilter->GetWidth();			// obtain contrast dimensions
	height = contrastFilter->GetHeight();
#endif

// initialize gabor jet for the first fiducial point
	gaborJet = new GaborJet;
	if ( kSaveFilter == 1 )
	{
		strcpy( filename, file );
		sprintf( suffix, "%d-", 0 );
		strcat( filename, suffix );
		gaborJet->SetFileName( filename );
	}
	gaborJet->Initialize( height, width, gLocations[0][0], gLocations[0][1],
						  gRadius, gS, gF, gU, gL, gA, kSaveFilter );
	
// filter image
	// response vector is initialized here, but needs to be disposed by user
	gaborJet->Filter( pixels, &gflen );
	if ( *len == 0 ) 
	{
		*len = gflen * gNumLocs;
		response = new float[(*len)]; // numLocs locations
	}
	for ( i = 0; i < *len; i++ ) response[i+offset] = gaborJet->GetResponse(i);
	delete gaborJet;
	
// we already save the filters for the first fiducial, so turn it off for the others
	kSaveFilter = 0;
	
// process the rest of the fiducial points
	for ( i = 1; i < gNumLocs; i++ )
	{
		offset = offset + gflen;

		gaborJet = new GaborJet;
		if ( kSaveFilter == 1 )
		{
			strcpy( filename, file );
			sprintf( suffix, "%d-", i );
			strcat( filename, suffix );
			gaborJet->SetFileName( filename );
		}
		gaborJet->Initialize( height, width, gLocations[i][0], gLocations[i][1], 
							  gRadius, gS, gF, gU, gL, gA );
		
	// filter image
		// response vector is initialized here, but needs to be disposed by user
		gaborJet->Filter( pixels, &dummy );
		for ( j = 0; j < gflen; j++ ) response[j+offset] = gaborJet->GetResponse(j);
		delete gaborJet;
	}	
	
#if kUseContrast
	delete contrastFilter;
#endif

	return response;
}


// read in patterns from file
bool ReadLocations( void ) 
{
	ifstream infile;
	
// open the file
	infile.open( gLocationsFile );
	if ( infile.fail() )
	{		
		FileOpenError( gLocationsFile );
		return false;
	}

// read the number of patterns
	SkipComments( &infile );
	infile >> gNumLocs;

// create the storage matrix
	gLocations = CreateMatrix( (int)0, gNumLocs, 2 );

// read in the pattern values
	for ( int i = 0; i < gNumLocs; i++ )
	{
		for ( int j = 0; j < 2; j++ )
		{
			SkipComments( &infile );
			infile >> gLocations[i][j];
			cerr << gLocations[i][j] << " ";
		}
	}
	
// close and return
	infile.close();
	return true;
}


void Usage( void )
{
    cerr << "Usage: gabor (-OPTIONS) -F <file> <image files>" << endl;
    cerr << "    -h = display this help and exit" << endl;
    cerr << "    -r = radius of filter" << endl;
    cerr << "    -s = sigma modulator" << endl;
    cerr << "    -a = number of orientations" << endl;
    cerr << "    -f = number of frequencies" << endl;
    cerr << "    -l = minimum frequency value" << endl;
    cerr << "    -u = maximum frequency value" << endl;
	cerr << "    -F = text file with coordinates of fiducials" << endl;
    cerr << "    -v = turn on/off verbosity" << endl;
    cerr << "    -S = save intermediate files" << endl;    
	exit(0);
}
