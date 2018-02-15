/*
 *  ClassPNG.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassPNG_H_
#include "ClassPNG.h"
#endif

#include <iostream>
#include <fstream>
#include <math.h>
#define log2(x) log((double) x)/log(2.0)                   // defining log with base 2

ifstream PNGfile;                        // input stream for reading the file
ofstream PNGfile_out;                    // output stream for writing the file

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length);  //set custom read function using ifstream
void userWriteData(png_structp pngPtr, png_bytep data, png_size_t length);  //set custom write function using ofstream
//void my_png_flush(png_structp png_ptr);

// ---------------------------constructor 
ClassPNG::ClassPNG()
{
	width = 0;
	height = 0;
	bitdepth = 8;
	channels = 1;
	row_bytes = 0;
	RLE_length = 0;
	Beam_length = 0;

	png_time = 0;
	is_read  = 1;
	pngPtr = NULL;
	infoPtr = NULL;
	RLE = NULL;
	rowPtrs = NULL;
	BeamPtrs = NULL;
	RLEPtrs = NULL;
	data1 = NULL;

	Beam_RLE_length = NULL;
	Total_Beam_RLE_length = NULL;
	Uncompressed_Beamlength = NULL;
	histograms = NULL;
}

// Destructor

ClassPNG::~ClassPNG()
{
}


void ClassPNG::ReadPNG(string filename)
{ 
	is_read = 1;
	#ifdef DEBUG                                           // Debugging code
	cout << "Reading the input image ....... " << endl;
	cout << "File Name : " << filename << endl;
    #endif

	data1 = NULL;                                 
	png_byte pngsig[PNGSIGSIZE];                       // PNG signature
	PNGfile.open(filename.c_str(),ios::binary);        // opening file using ifstream

	if (!PNGfile.good())                               // check whether file has opened 
	      {
		      cout << " error in opening the file"<<endl;
			  exit(1);
	      }
	PNGfile.read((char*)pngsig, PNGSIGSIZE);          // read signature of PNG file
          if (!PNGfile.good()) 
		   {
				cout << " error in reading the file"<<endl;
				PNGfile.close();
				exit(1);
	       }
	if((png_sig_cmp(pngsig, 0, PNGSIGSIZE))!= 0)     // verify whether signature represents png file
	     {  
			cout << " Not a PNG file";
			PNGfile.close();
			exit(1);
	     }

	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  //intialize read structure
		if (!pngPtr)
		   {
			    cout << "ERROR: Couldn't initialize png read struct" << endl;
		   }
    infoPtr = png_create_info_struct(pngPtr);                                 //initialize info structure
		if (!infoPtr)
		   {
			    cout << "ERROR: Couldn't initialize png info struct" << endl;
				png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
		   }

	if (setjmp(png_jmpbuf(pngPtr)))                      // checking for error and jumping (read libpng documentation)
		 {
				//An error occured, so clean up what we have allocated so far...
				png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
				if (rowPtrs != NULL) delete [] rowPtrs;
				if (data1 != NULL) delete [] data1;

				cout << "ERROR: An error occured while reading the PNG file" <<endl;

				//Make sure you return here. libPNG will jump to here if something
				//goes wrong, and if you continue with your normal code, you might
				//End up with an infinite loop.
				PNGfile.close();
				exit(1);  
		}

	png_set_read_fn(pngPtr,(png_voidp)&PNGfile, userReadData);      // make userReadData as custom read function

	//Set the amount signature bytes we've already read:
    //We've defined PNGSIGSIZE as 8;
    png_set_sig_bytes(pngPtr, PNGSIGSIZE);

	// read information about image
	png_read_info(pngPtr, infoPtr);
	    width = png_get_image_width(pngPtr, infoPtr);
	         cout << " image width          : " << width << endl;
	    height= png_get_image_height(pngPtr, infoPtr);
		     cout << " image Height         : " << height << endl;
		bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
		     cout << " bitdepth             : " << bitdepth << endl;
		channels   = png_get_channels(pngPtr, infoPtr);
		     cout << " channels             : " << channels << endl;
        row_bytes = png_get_rowbytes(pngPtr,infoPtr);
             cout << " No. of bytes in a row: " << row_bytes << endl;
			// cout << " Color_type : "<< int(png_get_color_type(pngPtr, infoPtr)) <<endl;
			 //cout << " filter method : "<< int(png_get_filter_type(pngPtr, infoPtr)) <<endl;
			 //cout << " compression type : "<< int(png_get_compression_type(pngPtr, infoPtr)) <<endl;
			 //cout << " interlace type : "<< int(png_get_interlace_type(pngPtr, infoPtr)) <<endl;

}

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length) {
    //This is the parameter we passed to the png_set_read_fn() function.
	//Our std::ifstream pointer.
    png_voidp a = png_get_io_ptr(pngPtr);
    //Cast the pointer to std::istream* and read 'length' bytes into 'data'
    ((fstream*)a)->read((char*)data, length);
}

// read number of rows of image

void ClassPNG::ReadRows(unsigned int row_number,unsigned int nRows)
{
    #ifdef DEBUG
	cout << " starting of row   : " << row_number << endl;
	cout << " size of row buffer: " << nRows << endl;
    #endif
	
	if (rowPtrs != NULL) delete [] rowPtrs;
    if (data1 != NULL) delete [] data1;
	
	rowPtrs = new png_bytep[nRows];
	data1 = new char[width * nRows * bitdepth * channels / 8];
	int stride = width * bitdepth * channels / 8;
	for (size_t i = 0; i < nRows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs             
 	}
	unsigned int start, end;
	start = clock();
	png_read_rows(pngPtr, rowPtrs,NULL,nRows);             // reading rows with rowPtrs as pointers to the start of row
	end = clock();
	png_time += ((double) (end-start)) / ((double) 1000);
}


// write to PNG file
void ClassPNG::WritePNG(string filename)
{
	is_read = 0;
	filename.erase(filename.end()-4,filename.end());
	filename = filename + "_Decoded.png";
	#ifdef DEBUG                                           // Debugging code
	cout << "Writing the decoded image ....... " << endl;
	cout << "File Name : " << filename << endl;
    #endif

	PNGfile_out.open(filename.c_str(),ios::binary);        //opening file using ofstream 

	    if (!PNGfile_out.good())                               // check whether file has opened 
		{
			cout << " error in opening the file"<<endl;
			exit(1);
	    }

	pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  //intialize write structure
		if (!pngPtr)
		{
			cout << "ERROR: Couldn't initialize png write struct" << endl;
		}
    infoPtr = png_create_info_struct(pngPtr);                                 //initialize info structure
		if (!infoPtr)
		{
			cout << "ERROR: Couldn't initialize png info struct" << endl;
			png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
		}

        if (setjmp(png_jmpbuf(pngPtr)))                      // checking for error and jumping (read libpng documentation)
		{
				//An error occured, so clean up what we have allocated so far...
				png_destroy_write_struct(&pngPtr, (png_infopp)0);
				if (rowPtrs != NULL) delete [] rowPtrs;
				if (data1 != NULL) delete [] data1;

				cout << "ERROR: An error occured while writing the PNG file" <<endl;

				//Make sure you return here. libPNG will jump to here if something
				//goes wrong, and if you continue with your normal code, you might
				//End up with an infinite loop.
				PNGfile_out.close();
				exit(1);  
		}
    png_set_write_fn(pngPtr,(png_voidp)&PNGfile_out, userWriteData, NULL);      // make userWriteData as custom write function 

    png_set_IHDR(pngPtr, infoPtr, width, height,bitdepth, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	//png_set_tIME(pngPtr, infoPtr,PNG_INFO_tIME);
 
	png_write_info(pngPtr, infoPtr);
}

void userWriteData(png_structp pngPtr, png_bytep data, png_size_t length) {
    //This is the parameter we passed to the png_set__fn() function.
	//Our std::ofstream pointer.
    png_voidp a_out = png_get_io_ptr(pngPtr);
    //Cast the pointer to std::istream* and write 'length' bytes into 'data'
    ((ofstream*)a_out)->write((char*)data, length);
}
/* This is optional but included to show how png_set_write_fn() is called */
//void my_png_flush(png_structp png_ptr)
//{
//}

