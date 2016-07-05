/*
 * XEN File Manager (C++ Version)
 * 
 * Description:
 *     Manages files in DERPG XEN formats, namely DXEN
 *     and SXEN. This is for Project RX and is to be
 *     used only as a header in DERPG command line tools.
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

#ifndef __XEN_FM_CPPHAN__
#define __XEN_FM_CPPHAN__

//C Includes
#include <cstdio>
#include <cstdlib>

//C++ Includes
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

//Custom Library Includes
#include "vlq.hpp"

using namespace std;

typedef unsigned int       cn_uint;
typedef unsigned char      cn_byte;
typedef unsigned short     cn_short;
typedef unsigned long long cn_ull;
typedef unsigned long long cn_u64;

//Define other things
#define __XEN_VER 3

class XEN {
	public:
		XEN();
		XEN(const char *);

		//Special Declarations
		enum MODE_TYPE { M_SXEN, M_DXEN };
		enum ENC_TYPE  { E_NONE, E_KBH, E_RX };

		//File Subclass
		struct _FILE {
			public:
				_FILE() : size(0), in_xen(false), path(NULL), _BYTE(NULL) {};
				~_FILE();
				cn_uint  size;
				char   * path;
				cn_byte* _BYTE;
				bool     in_xen;
		};

		//Parse Functions
		void parse_file    (const char*);

		//Set Functions
		void set_encryption(const ENC_TYPE&);
		void set_version   (const cn_short&);
		void set_mode      (const MODE_TYPE&);
		void set_tag       (const string&, const string&);
		void clear();

		//Get Functions
		ENC_TYPE            get_encryption();
		cn_short            get_version();
		MODE_TYPE           get_mode();
		string&             get_tag(const string&);
		_FILE&              get_file(const string&, const string&);
		map<string, _FILE>& get_directory(const string&);           //I have no idea why the hell you would need this...

		//Check Functions
		bool tag_exists(const string&);
		
		//Write Functions
		void write_file(const char*);

		//Read Functions
		void read_file(const char*);

	private:
		MODE_TYPE _MODE;    //0 - SXEN; 1 - DXEN
		ENC_TYPE  _ENCRYPT; //Encryption Method (DXEN Only)
		cn_short  _VERSION; //Version of the file

		//Data Structures
		map<string, map<string, _FILE> > _DIRECTORY; //For folders to store files in.
		map<string, string>             _TAG;        //For file information.
		map<string, vector<int> >       _LIST;       //For difficulties.


	protected:
		//TODO: Add shit here.
};

#endif