/*
 *  EndPoints.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */


#ifndef EndPoints
#include "EndPoints.h"
#endif

#include <ctime>

#ifdef WIN32
#define TIME_NORMALIZER 1000		// For Microsoft Windows Systems
#else
#define TIME_NORMALIZER 1000000		// For Mac OS X/Linux/Unix Systems
#endif


EndPoint::EndPoint()
{
	//passbool = NULL;
}

EndPoint::~EndPoint()
{
}

int EndPoint::Compression(string filename)
{ 
    
#ifndef DEBUG
	cout << filename << endl;
#else
	//cout << "File Name : " << filename << endl;
#endif

	// Start Compression ------------------------------------------------>
	unsigned int start, end;
	double elapsed;
	
	ClassEncoder original;

	start = clock();

	// Read the layout image
	original.ReadPNG(filename);

	// Grab the image dimension
	width = original.width;
	height = original.height;
	bitdepth = original.bitdepth;
	row_bytes = original.row_bytes;

#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << '\t' << '\t' << " Opening and Reading Info Time = " << elapsed << endl;
	start = clock();
#else
	cout << "Image Size = " << ((unsigned long long)original.width) * ((unsigned long long)original.height) << endl;
#endif
	
	original.Uncompressed_Beamlength = new unsigned int[N*(N-1)];

#ifdef Compress
	original.Total_Beam_RLE_length = new unsigned int[N*(N-1)];
	original.Beam_RLE_length = new unsigned int[N*(N-1)];
#endif

	original.histograms = new unsigned int*[N*(N-1)];			//pointers for histograms of indiviual beams
	unsigned int *hist = new unsigned int[Z*N*(N-1)];								// total data required for all pointers
	
	for (size_t i = 0; i < N*(N-1); i++)
	{
		unsigned int q =  i * Z;
		original.histograms[i] = (unsigned int*)hist + q;                  // setting the values of rowPtrs       
 	}

#ifdef Compress
	for (int i = 0; i<N*(N-1);i++)
		original.Total_Beam_RLE_length[i] = 0;
#endif

	for (int i = 0; i<N*(N-1);i++)
	{
		original.Uncompressed_Beamlength[i] = 0;	

		for(int j =0;j<=(Z-1);j++)
			original.histograms[i][j] = 0;
	}

	for(int j =0;j<=255;j++)
		original.histogram[j] = 0;

	unsigned int sum = 0;
	unsigned long sum_un = 0;
	unsigned int Max = 0;
	unsigned int Min = -1;
	unsigned int s_length = 0;

	s_length = original.Multibeam_split(filename);					//split function
	cout << "ok" << endl;

	for (int i = 0; i<N*(N-1);i++)
	{
		if(i == 0)
		{
			cout <<"Beamnumber: " << i <<"   Uncompressed: " << original.Uncompressed_Beamlength[i] ;

#ifdef Compress
			cout << "   Compressed: " <<original.Total_Beam_RLE_length[i] << endl;
#endif
		}
#ifdef Compress
		sum = sum + original.Total_Beam_RLE_length[i];
		
		if (original.Total_Beam_RLE_length[i] > Max)
			Max = original.Total_Beam_RLE_length[i];

		if (original.Total_Beam_RLE_length[i] < Min)
			Min = original.Total_Beam_RLE_length[i];
#endif
		sum_un  = sum_un + original.Uncompressed_Beamlength[i];
	}
	

	for (int i = 0; i<N*(N-1);i++)					// output histograms of indiviual beams
	{
		//cout << endl << "-----------------------------Histogram of beam: " << i << "-----------------------------" << endl; 
		
		for(int j =0;j<=(Z-1);j++)
		{
			//cout << j << '\t' << original.histograms[i][j] << endl; 

//#ifndef Compress
			original.histogram[j] = original.histogram[j] +  original.histograms[i][j] ;
//#endif
		}
		//cout << endl << "-----------------------------Entropy of beam " << i << ": " << original.Entropy(i) << endl; 
	}

	cout << endl << "Histogram of all the beams combined: (for compressed file, ignore for uncompressed)" << endl;
	for(int j =0;j<=(Z-1);j++)
		cout << j << '\t' << original.histogram[j] << endl; 
	
	cout << endl << "Run length statistics" << endl;
	for(int j =0;j<=s_length;j++)
		cout << j << '\t' << original.RLE_histo[j] << endl; 
	//cout << 0 << '\t' << original.RLE_histo[0] << endl;

	cout << endl << "-----------------------------Entropy of total data: " << original.Entropy(-1) << endl;

	cout << endl <<"Total uncompressed file size: " << sum_un << endl;
	cout << "Total compressed file size: " << sum << endl;
	cout << "Max compressed file size: " << Max << endl;
	cout << "Min compressed file size: " << Min << endl;


	
#ifdef DEBUG
	
#else
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "\t" << "\t" << "               Time taken for reading png = " << original.png_time << endl;
	cout << '\t' << '\t' << "                      Total encoding time = " << elapsed << endl;
	
#endif

	// free memory
	if (original.Beam_RLE_length != NULL) delete [] original.Beam_RLE_length;
	if (original.Uncompressed_Beamlength != NULL) delete [] original.Uncompressed_Beamlength;
	if (original.Total_Beam_RLE_length != NULL) delete [] original.Total_Beam_RLE_length;
	if (hist != NULL) delete[] hist;
	if (original.RLE_histo != NULL) delete [] original.RLE_histo;
	if (original.histograms != NULL) delete [] original.histograms;
	original.Free();

	return 0;
}

unsigned char *EndPoint::Decompression(string filename)
{
	#ifdef DEBUG
	cout << "File Name : " << filename << endl;
    #endif
	
	// Start Decompression ---------------------------------------------->
	unsigned int start, end;
	double elapsed;
	
	ClassDecoder corner;
	
	// Initialize Workspace
	corner.width = width;
	corner.height= height;
//	corner.RLE_length = RLE_Length;

	start = clock();

	//corner.EntropyDecoder_AC(filename);
	//corner.inflate_decompression(filename);
	//corner.ReadRLE(filename,0);
	

#ifdef DEBUG
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "AC Decoding Time = " << elapsed << endl;
	start = clock();
	//corner.inflate_decompression(filename);
	end = clock();
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
	cout << "inflate Decoding Time = " << elapsed << endl;
	start = clock();
#endif

	corner.WritePNG(filename);
		// Inverse Transform: Corner -> Image

	//corner.Transform_RLE_EOB_decoding();
	
	end = clock();
	// End Decompression ------------------------------------------------>
	
	// Compute Runtime
	elapsed = ((double) (end-start)) / ((double) TIME_NORMALIZER);
#ifndef DEBUG
	cout << "\t" << "\t" << "               Time taken for writing png = " << corner.png_time << endl;
	cout << "\t" << "\t" << "                      Total decoding time = " << elapsed << endl;
#else
	cout << "RLE + EOB + Inverse Transform + writing file Time = " << elapsed << endl;
#endif

	corner.Free();
	return NULL;
}
