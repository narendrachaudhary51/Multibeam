/*
 *  Main.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef EndPoints
#include "EndPoints.h"

#endif

/* Main Function */
int main(int argc, char* argv[])
{
	
	string filename;
	EndPoint pts;
	if(argc == 2)
	{
		filename = argv[1];
	}
	else if(argc == 4)
	{
		filename = argv[1];
		pts.width = atoi(argv[2]);
		pts.height = atoi(argv[3]);
		//pts.RLE_Length = atoi(argv[4]);
	}
	else
	{
		cout << "Usage: " << argv[0] << " filename [pattern_name] [width height RLE_Length]" << endl;
   #ifndef TEST
		return 0;
   #endif
	}	

	#ifdef ENCODER
	// Compression
	pts.Compression(filename);

    #ifdef DEBUG                   // debugging
	//cout << "Image Width  = " << pts.width << endl;
	//cout << "Image Height = " << pts.height << endl;
	//cout << "RLE Length   = " << pts.RLE_Length << endl;
	//cout << endl;
    #else
       //	cout << "\t" << pts.width << "\t" << pts.height << "\t" << pts.RLE_Length;
    #endif
    #endif

    #ifdef DECODER
	pts.Decompression(filename);
    #endif
	return 0;
}