// write number of rows of image

void ClassPNG::WriteRows(unsigned int row_number,unsigned int nRows)
{
    #ifdef DEBUG
	//cout << " starting of row   : " << row_number << endl;
	//cout << " size of row buffer: " << nRows << endl;
    #endif
	
	unsigned int start, end;
	start = clock();
	png_write_rows(pngPtr, rowPtrs,nRows);             // writing rows with rowPtrs as pointers to the start of row
	end = clock();
	png_time += ((double) (end-start)) / ((double) 1000);

	if (rowPtrs != NULL) delete [] rowPtrs;
    if (data1 != NULL) delete [] data1;
	rowPtrs = new png_bytep[nRows];
	data1 = new char[width * nRows * bitdepth / 8];
	int stride = width * bitdepth / 8;
	for (size_t i = 0; i < nRows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs             
 	}
	
}


//Free memory
void ClassPNG::Free()
{
	#ifdef DEBUG                                           // Debugging code
	cout << "Closing the file and free buffers " << endl;
    #endif
	//delete RLE
//	if (RLE != NULL) delete [] RLE;

	if (BeamPtrs != NULL) delete[] (unsigned char*)BeamPtrs;
	if (RLEPtrs != NULL) delete[] (unsigned char*)RLEPtrs;
	//Delete the row pointers array....
    if (rowPtrs != NULL) delete[] (png_bytep)rowPtrs;
	//if (rowPtr != NULL) delete[] rowPtr;
	if (data1 != NULL) delete [] data1;
	
	if (is_read)
	{
		// clean read and info structures
	png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
	}
	else
	{
		// clean write and info structures
		png_destroy_write_struct(&pngPtr,(png_infopp)0);
	}
	// close stream
	PNGfile.close();
	PNGfile_out.close();
}


