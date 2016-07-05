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

#ifndef __VLQ_CPPHAN__
#define __VLQ_CPPHAN__

#include <cstdio>
#include <cstdlib>

namespace VLQ {
	//These can grow to be unreasonably large. Therefore, we simply
	//declare the largest sized type the compiler supports.

	unsigned char      bit_num  (unsigned long long);
	void               write_int(FILE*, unsigned long long);
	unsigned long long read_int (FILE*);
}

#endif