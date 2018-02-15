/*
 *  ClassEncoder.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */
#ifndef ClassEncoder_H_
#include "ClassEncoder.h"
#endif

#ifndef AC_HEADER
#include "AC.h"
#endif
#include <algorithm>
#include <math.h>
#include <bitset>

#define logX(x, y) log((double) x)/log((double) y)
#define log2(x) log((double) x)/log(2.0)

ofstream csvfileout;							// stream for csv file

/* Constructor */
ClassEncoder::ClassEncoder()
{
	//MAX_SYMBOL = 0;
}

/* Destructor */
ClassEncoder::~ClassEncoder()
{
}

//----------------------------------Multibeam -------------------------------------------------
int ClassEncoder::Multibeam_split(string filename)
{
	#ifdef DEBUG
	cout << "Doing Multibeam reading and split............ " << endl;
    #endif

	//-----------------memory allocation-------

	unsigned char *buffer_front_row, *buffer_last_row;
	if( (buffer_front_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}
	if( (buffer_last_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	BeamPtrs = new unsigned char* [N*(N-1)];

	unsigned int buffersize  = (width + 2*N*Distance)*(ROWBUFFER+1);

	char *data2 = new char[buffersize];								// total data required for all pointers
	unsigned int stride1 = (buffersize/(N*(N-1))) ;  cout << "stride length for beam pointers: "<<  stride1 << endl; 
	unsigned int stripe_length = 0;

	if( (RLE_histo = (unsigned int *) calloc(stride1, sizeof(int))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for RLE_histo... Please check memory and try again later..." << endl;
		exit(-1);
	}

	for (size_t i = 0; i < N*(N-1); i++)
	{
		unsigned int q =  i * stride1;
		BeamPtrs[i] = (unsigned char*)data2 + q;                  // setting the values of rowPtrs       
 	}
	
#ifdef Compress

	RLEPtrs = new unsigned char* [N*(N-1)];
	for (size_t i = 0; i < N*(N-1); i++)
	{
		RLEPtrs[i] = NULL;                  // setting the values of rowPtrs       
 	}
#endif
	//char *data3 = new char[buffersize/2];								// total data required for all pointers
	//int stride2 = (buffersize/(N*(N-1)))/2 ;  cout << "stride length for RLE pointers: " << stride2 << endl; 
	//for (size_t i = 0; i < N*(N-1); i++)
	//{
	//	unsigned int q =  i * stride2;
	//	RLEPtrs[i] = (unsigned char*)data3 + q;                  // setting the values of rowPtrs       
 //	}

//------------------------------------------------------loop start----------------------------------------------------


	unsigned int new_buffer_length = ROWBUFFER; 
	
	unsigned int *count_beam = NULL;
	count_beam = new unsigned int[N*(N-1)];								//allocate memory for count_beam

	for(unsigned int i=0;i < (N*(N-1));i++)
		count_beam[i] = 0;

	int sign = 1;															//sign for handling traversal in Y-direction 
	int temp = 0;
	int edgeflag = 0;
	div_t divresult;

#ifdef FRAME_PREDICTION
	//unsigned int Frames[2][N*(N-1)];					//declare frames
	unsigned int *Frames[2];
	
	Frames[0] = new unsigned int[N*(N-1)];
	Frames[1] = new unsigned int[N*(N-1)];

	for (unsigned int i=0;i<(N*(N-1));i++)				//Initialize frame values to zero
	{
		Frames[0][i] = 0;
		Frames[1][i] = 0;
	}
#endif

#ifdef write_single_file

		csvfileout.open("compressed.csv");
		
#endif
	
	for(unsigned int row_number=0; row_number<height;)               //reading number of rows starting from row_number
	{   
		if ((row_number + ROWBUFFER) < height)
			ReadRows(row_number,ROWBUFFER);                           
		else
		{  
			new_buffer_length = (height - row_number);               // check and modify for last chunk of row buffer
            ReadRows(row_number,new_buffer_length);
			//cout << "fine" << endl;
		}
		
//#ifndef two		
		for(int x = 0,y=0,z=(ROWBUFFER-1); (x < (width + (N-1)*Distance));)				// loop for N*(N-1) beams traversal
		{
			if(N ==2 && x==0 && y==0) x =-1;										// edge case for 2x1 case
			if(y == Distance)														// case to include the front (top) row
			{
				for(int i=0; i<(N-1) ;i++)															//loop for top row
				{
					if(((x - i*Distance) < 0) || ((x - i*Distance) >= width))
					{
#ifdef FRAME_PREDICTION
						if((i+(N-1)*(N-1))%(N-1) != 0)
						{
							temp = 0 - Frames[0][i + (N-1)*(N-1) - 1];
							//temp = 0;
							if(temp < 0)
								temp = -2*temp - 1;
							else
								temp = 2*temp;

							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = temp;
						}
						else
							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = 0;

						Frames[0][i+(N-1)*(N-1)] = Frames[1][i+(N-1)*(N-1)];
						Frames[1][i+(N-1)*(N-1)] = 0;
#else
						(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = 0;
#endif
					}
					else
					{
#ifdef FRAME_PREDICTION
						if((i+(N-1)*(N-1))%(N-1) != 0)
						{
							temp = ((buffer_front_row[x - i*Distance])/DIV)  - Frames[0][i + (N-1)*(N-1) - 1];
							if(temp < 0)
								temp = -2*temp - 1;
							else
								temp = 2*temp;

							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = temp;
						}
						else
						{
							/*divresult = div(buffer_front_row[x - i*Distance],DIV);
							if(divresult.rem <= (DIV/2))
								(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = divresult.quot;
							else
								(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = divresult.quot + 1;*/

							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = (buffer_front_row[x - i*Distance])/DIV;
						}
						Frames[0][i+(N-1)*(N-1)] = Frames[1][i+(N-1)*(N-1)];
						/*divresult = div(buffer_front_row[x - i*Distance],DIV);
						if(divresult.rem <= (DIV/2))
							Frames[1][i+(N-1)*(N-1)] = divresult.quot;
						else
							Frames[1][i+(N-1)*(N-1)] = divresult.quot + 1;*/

						Frames[1][i+(N-1)*(N-1)] = (buffer_front_row[x - i*Distance])/DIV;
#else
						/*divresult = div(buffer_front_row[x - i*Distance],DIV);
						if(divresult.rem <= (DIV/2))
							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = divresult.quot;
						else
							(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = divresult.quot + 1;*/
						
						(*(BeamPtrs[i + (N-1)*(N-1)]+count_beam[i + (N-1)*(N-1)])) = (buffer_front_row[x - i*Distance])/DIV;
#endif
					}
				}

				for(int j = 0; j < (N-1) ;j++)						// loop for beam array rows
				{
					for(int i = 0; i<(N-1) ; i++)					// loop for beam array columns
					{
						if(((x - i*Distance) < 0) || ((x - i*Distance) >= width) || ((z - j*Distance) >= new_buffer_length))
						{
#ifdef FRAME_PREDICTION
							if((i+j*(N-1))%(N-1) != 0)
							{
								temp = 0 - Frames[0][i + j*(N-1) - 1];
								//temp = 0;
								if(temp < 0)
									temp = -2*temp - 1;
								else
									temp = 2*temp;

								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = temp;
							}
							else
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = 0;

							Frames[0][i+j*(N-1)] = Frames[1][i+j*(N-1)];
							Frames[1][i+j*(N-1)] = 0;
#else
							(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = 0;
#endif
						}
						else
						{
#ifdef FRAME_PREDICTION
							if((i+j*(N-1))%(N-1) != 0)
							{
								temp = ((*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV) - Frames[0][i + j*(N-1) - 1];
								if(temp < 0)
									temp = -2*temp - 1;
								else
									temp = 2*temp;

								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = temp;	
							}
							else
							{
								/*divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
								if(divresult.rem <= (DIV/2))
									(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot;
								else
									(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot + 1;*/
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
							}

							Frames[0][i+j*(N-1)] = Frames[1][i+j*(N-1)];
							/*divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
							if(divresult.rem <= (DIV/2))
								Frames[1][i+j*(N-1)] = divresult.quot;
							else
								Frames[1][i+j*(N-1)] = divresult.quot + 1 ;*/

							Frames[1][i+j*(N-1)] = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
#else
							/*divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
							if(divresult.rem <= (DIV/2))
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot;
							else
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot + 1;*/
							
							(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
#endif
						}
					}
				}
				

				if((y+1)%(Distance+1) == 0)						
					sign = (-1)*sign;
				if(y==0)
					sign = 1;

				 y = y + (sign)*1 ;						// update y by +-1  based on sign 	
				 z = z - (sign)*1 ;						// update z based on sign 

				 if(y%Distance == 0)								// update x 
					 x = x + (Distance/2) - 1;
				 else
					 x = x + (Distance/2);

				 for(unsigned int i=0;i < (N*(N-1));i++)
						count_beam[i]++;

				 /*if(new_buffer_length==1)
				 {
					cout << z << '\t' << count_beam[0] <<'\t'<< x << endl;
				 }*/
			}

//----------------------------------------------Normal case---------------------------------------------------------
			else					//normal case
			{	
				for(int j = 0; j < N ;j++)				// loop for beam array rows
				{
					for(int i = 0; i<(N-1) ; i++)				// loop for beam array columns
					{
						if(((x - i*Distance) < 0) || ((x - i*Distance) >= width) || ((z - j*Distance) >= new_buffer_length))
						{
#ifdef FRAME_PREDICTION
							if((i+j*(N-1))%(N-1) != 0)
							{
								temp = 0 - Frames[0][i + j*(N-1) - 1];
								//temp = 0;
								if(temp < 0)
									temp = -2*temp - 1;
								else
									temp = 2*temp;
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = temp;
							}
							else
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = 0;

							Frames[0][i+j*(N-1)] = Frames[1][i+j*(N-1)];
							Frames[1][i+j*(N-1)] = 0;
#else
							(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = 0;
#endif
						}
						else
						{
#ifdef FRAME_PREDICTION
							if((i+j*(N-1))%(N-1) != 0)
							{
								temp = ((*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV) - Frames[0][i + j*(N-1) - 1];
								if(temp < 0)
									temp = -2*temp - 1;
								else
									temp = 2*temp;
								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = temp;
							}
							else
							{
								/*divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
								if(divresult.rem <= (DIV/2))
									(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot;
								else
									(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot + 1;*/

								(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
							}

							Frames[0][i+j*(N-1)] = Frames[1][i+j*(N-1)];
							/*divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
							if(divresult.rem <= (DIV/2))
								Frames[1][i+j*(N-1)] = divresult.quot;
							else
								Frames[1][i+j*(N-1)] = divresult.quot + 1 ;*/

							Frames[1][i+j*(N-1)] = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
#else
							//divresult = div(*(rowPtrs[z - j*Distance] + x - i*Distance),DIV);
							//if(divresult.rem <= (DIV/2))
							//	(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot;
							//else
							//	(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = divresult.quot + 1;
							(*(BeamPtrs[i + j*(N-1)]+count_beam[i + j*(N-1)])) = (*(rowPtrs[z - j*Distance] + x - i*Distance))/DIV;
#endif
						}
					}
				}
				

				if((y+1)%(Distance+1) == 0)
					 sign = (-1)*sign;
				if(y==0)
					sign = 1;

				y = y + (sign)*1 ;						// update y by +-1  based on sign 	
				z = z - (sign)*1 ;						// update z based on sign 

				if(y%Distance == 0)								// update x 
					 x = x + (Distance/2) - 1;
				else
					 x = x + (Distance/2);

				for(unsigned int i=0;i < (N*(N-1));i++)
						count_beam[i]++;
				
				/*if(new_buffer_length==1)
				{
					cout << z << '\t' << count_beam[0] <<'\t' << x << endl;
				}*/
			}
			//cout << y << '\t';
		}     
//#endif

		if(ROWBUFFER == new_buffer_length)
			memcpy(buffer_front_row,rowPtrs[ROWBUFFER-1],width*sizeof(char));

//#ifdef write_file
		for(int i=0;i < N*(N-1); i++)									// write all beams to seperate files
		{
			if(row_number==0)												
				WriteBeam(filename,i,count_beam[i],1,0);				// start of writing file
			else
				WriteBeam(filename,i,count_beam[i],1,1);

		}
//#endif	

		if(row_number == 0)
		{
			cout << "stripe length (count_beam): " << count_beam[0] << endl;
			stripe_length = count_beam[0];
		}

#ifdef Compress
		for(int i=0;i < N*(N-1); i++)									// write all beams to seperate files
		{
			if(row_number==0)												
				EntropyEncoder_RLE(filename,i,count_beam[i],0);				// start of writing file
			else
				EntropyEncoder_RLE(filename,i,count_beam[i],1);
			
			Uncompressed_Beamlength[i] = Uncompressed_Beamlength[i] + count_beam[i];
			count_beam[i] = 0;
		}

#else
		for(int i=0;i < N*(N-1); i++)									// write all beams to seperate files
		{
			Uncompressed_Beamlength[i] = Uncompressed_Beamlength[i] + count_beam[i];
			count_beam[i] = 0;
		}
#endif
		row_number = row_number + new_buffer_length;                           //update starting row

		if(N ==2 && row_number>=height && edgeflag == 0)				//hack for last row of 2x1 case
		{
			//cout << row_number <<endl;
			edgeflag = 1;
			memcpy(buffer_last_row,rowPtrs[ROWBUFFER-1],width*sizeof(char));
		}
		//cout << row_number << '\t';

	}

//----------------------------------------------------------- end of long loop ----------------------------------------
	int counter = 0;
	if(N==2 && edgeflag == 1)								//hack for last column of 2 beams case
	{
		edgeflag = 0;
		for (int x = 0,y=0; (x < (width + (N-1)*Distance));)
		{
			if(x==0 && y==0)
				x = -1;

			(*(BeamPtrs[0]+counter)) = 0;
			if (y == 2)
			{
				if(x < 0 || x >= width)
				{
					(*(BeamPtrs[1]+counter)) = 0;
				}
				else
				{
					(*(BeamPtrs[1]+counter)) = buffer_last_row[x];
				//	cout << x << '\t';
				}
			}
			else 
			{
				(*(BeamPtrs[1]+counter)) = 0;
			}

			counter++;

			if((y+1)%(Distance+1) == 0)
				sign = (-1)*sign;
			if(y==0)
				sign = 1;

			y = y + (sign)*1 ;	

			if(y%Distance == 0)								// update x 
					 x = x + (Distance/2) - 1;
				else
					 x = x + (Distance/2);

		}
		Uncompressed_Beamlength[0] = Uncompressed_Beamlength[0] + counter;
		Uncompressed_Beamlength[1] = Uncompressed_Beamlength[1] + counter;
		WriteBeam(filename,0,counter,1,1);
		WriteBeam(filename,1,counter,1,1);
	}
	
	// free memory
	if (count_beam != NULL) delete [] count_beam;
#ifdef FRAME_PREDICTION
	if (Frames[0] != NULL) delete [] Frames[0];
	if (Frames[1] != NULL) delete [] Frames[1];
#endif

#ifdef write_single_file
	csvfileout.close();
#endif
	return stripe_length;

}

// Write memory to text
void ClassEncoder::WriteBeam(string filename,unsigned int Beamnumber,unsigned int length,int istext,int append)
{
	
	unsigned int start,end;
	ios_base::openmode mode;
	start = clock();
	filename.erase(filename.end()-4,filename.end());
	
	if (istext)
	{
		for (unsigned int i=0; i<length;i++)
			histograms[Beamnumber][(*(BeamPtrs[Beamnumber]+i))]++;

#ifdef write_single_file
		//csvfileout << Beamnumber <<',';
		for (unsigned int i = 0; i < length;i++)
		{
			csvfileout << int(*(BeamPtrs[Beamnumber] + i)) <<",";
		}
		csvfileout << "\n";
#endif

#ifdef write_file
		char str[15];
		sprintf(str, "%d", Beamnumber);
		filename = filename + "_Beam_" + str + ".txt";
		if (append)
			mode = ios::app;
		else
			mode = ios::out;
		ofstream outTXT(filename, mode);
		
		char *Data = new char[length];
		//Data[length] = 0;
		for (unsigned int i=0; i<length;i++)
		{
			Data[i] = '0' + (*(BeamPtrs[Beamnumber]+i));
			//Data[i] = char(*(BeamPtrs[Beamnumber]+i));
			outTXT << Data[i];
		}

		//outTXT << Data;
		outTXT.close();
	
#endif
	}
	else
	{

		char str[15];
		sprintf(str, "%d", Beamnumber);
		filename = filename + "_Beam_" + str + ".txt";
		ofstream out(filename,ios::binary);
		for(unsigned int i=0;i<length;i++)
		{
			out << (*(BeamPtrs[Beamnumber]+i)) << endl;
			//cout<< RLE[i] << '\t';
		}

		out.close();
	}

	end = clock();
	//cout << "\t" << "\t" << "               Compressed file writing time = " << ((double) (end-start)) / ((double) 1000) << endl;
}

	
// Entropy Encoder: RLE
int ClassEncoder::EntropyEncoder_RLE(string filename,unsigned int Beamnumber,unsigned int length,int append)
{
	filename.erase(filename.end()-4,filename.end());
	//ios_base::openmode mode;

	RLEPtrs[Beamnumber] = new unsigned char[length];

	char str[15];
	sprintf(str, "%d", Beamnumber);
	filename = filename + "_Beam_" + str + "_RLE.txt";
	unsigned int nonzero_count = 0;
	
	Beam_RLE_length[Beamnumber] = 0;							//initialize RLE_length with 0 
	for(unsigned int i=0; i<length; i++)						//loop for Data
	{	
		if(((*(BeamPtrs[Beamnumber]+i)) > 0) && ((*(BeamPtrs[Beamnumber]+i)) < MAX_SYMBOL))				// if symbol is other than zero	
		{
			nonzero_count++;
			(*(RLEPtrs[Beamnumber] + Beam_RLE_length[Beamnumber])) = (*(BeamPtrs[Beamnumber]+i));		// add symbol to particular beam of RLEPtrs
			Beam_RLE_length[Beamnumber]++;
			//histogram[(*(BeamPtrs[Beamnumber]+i))]++;   
			if (nonzero_count > 1)
				RLE_histo[0]++;							// Run length of zero
			//if(i==length)
			//	RLE_histo[length+1]++;						// if it is end of stripe
		}

		else if((*(BeamPtrs[Beamnumber]+i)) == 0)
		{
			nonzero_count = 0;
			unsigned int count = 0;
			while((*(BeamPtrs[Beamnumber]+i)) == 0)
			{
				count ++;
				i++;
			}
			//cout <<"count: " <<count <<endl;
			//cout << "RLE: " << RLE_histo[count]++ << endl;
			RLE_histo[count]++;
			if(count > 0)
			{
				i--;
				unsigned int repeat = (unsigned int) (logX(count, M));
				for(unsigned int r=0; r<=repeat; r++)
				{
					(*(RLEPtrs[Beamnumber] + Beam_RLE_length[Beamnumber])) = MAX_SYMBOL + (count % M);
					Beam_RLE_length[Beamnumber]++;
				//	histogram[MAX_SYMBOL + (count % M)]++;
//					count /= M;
					count >>= RSHIFT_M;		 // If M is 2^N, use N bit right shift instead.

				}
			}
		}
		
	}
	
		(*(RLEPtrs[Beamnumber] + Beam_RLE_length[Beamnumber])) = MAX_SYMBOL + M + 1;
		Beam_RLE_length[Beamnumber]++;
		histogram[MAX_SYMBOL + M + 1]++;
		Total_Beam_RLE_length[Beamnumber] = Total_Beam_RLE_length[Beamnumber] + Beam_RLE_length[Beamnumber] ;
        

#ifdef C_File_Write
		
		if (append)
			mode = ios::app;
		else
			mode = ios::out;
		ofstream outTXT(filename, mode);

		char *Data = new char[Beam_RLE_length[Beamnumber]];
		//Data[length] = 0;
		for (unsigned int i=0; i<Beam_RLE_length[Beamnumber];i++)
		{
			Data[i] = '0' + (*(RLEPtrs[Beamnumber]+i));
			//Data[i] = char(*(BeamPtrs[Beamnumber]+i));
			//outTXT << Data[i];
		}

		outTXT << Data;
		outTXT.close(); 
		
#endif
		if(RLEPtrs[Beamnumber] != NULL) delete [] RLEPtrs[Beamnumber];
		return 0;
}

double ClassEncoder::Entropy(int beamnumber)
{
	unsigned int *p = (unsigned int *) calloc(Z, sizeof(unsigned int));
	unsigned long sum = 0;
	double entropy = 0;
	double *prob = (double *)calloc(Z, sizeof(double));

	if(beamnumber < 0)
	{
		for (int i =0; i<=(Z-1);i++)
		{
			sum += histogram[i];
			p[i] = histogram[i];
		}
	}
	else
	{
		for (int i =0; i<=(Z-1);i++)
		{
			sum += histograms[beamnumber][i];
			p[i] = histograms[beamnumber][i];
		}
		//cout << "sum: " << sum << endl;
	}

	for (int i =0; i<=(Z-1);i++)
	{
		prob[i] = (double) p[i] / (width*height);
		if(p[i] != 0)
			entropy -= prob[i] * log2(prob[i]);
	}
	free(prob);
	free(p);
	return entropy;
}

int ClassEncoder::EntropyEncoder_AC(string filename)						//useless right now
{
	/* Arithmetic Coding */
	// Initialize Arithmetic Coding
	unsigned int ac_bytes = 0;	// Size of the compressed file
	unsigned int ac_bits = 0;
	filename += ".enc";
	ac_encoder ace;
	ac_model acm;
	ac_encoder_init (&ace, filename.c_str());
	
	// Encode each symbol using AC
	ac_model_init (&acm, MAX_SYMBOL+M+K+1, NULL, 1);
	for(unsigned int j=0; j<RLE_length; j++)
	{
		ac_encode_symbol(&ace, &acm, RLE[j]);
	}
	ac_encode_symbol(&ace, &acm, EOF_SYMBOL);
	// Finalize Arithmetic Coder
	ac_encoder_done (&ace);
	ac_model_done (&acm);
	ac_bits = ac_encoder_bits (&ace);
	if ((ac_bits%8) !=0)
		ac_bytes = (int) ((ac_bits/8)+1);
	else
		ac_bytes = (int) (ac_bits/8);
	
#ifdef DEBUG
	cout << "                                [Done]" << endl;
	//cout << "Compressed File Size after AC Encoding in byte = " << ac_bytes << endl;
	cout << "Compressed File Size after AC Encoding in bits = " << ac_bits << endl;
#endif
	cout << "RLE length               : " << RLE_length << endl;
	cout << "Arithmatic encoded bytes : " << ac_bytes << endl;
	cout.precision(15);
	double c_ratio;
	c_ratio = double(width)*double(height)/double(ac_bytes+12);
	cout << "\t" << "approx. compression ratio : " << c_ratio << endl;
	return ac_bytes;
}


int ClassEncoder::deflate_compression(string filename)
{
	//filename +=".txt";
	string filename_dest = filename + ".dft"; 
	int ret, flush;
	unsigned int i,count =0;
    unsigned have;
    z_stream strm;
    unsigned char in[32768];
    unsigned char out[32768];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFLATED);
    if (ret != Z_OK)
        return ret;
	FILE *dest;
	//FILE *source;
	//source = fopen(filename.data(),"r");
	dest = fopen(filename_dest.data(),"wb");
    /* compress until end of file */
    do {
        //strm.avail_in = fread(in, 1, 32768, source);
		count++;
		
		for (i =(count-1)*32768; i<(count*32768) && i<RLE_length;i++)
		{
			in[i-(count-1)*32768] = unsigned char(RLE[i]) ;
		}
		strm.avail_in = i - (count-1)*32768;
		
        /*if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }*/
		if((i)%(RLE_length)==0) {
			flush = Z_FINISH;
		}
		else flush = Z_NO_FLUSH;
       // flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;

        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = 32768;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = 32768 - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when last data in file processed */
    } while (flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&strm);
	//fclose(source);
	fclose(dest);
	return 0;
}

