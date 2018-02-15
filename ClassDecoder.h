/*
 *  ClassDecoder.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */
#ifndef ClassDecoder_H_
#define ClassDecoder_H_

#ifndef Parameters_H_
#include "Parameters.h"
#endif

#ifndef ClassPNG_H_
#include "ClassPNG.h"
#endif

#include <iostream>
#include <string>

using namespace std;

class ClassDecoder: public ClassPNG

{
public:
	ClassDecoder();
	~ClassDecoder();
	
	/* Inverse Transform + RLE + EOB decoding : Corner -> Image */
	int Transform_RLE_EOB_decoding();

	/* Entropy Decoder: AC */
	int EntropyDecoder_AC(string filename);

	//inflate (LZ77+Huffman)
	int inflate_decompression(string filename);


	//Read RLE data from text
	void ReadRLE(string filename,int flag);				// flag 1 for text, 0 for binary
};

#endif