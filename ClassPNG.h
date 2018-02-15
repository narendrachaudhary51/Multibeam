/*
 *  ClassPNG.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassPNG_H_               // Defining ClassPNG_H if not defined earlier
#define ClassPNG_H_

#ifndef Parameters_H_            // including parameters.h if not included earlier
#include "Parameters.h"
#endif

#include <string>
#include <vector>

#ifdef WIN32                     // include png.h and zlib.h library if windows environment
#include <png.h>
#include <zlib.h>
#pragma comment(lib, "libPNG.lib")
 //#pragma comment(lib, "libtiff.lib")
//#else
//#include "/Users/xosh/Documents/Projects/Library/tiff-3.9.4/libtiff/tiffio.h"
#endif

using namespace std;

class ClassPNG
{
public:
	unsigned int width;
	unsigned int height;
	unsigned int bitdepth;
	unsigned int channels;
	unsigned int row_bytes;                  // number of bytes in a row 
	double png_time;

	png_structp pngPtr;                      // structure pointer for libpng 
	png_infop infoPtr;                       // info pointer for libpng

	unsigned short *RLE;                     // pointer for run length encoding
	size_t RLE_length;
	size_t Beam_length;   

	char *data1;

	png_bytep *rowPtrs;								// row pointers
	//png_bytep rowPtr;

	unsigned char **BeamPtrs;						// beam pointers
	unsigned char **RLEPtrs;						// RLE data pointers

	//unsigned int Beam_RLE_length[N*(N-1)];			// RLE_length's array
	unsigned int *Beam_RLE_length;

	//unsigned int Total_Beam_RLE_length[N*(N-1)];			// RLE_length's array
	unsigned int *Total_Beam_RLE_length;

	//unsigned int Uncompressed_Beamlength[N*(N-1)];
	unsigned int *Uncompressed_Beamlength;

	unsigned int histogram[256];						// one historgram for entire data
	unsigned int *RLE_histo;							// histogram of run lengths
	unsigned int **histograms;


	bool is_read;
	
	ClassPNG();
	~ClassPNG();

	// Read PNG image to memory
	void ReadPNG(string filename);

	// Read number of rows of image
	void ReadRows(unsigned int row_number,unsigned int nRows);

	// Write number of rows of image
	void WriteRows(unsigned int row_number,unsigned int nRows);

	// Write memory to TIFF image
	void WritePNG(string filename);

	// Free memory
	void Free();

};

#endif