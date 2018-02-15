/*
 *  ClassDecoder.cpp
 *  
 *
 *  Created by Narendra Chaudhary on 2/27/2015.
 *  Texas A&M University.
 *
 */

#ifndef ClassDecoder_H_
#include "ClassDecoder.h"
#endif
#include <fstream>
#ifndef AC_HEADER
#include "AC.h"
#endif
#include <bitset>
#include <math.h>
#define logX(x, y) log((double) x)/log((double) y)
#define log2(x) log((double) x)/log(2.0)

/* Constructor */
ClassDecoder::ClassDecoder()
{
}

/* Destructor */
ClassDecoder::~ClassDecoder()
{
}

                                        
int ClassDecoder::Transform_RLE_EOB_decoding()
{
	char *current_row;
	unsigned char *buffer_previous_row;
	if( (buffer_previous_row = (unsigned char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for previous row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	if( (current_row = (char *) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for current row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}

	/*if( (rowPtr = (png_bytep) calloc(width, sizeof(char))) == NULL)
	{
		cout << endl << "[Error] Cannot allocate memory for png row buffer... Please check memory and try again later..." << endl;
		exit(-1);
	}*/
	unsigned int Rows = height;
	if (Rows > WRITE_ROWBUFFER) 
		Rows = WRITE_ROWBUFFER;

	rowPtrs = new png_bytep[Rows];                                   // allocating rowPtrs memory for first iteration of writing
	data1 = new char[width * Rows * bitdepth / 8];
	int stride = width * bitdepth / 8;
	for (size_t i = 0; i < Rows; i++)
	{
		png_uint_32 q =  i * stride;
		rowPtrs[i] = (png_bytep)data1 + q;                  // setting the values of rowPtrs     
	}

	unsigned long long idx = 0, image_size = ((unsigned long long)width)*((unsigned long long)height);
	unsigned int count = 0,count_row =0, count_edge = 0;
	unsigned int index = 0;
	unsigned int exponent = 1, start_row = 0;
	char temp = 0;
#ifdef PAETH
	int pb,pc,pd,predict;				// variables for PAETH filter implementation
#endif

	for (size_t i=0; i < RLE_length; i++)
	{

		if (RLE[i] < MAX_CORNER_SYMBOL)
		{
			temp = (char) RLE[i];
#ifndef PAETH
			if ((temp%2)==0) temp = temp/2;
			else temp = -((temp+1)/2);
#endif
			//index = idx % width;
			//if ((index) == (width-1))
			current_row[idx % width] = temp;
			if ((idx % width) == (width-1))                        // if reached at the end of row start corner decoding and write to file
		    {
				count_edge++;
				for (unsigned int j=0;j<width;j++)                // Inverse corner transform
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
							//if (idx<width)predict=0;
							//else{
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							//}
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;;
#endif
						}
			        }
				      memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));         // put row data into previous row

#ifdef POSTPROCESS                                                                 // do postprocessing of pxels if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                 // write row to PNG file

#endif
	               count_row++;
			       if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)            // writing WRITE_ROWBUFFER rows to PNG filez
			       {
					   if ((count_row % WRITE_ROWBUFFER) ==0)
					   {
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
					   }
					   else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			       }
			}
			
			idx++;
			//cout << "ID of nonzero symbols : " << idx << endl;
			//cout << "temp :" << int(temp) << endl;
		}
		// If RLE of 0 is detected, reconstruct the run of 0s.
		else if(RLE[i] < MAX_CORNER_SYMBOL + M)
		{
			exponent = 1;
			count = 0;
			// Decode run count from M-ary representation
			while(RLE[i] >= MAX_CORNER_SYMBOL && RLE[i] < MAX_CORNER_SYMBOL + M)
			{
				count += exponent * (RLE[i] - MAX_CORNER_SYMBOL);
//				exponent *= M;
				exponent <<= RSHIFT_M;	 // If M is 2^N, use N bit right shift instead.
				i++;
			}
			i--;
			unsigned int x = idx % width; 
			idx += count;
			for (unsigned int j=x;j<(idx % width);j++)                        // write zero's in current row
			{
				current_row[j] = 0;
			}
		}

		// If RLE of EOB is detected, move to end of the block
		else if(RLE[i] < MAX_CORNER_SYMBOL + M + K)
		{
			exponent = 1;
			count = 0;
			// Decode EOB run count from M-ary representation
			while(RLE[i] >= MAX_CORNER_SYMBOL + M && RLE[i] < MAX_CORNER_SYMBOL + M + K)
			{
				count += exponent * (RLE[i] - MAX_CORNER_SYMBOL - M);
//				exponent *= K;
				exponent <<= RSHIFT_K;	 // If K is 2^N, use N bit right shift instead.
				i++;
			}
			i--;
			// Process the run of EOB
			for(unsigned int count_EOB=0; count_EOB<count; count_EOB++)
			{
				unsigned int x = idx % width;
				if(BlockSize * (x / BlockSize) + BlockSize < width) 
				{
					idx += BlockSize - (x % BlockSize);
					for (unsigned int j=x;j<(idx % width);j++)
			        {
						current_row[j] = 0;
			        }
				}
				else
				{
					idx += width - (x % width);
					for (unsigned int j=x;j< width;j++)                              // write zeros till end of block
			        {
						current_row[j] = 0;
			        }
					
					//cout << "count of row : " << count_row << endl;
					for (unsigned int j=0;j<width;j++)                // Inverse corner transform
			        {
						if (j>0)
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + *(rowPtrs[count_row-start_row]+j-1) + buffer_previous_row[j] - buffer_previous_row[j-1];
							//rowPtr[j] = current_row[j] + rowPtr[j-1] + buffer_previous_row[j] - buffer_previous_row[j-1];
#else
							//if (idx<width)predict=0;
							//else{
								predict = buffer_previous_row[j] + *(rowPtrs[count_row-start_row]+j-1) - buffer_previous_row[j-1];
								pb = abs(predict - *(rowPtrs[count_row-start_row]+j-1));                  //distances to  b, c,d
								pd = abs(predict - buffer_previous_row[j]);
								pc = abs(predict - buffer_previous_row[j-1]);
								if(pb <= pd && pb <= pc) predict = *(rowPtrs[count_row-start_row]+j-1);
								else if (pd <= pc) predict = buffer_previous_row[j];
								else predict = buffer_previous_row[j-1];
							//}
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;
#endif
						}
						else
						{
#ifndef PAETH
							*(rowPtrs[count_row-start_row]+j) = current_row[j] + buffer_previous_row[j];
							//rowPtr[j] = current_row[j] + buffer_previous_row[j];
#else						
							predict = buffer_previous_row[j];
							*(rowPtrs[count_row-start_row]+j) = (current_row[j]+predict)%32;;
#endif
						}
			        }

					memcpy(buffer_previous_row,rowPtrs[count_row-start_row],width*sizeof(char));         // put row data into previous row
					//memcpy(buffer_previous_row,rowPtr,width*sizeof(char));              // write current row into previous row

#ifdef POSTPROCESS                                                                      // do postprocessing if enabled
					for (unsigned int j=0, index=0;j<width;j++)
					{
						//if (rowPtr[j] !=0)
						if (*(rowPtrs[count_row-start_row]+j) !=0)
						{ 
							index = int (*(rowPtrs[count_row-start_row]+j));
							*(rowPtrs[count_row-start_row]+j) = *(Maparray+index);
							//index = int (rowPtr[j]);
							//rowPtr[j] = *(Maparray+index);
						}
					}
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#else
					//png_write_row(pngPtr,rowPtr);                                     // write row to PNG file
#endif
					count_row++;
					if ((count_row % WRITE_ROWBUFFER)== 0 || count_row == height)
			        {
						if ((count_row % WRITE_ROWBUFFER) ==0)
						{
						   WriteRows(start_row,WRITE_ROWBUFFER);
						   start_row += WRITE_ROWBUFFER;
						}
					    else
						   WriteRows(start_row,(count_row % WRITE_ROWBUFFER));
			        }
					
                }
				
				//cout << "idx : " << idx << endl;
			}
		}
        
		else
		{
			cout << "[Error] Cannot decode the RLE stream..." << endl;
		}
		
		if(idx > image_size)
		{
			cout << "Out of Bound" << endl;
			break;
		}
		//cout << "count of rows : " << count_row << endl;
		//cout << "ID of nonzero symbols : " << idx << endl;
	}

	png_write_end(pngPtr,infoPtr);                       // end the writing of PNG file

	free(RLE);
	RLE = NULL;
	free(buffer_previous_row);
	free(current_row);
