/*
 *  EndPoints.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef EndPoints
#define EndPoints

#ifndef Parameters_H_
#include "Parameters.h"
#endif

#ifndef ClassEncoder_H_
#include "ClassEncoder.h"
#endif

#ifndef ClassDecoder_H_
#include "ClassDecoder.h"
#endif

using namespace std;

class EndPoint
{
public:
	unsigned int width;
	unsigned int height;
	unsigned int bitdepth;
	unsigned int channels;
	unsigned int row_bytes;

	//unsigned short *pass;
	//bool *passbool;
	
	size_t RLE_Length;

	EndPoint();
	~EndPoint();
	
	int Compression(string filename);
	unsigned char *Decompression(string filename);
};

#endif