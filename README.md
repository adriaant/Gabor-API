## Gabor API

### Introduction

The Gabor-API is a C++ code library implementation of Gabor Filters. The API provides the necessary routines for the user to apply Gabor filters to images in .PGM format. Optionally, contrast filters and log-polar transform can be applied to the image before Gabor convolution. Additionally, the library comes in two variants. The first variant implements the standard Gabor filter method in which an image is convoluted with a lattice of possibly overlapping banks of Gabor filters at different orientations and frequencies. The second variant convolutes banks of Gabor filters at so-called fiducial points in an image. The coordinations of these fiducial points are supplied in a text file.

### Installation

Unpack the downloaded tar.gz file in the directory of your choosing. To compile the library, navigate to the desired subdirectory `gabor-global/src/` (for the standard Gabor implementation) or `gabor-local/src/` (for the fiducial implementation) and just issue a `make` command. The Makefile located in `gabor-global/` and `gabor-local/` will compile an executable based on the library and the `Gabor.cpp` file, but it needs to be modified first by changing the listed `bin/` directory, in which the compiled executables will be installed. If you are using Project Builder, you will also need to modify the working directory in the Custom Build Command Settings of the four available build targets.

In addition, the `Sample Files/` directory contains test images and a text file containing fiducial points.

### Quickstart

As a quickstart, `cd` to the `Sample Files/` directory and run the sample command:

    gaborglobal -X 8 -Y 8 -x 2 -y 2 -s 3 -a 6 -f 4 -l 0.25 -u 1.5 -v 0 -S 1 lena.ppm

or:

    gaborlocal -s 15 -a 4 -f 2 -l 0.5 -u 0.75 -r 10 -v 0 -S 1 -F face-fiducials.txt face.ppm

Pass a `-h` flag to the executable for an explanation of the command line options. Naturally, a variety of parameter ranges is possible, depending on the nature of the image, the size and spacing of gabor filters, and so on. If the `-S` flag is set to 1, then the gabor filters are themselves saved as images. These images will give a good idea how to change the values for the sigma modulator and the frequencies to get an optimal setting. It is a good idea to play with these values and observe the changes in the produced images.

When using the fiducial implementation of Gabor filters, a file containing the coordinates of the fiducial points must be passed as a mandatory `-F` argument to the executable. This text file should have the number of fiducials listed in the first row, with subsequent rows containing the x and y coordinates. For example:

    # comments follow a hedge. The rest of the line is ignored.
    4	# the number of fiducial points
    # following is a list of coordinates
    186	84
    147	111
    219	112
    150	148

### Usage

`⇒` To use the library as part of a code project, write an interface file containing a function that receives the image data and returns a vector with the Gabor filter responses. For example, if the image data is piped from a camera capture into a vector of RGB value integers, then pass this vector, along with the dimensions of the image and a pointer to an integer holding the length of the response vector to a function:

    float *ProcessImageData( int* rgb, int h, int w, int* respLen )

In this function, convert the integer vector into a matrix of floats and call the function

    float* ProcessChannel( float** image, int h, int w, float* response, int* len )

which is already defined in the `Gabor.cpp` files included in the archive. The `ProcessImageData()` function could be defined as:

    float *ProcessImageData( int *rgb, int h, int w, int* respLen )
    {
        int     i, j, k = 0;
        float*  response = NULL;
        float   norm, max, min;
    
    // allocate pixels for rgb matrix
        float** pixels = CreateMatrix( (float)255.0, h, w );
    
    // in this example, we convert to grayscale
        for ( i = 0; i < h; i++ )
        {
            for ( j = 0; j < w; j++ )
            {
                pixels[i][j] = sqrt( (float)(
                                      rgb[k]*rgb[k] +
                                      rgb[k+1]*rgb[k+1] +
                                      rgb[k+2]*rgb[k+2] ) ) / sqrt( 3.0 );
                k = k + 3;
            }
        }
    
    // process grayscale pixels, get gabor filter response length
    // and allocate the to be returned vector
        *respLen = 0;
        response = ProcessChannel( pixels, h, w, response, respLen );
    
    // scale the responses to 0 and 1
        max = min = response[0];
        for ( i = 1; i < *respLen; i++ )
        {
            if( response[i] > max ) max = response[i];
            if( response[i] < min ) min = response[i];
        }
        norm = max - min;
        for ( i = 0; i < *respLen; i++ )
            response[i] = 1.0 * ( ( response[i] - min ) / norm );
    
    // make sure to clean up the pixel buffer
        DisposeMatrix( pixels, h );
    
    // the response vector needs to be disposed by the caller
        return response;
    }


In the main code, this function could be called as follows:

    float *response = NULL;
    int   *frameBuf = new int[Xdim*Ydim*3];
    int   len;
    CaptureCamera( frameBuf );
    response = ProcessImageData( frameBuf, Ydim, Xdim, &len );
    // do something nice with response
    delete[] response;
    delete[] frameBuf;


The `ProcessChannel()` function implemented in the `Gabor.cpp` files does not require any modification and can be used as is. It is, however, well commented and easy to understand, so that it allows for straightforward adjustment according to user directions.

`⇒` Please observe the following defines:

In `gabor-global/src/GaborJet.h`: `kAngleSeparation`.  
If set to 1, collecting the Gabor filter responses occurs by iterating over angles and frequencies, producing a response vector the length of the product of the sum of angles and the sum of frequencies. It will also generate filtered images for each angle-frequency combination. When set to 0, iteration is over all filter locations, averaging the angle and frequency responses. The length of the response vector in this case is the sum of Gabor filter banks. Only one filtered image is produced.

In `Gabor.cpp`: `kUseLogPolar`, `kUseContrast`, `kUsingColor`  
The first two defines determine whether to apply the Log-Polar transform and/or the Contrast filter. In case of the fiducial implementation, the Log-Polar transform does not apply. Alternatively, one can also specify whether an image's red, green, and blue channels will be filtered separately, or whether the RGB values are first converted to grayscale (default).

### References

*   Atick & Redlich, "[What does the retina know about natural scenes?](http://www.klab.caltech.edu/~pam/nnss/atick8.html)", _Neural Computation_ 4(2), pp. 196-210, 1992\. [Background on the contrast filter]
*   [Hotta](http://www.htlab.ice.uec.ac.jp/~hotta/), [Kurita](http://staff.aist.go.jp/takio-kurita/index.html) & Mishima, "Scale invariant face recognition method using spectral features of log-polar image", _SPIE'99 Conference: Applications of Digital Image Processing XXII_, 1999.
*   [Hotta](http://www.htlab.ice.uec.ac.jp/~hotta/), Mishima, [Kurita](http://staff.aist.go.jp/takio-kurita/index.html) & Umeyama, "[Face matching through information theoretical attention points and its applications to face detection and classification](http://citeseer.nj.nec.com/hotta00face.html)", _Fourth IEEE International Conference on Automatic Face and Gesture Recognition_, pp.34-39, Grenoble, France, March 28-30, 2000.
*   [Sandini](http://www.pspc.dibe.unige.it/~neuroinfo/sandini.html) & Tagliasco, "An anthropomorphic retina-like structure for scene analyisis", _Computer Graphics and Image Processing_ 14, pp. 365-372, 1980\. [Background on the log-polar transform]
*   Turner, M.R. "Texture discrimination by Gabor functions", _Biological Cybernetics_ 55, pp. 71-82, 1986\. [Excellent paper on Gabor filters]