#ifdef DEBUG
	cout << "count of rows : " << count_row << endl;
	cout << "count of edges : " << count_edge << endl;
#endif
	return 0;
}


int ClassDecoder::EntropyDecoder_AC(string filename)
{
	// Run Arithmetic Decoding
#ifdef DEBUG
	cout << "Decoding Arithmetic Codes...... " << endl;
#endif
	unsigned char temp = 0;
	ac_decoder acd;
	ac_model acm;
	filename += ".enc";
	ac_decoder_init (&acd, filename.c_str());
	ac_model_init(&acm, MAX_CORNER_SYMBOL+M+K+1, NULL, 1);
	RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
	//for(size_t i=0; i<RLE_length; i++)
	for(size_t i=0;; i++)
	{
		temp = ac_decode_symbol (&acd, &acm);
		if (temp != EOF_SYMBOL)
			RLE[i] = temp;
		else
		{ 
			cout << "End of file occured" << endl;
			break;
		}
		//RLE[i] = ac_decode_symbol (&acd, &acm);
		//cout << "Decoded RLE symbol : " << RLE[i] << endl;
	}
	ac_decoder_done (&acd);
	ac_model_done (&acm);
#ifdef DEBUG
	cout << "                                [Done]" << endl;
#endif
	return 0;
}

