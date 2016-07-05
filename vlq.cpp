/*
 * VLQ [Variable Length Quantity] (C++ Version)
 * 
 * Description:
 *     Allows storage of an int of any size by using 7 bits
 *     of a byte to store number information, then using the
 *     last bit to tell whether to use another byte to store
 *     more information. This was made popular by MIDI files.
 *
 * Notes:
 *     This is for a Windows game. As such, it should only
 *     be compiled for Windows. Use Visual Studio C++ 2010.
 *
 * Author:
 *     Clara Van Nguyen
 *
 * Copyright DERPG 2007-2016, All Rights Reserved. This
 * library is made for Project RX and other DERPG Software
 * and should not be used in any other software.
 */

#include "vlq.hpp"

unsigned char VLQ::bit_num(unsigned long long data) {
	//We want to support as much as 64 characters
	unsigned char count = 0;
	unsigned long long tmp_data = data;

	while (data != 0)
		data >>= 7, count++;

	return count + (1 * count == 0);
}

void VLQ::write_int(FILE* fp, unsigned long long data) {
	//Now for the fun part.
	//VLQ - Variable Length Quanity. Uses as many bytes as possible to store an integer.
	unsigned char vlq_size   = bit_num(data),
				  vlq_sizeat = vlq_size;
	unsigned char* vlq_stream = (unsigned char *) malloc(sizeof(char) * vlq_size);
	unsigned long long data_cpy = data;

	do {
		vlq_stream[vlq_sizeat-- - 1] = (data_cpy & 0x7F) + (0x80 * (data_cpy != data));
		data_cpy >>= 7;
	} while (data_cpy != 0);

	fwrite(vlq_stream, sizeof(char), vlq_size, fp);
	free(vlq_stream);
}
	
unsigned long long VLQ::read_int(FILE* fp) {
	unsigned char data;
	unsigned long long ret_val = 0;
	unsigned int pos = 0;
	do {
		fread(&data, sizeof(char), 1, fp);
		ret_val = (ret_val << 7) + (data & 0x7F);
	} while (data > 0x7F);

	return ret_val;
}