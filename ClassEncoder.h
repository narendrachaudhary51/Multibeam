/*
 *  ClassEncoder.h
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassEncoder_H_
#define ClassEncoder_H_

#ifndef Parameters_H_
#include "Parameters.h"
#endif

//#ifndef USE_PATTERN_MATCHING
#ifndef ClassPNG_H_
#include "ClassPNG.h"
#endif

#include <iostream>
#include <fstream>
#include <string>
//#include <map.h>

using namespace std;

class ClassEncoder: public ClassPNG

{
public:
	ClassEncoder();
	~ClassEncoder();

	
	int Multibeam_split(string filename);

	
	// Transformation + RLE + EOB: Image -> Corner/Paeth -> RLE+EOB 
	int Transform_RLE_EOB(unsigned char* Map, int No_of_Map);

	// Entropy Encoder: AC
	int EntropyEncoder_AC(string filename);

	// deflate 
	int deflate_compression(string filename);


	// Entropy Encoder: AC
	int EntropyEncoder_RLE(string filename,unsigned int Beamnumber,unsigned int length,int append);

	// Write RLE data to text
	void WriteBeam(string filename,unsigned int Beamnumber,unsigned int length, int istext,int append);           // flag 1 for text, 0 for binary

	// Entropy calculation
	double Entropy(int beamnumber);

};

#endif