int ClassDecoder::inflate_decompression(string filename)
{
	RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
	filename = filename + ".dft";
	int ret;
    unsigned have;
    z_stream strm;
	//int CHUNK = 32768;
    unsigned char in[32768];
    unsigned char out[32768];
	unsigned int i,count = 0;
	FILE *source;
	source = fopen(filename.data(),"rb");
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, 32768, source);
		//cout << strm.avail_in << endl;
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = 32768;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
			//cout << "ret:" << ret << endl;
            have = 32768 - strm.avail_out;
            /*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }*/
			//cout << "have:" << have << endl;
			for(i = 0; i<have;i++)
			{
				RLE[i + count] = unsigned short(out[i]);
			}
			count = count + have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
	cout << "inflated count : " << count << endl;
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}



void ClassDecoder::ReadRLE(string filename,int flag)
{
	unsigned int start,end;
	start = clock();
	if(flag)
	{
		RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
		filename += ".txt";
		ifstream inTXT;
		inTXT.open(filename);
		char *Data = new char[RLE_length+1];
		inTXT.read(Data, RLE_length+1);
		
		for (int i=0; i<RLE_length;i++)
		{
			RLE[i] = unsigned char (unsigned char(Data[i] - '0') + 128);
			//cout << RLE[i] << '\t' << Data[i] << endl;
		}
		
		inTXT.close();
	}
	else
	{
		RLE = (unsigned short *) calloc(RLE_length, sizeof(unsigned short));
		filename += ".line";
		ifstream in(filename,ios::binary);
		int i = 0;
		
		while ((!in.eof())&&(i<RLE_length))
		{
             std::string inp;
             getline(in,inp);
			 RLE[i] = stoi(inp);
             //cout << RLE[i] << endl;
			 i++;
		}
		
		in.close();
	}
	end = clock();
	cout << "\t" << "\t" << "               Compressed file reading time = " << ((double) (end-start)) / ((double) 1000) << endl;
}