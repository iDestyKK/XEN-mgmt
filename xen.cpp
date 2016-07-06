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

//Library Include
#include "xen.hpp"

XEN::XEN() {
	//Default Settings
	_ENCRYPT = XEN::E_NONE;
	_MODE    = XEN::M_SXEN;
	_VERSION = __XEN_VER;
	_PATH    = NULL;
}

XEN::XEN(const char* fname) {
	XEN();

	//TODO: Add file importing support
}

XEN::~XEN() {
	if (_PATH != NULL)
		free(_PATH);
}

//Parse Functions
void XEN::parse_file(const char* fname) {
	string cur_dir = "",
	       tmp_str, action, ac_name, parameter;
	istringstream _SS;
	clear();
	ifstream fp(fname); //This disgusts me

	while (getline(fp, tmp_str)) {
		_SS.str(tmp_str);
		_SS >> action;

		/*if (action.substr(0, 2) == "//")
			continue; //This is a comment in the file. Ignore it.*/

		if (action == "TAG") {
			//We are adding a tag in. The next string is the tag name.
			_SS >> ac_name;
			parameter = get_rest_of_stringstream(_SS);

			_TAG.insert(pair<string, string>(ac_name, parameter));
		}
		if (action == "FOLDER") {
			parameter = get_rest_of_stringstream(_SS);
			cur_dir = parameter;
			_DIRECTORY.insert(pair< string, map<string, _FILE> >(
				parameter, 
				map<string, _FILE>()
			));
		}

		if (action == "FILE") {
			parameter = get_rest_of_stringstream(_SS);
			string fname = parameter.substr(parameter.find_last_of("\\") + 1);
			_DIRECTORY[cur_dir].insert(pair<string, _FILE>(
				fname,
				_FILE()
			));

			_FILE* ii = &_DIRECTORY[cur_dir][fname];
			ii->path = strdup(parameter.c_str());
		}

		if (action == "LIST") {
			_SS >> ac_name;
			cur_dir = parameter;
			_LIST.insert(pair< string, vector<int> >(
				ac_name, 
				vector<int>()
			));

			vector<int>* ii = &_LIST[ac_name];
			string ts;
			while (_SS && !_SS.eof()) {
				_SS >> ts;
				ii->push_back(atoi(ts.c_str()));
			}
		}
		
		_SS.clear();
	}

	fp.close();
}

//Set Functions
void XEN::set_encryption(const XEN::ENC_TYPE& _TYPE) {
	_ENCRYPT = _TYPE;
}

void XEN::set_version(const cn_short& _VAR) {
	_VERSION = _VAR;
}

void XEN::set_mode(const XEN::MODE_TYPE& _VAR) {
	_MODE = _VAR;
}

void XEN::set_tag(const string& _NAME, const string& _DATA) {
	if (_TAG.find(_NAME) == _TAG.end())
		_TAG.insert(pair<string, string>(_NAME, _DATA)); //Name doesn't exist.
	else
		_TAG[_NAME] = _DATA; //Overwrite
}

void XEN::clear() {
	//Clear out our Red-Black Trees
	_DIRECTORY.clear();
	_TAG      .clear();
	_LIST     .clear();

	//Recall the constructor again to ensure obliteration
	XEN();
}

//Get Functions
XEN::ENC_TYPE XEN::get_encryption() {
	return _ENCRYPT;
}

cn_short XEN::get_version() {
	return _VERSION;
}

XEN::MODE_TYPE XEN::get_mode() {
	return _MODE;
}

string& XEN::get_tag(const string& _NAME) {
	if (_TAG.find(_NAME) == _TAG.end()) {
		return string(""); //Safely return
	}
	else
		return _TAG[_NAME];
}

XEN::_FILE& XEN::get_file(const string& _DIR, const string& _FNAME) {
	//We need this to seg fault upon failure.
	//Honestly I have no idea why you would use this.

	map<string, map<string, _FILE> >::iterator _I_DIR;
	map<string,             _FILE  >::iterator _I_FILE;

	_I_DIR  = _DIRECTORY.find(_DIR);
	if (_I_DIR == _DIRECTORY.end())
		return *(XEN::_FILE*)0x0; //STRAIGHT IN TO SEG-FAULT LAND!!

	if (_I_FILE == _DIRECTORY[_DIR].end())
		return *(XEN::_FILE*)0x0; //Probably should have done 0xDEADBEEF...

	return _DIRECTORY[_DIR][_FNAME]; //Awesome Casting
}

map<string, XEN::_FILE>& XEN::get_directory(const string& _DIR) {
	//I have no idea why the hell you would need this either...
	//OH! Let's make this one segfault too on failure!

	map<string, map<string, _FILE> >::iterator _I_DIR;

	_I_DIR  = _DIRECTORY.find(_DIR);
	if (_I_DIR == _DIRECTORY.end())
		return *(map<string, XEN::_FILE>*)0x0; //STRAIGHT IN TO SEG-FAULT LAND!!

	return _DIRECTORY[_DIR];
}

//Check Functions
bool XEN::tag_exists(const string& _STR) {
	return (_TAG.find(_STR) != _TAG.end());
}

//Write Functions
void XEN::write_file(const char* fname) {
	FILE* fp = fopen(fname, "wb");

	//File Header
	char* _HEADER;
	switch (_MODE) {
		 case M_SXEN: _HEADER = "SXEN"; break;
		 case M_DXEN: _HEADER = "DXEN"; break;
	}

	fwrite(_HEADER, sizeof(char), 4, fp);

	//File information
	VLQ::write_int(fp, _VERSION);
	VLQ::write_int(fp, (_MODE == M_DXEN) ? _ENCRYPT : 0);

	//Now for the REAL fun
	map<string, string>             ::iterator _MAP_ITERATOR;
	map<string, vector<int> >       ::iterator _LIST_ITERATOR;
	map<string, map<string, _FILE> >::iterator _DIR_ITERATOR;
	map<string, _FILE>              ::iterator _FILE_ITERATOR;
	cn_uint s1, s2;

	//Manage Tags first
	VLQ::write_int(fp, _TAG.size());

	for (_MAP_ITERATOR = _TAG.begin(); _MAP_ITERATOR != _TAG.end(); _MAP_ITERATOR++) {
		s1 = _MAP_ITERATOR->first.size ();
		s2 = _MAP_ITERATOR->second.size();

		VLQ::write_int(fp, s1);
		fwrite(_MAP_ITERATOR->first .c_str(), sizeof(char), s1, fp);

		VLQ::write_int(fp, s2);
		fwrite(_MAP_ITERATOR->second.c_str(), sizeof(char), s2, fp);
	}

	//Write lists
	VLQ::write_int(fp, _LIST.size());

	for (_LIST_ITERATOR = _LIST.begin(); _LIST_ITERATOR != _LIST.end(); _LIST_ITERATOR++) {
		vector<int>& _REF = _LIST_ITERATOR->second;
		s1 = _LIST_ITERATOR->first.size();
		s2 = _REF.size();

		VLQ::write_int(fp, s1);
		fwrite(_LIST_ITERATOR->first.c_str(), sizeof(char), s1, fp);

		VLQ::write_int(fp, s2);
		for (int i = 0; i < s2; i++)
			VLQ::write_int(fp, _LIST_ITERATOR->second[i]);
	}

	//Write files
	VLQ::write_int(fp, _DIRECTORY.size());
	for (_DIR_ITERATOR = _DIRECTORY.begin(); _DIR_ITERATOR != _DIRECTORY.end(); _DIR_ITERATOR++) {
		s1 = _DIR_ITERATOR->first .size(); //Size of string "in constant time"
		s2 = _DIR_ITERATOR->second.size(); //Number of files

		VLQ::write_int(fp, s1);
		fwrite(_DIR_ITERATOR->first.c_str(), sizeof(char), s1, fp);

		VLQ::write_int(fp, s2);
		//Now things get nasty
		for (_FILE_ITERATOR = _DIR_ITERATOR->second.begin(); _FILE_ITERATOR != _DIR_ITERATOR->second.end(); _FILE_ITERATOR++) {
			s1 = _FILE_ITERATOR->first.size();
			VLQ::write_int(fp, s1);
			fwrite(_FILE_ITERATOR->first.c_str(), sizeof(char), s1, fp);

			FILE* token = fopen(_FILE_ITERATOR->second.path, "rb");

			//First off, get filesize.
			fseek(token, (size_t)0, SEEK_END);
			unsigned long long sz = ftell(token);
			rewind(token); //Back to the beginning.

			//Read all bytes into array.
			cn_byte* _BYTE = (cn_byte *) malloc(sz);
			fread(_BYTE, 1, sz, token);
			fclose(token);

			//Copy the bytes into the destination file.
			VLQ::write_int(fp, sz);
			fwrite(_BYTE, 1, sz, fp);

			//Have a nice day.
			free(_BYTE);
		}
	}

	fclose(fp);
}

//Read Functions
void XEN::read_file(const char* fname) {
	FILE *fp = fopen(fname, "rb");

	if (_PATH != NULL)
		free(_PATH);
	_PATH = strdup(fname);

	char* SXEN_HEADER = "SXEN",
	    * DXEN_HEADER = "DXEN",
		  SCAN_HEADER[5];
	SCAN_HEADER[4] = '\0';

	fread(SCAN_HEADER, 1, 4, fp);

	if (strcmp(SXEN_HEADER, SCAN_HEADER) == 0) {
		//printf("Type: SXEN File.\n");
	}
	else
	if (strcmp(DXEN_HEADER, SCAN_HEADER) == 0) {
		//printf("Type: DXEN File.\n");
	}
	else {
		fprintf(stderr, "[ Error ] The file %s is not an SXEN or DXEN file!\n", fname);
		return;
	}
	
	_VERSION =           VLQ::read_int(fp);
	_ENCRYPT = (ENC_TYPE)VLQ::read_int(fp);
	//printf("\nFile Version: %d\nEncryption Method: %d\n", _VERSION, _ENCRYPT);

	//Load Tags
	_TAG.clear();
	unsigned int s1, s2, s3, i, j,
	             dir_count, file_count;
	char* name,
		* file_n,
	    * desc;
	s3 = VLQ::read_int(fp);
	//printf("\nTags: %d\n", s3);
	for (i = 0; i < s3; i++) {
		//Read the name
		s1 = VLQ::read_int(fp);
		name = (char *) calloc(s1 + 1, 1);
		fread(name, 1, s1, fp);

		//Read the description
		s2 = VLQ::read_int(fp);
		desc = (char *) calloc(s2 + 1, 1);
		fread(desc, 1, s2, fp);

		_TAG.insert(pair<string, string>(
			string(name),
			string(desc)
		));
		//printf("  %s = %s\n", name, desc);

		free(name);
		free(desc);
	}

	//Lists
	s3 = VLQ::read_int(fp);
	//printf("\nLists:\n");
	for (i = 0; i < s3; i++) {
		//Read the name
		s1 = VLQ::read_int(fp);
		name = (char *) calloc(s1 + 1, 1);
		fread(name, 1, s1, fp);

		_LIST.insert(pair< string, vector<int> >(
			string(name),
			vector<int>()
		));

		//printf("  %s: ", name);

		vector<int>& _REF = _LIST[string(name)];
		_REF.resize(VLQ::read_int(fp));
		for (j = 0; j < _REF.size(); j++) {
			_REF[j] = VLQ::read_int(fp);
			//printf("%d%s", _REF[j], (j == _REF.size() - 1) ? "\n" : ", ");
		}

		free(name);
	}

	//Read Directories and Files
	//printf("\nFiles:\n");
	dir_count = VLQ::read_int(fp);
	for (i = 0; i < dir_count; i++) {
		s1 = VLQ::read_int(fp);
		name = (char *) calloc(s1 + 1, 1);
		fread(name, 1, s1, fp);
		s2 = VLQ::read_int(fp);

		_DIRECTORY.insert(pair< string, map<string, _FILE> >(
			string(name),
			map<string, _FILE>()
		));
		map<string, _FILE>& _CUR_DIR = _DIRECTORY[string(name)];
		//printf("  Directory: %s\n", name);

		//Now for the files.
		for (j = 0; j < s2; j++) {
			s3 = VLQ::read_int(fp); //Length of name
			file_n = (char *) calloc(s3 + 1, 1);
			fread(file_n, 1, s3, fp);

			_CUR_DIR.insert(pair<string, _FILE>(
				string(file_n),
				_FILE()
			));
			//printf("    File: %s\n", file_n);
			_FILE& _CUR_FILE = _CUR_DIR[string(file_n)];
			
			//Set up file properties
			_CUR_FILE.size = VLQ::read_int(fp);
			_CUR_FILE.pos  = ftell(fp);

			//Skip to the next file.
			fseek(fp, _CUR_FILE.size, SEEK_CUR);
			free(file_n);
		}

		free(name);
	}

	fclose(fp);
}

//Extraction Functions
void XEN::extract_all(const char* path) {
	if (_PATH == NULL)
		return; //You are a failure.

	FILE* fp = fopen(_PATH, "rb");

	//Create Extraction Directory
	CreateDirectory(path, NULL);

	//Set up iterators
	map<string, map<string, _FILE> >::iterator _DIR_ITERATOR;
	map<string, _FILE>              ::iterator _FILE_ITERATOR;
	string _cur_dir = ".";

	for (_DIR_ITERATOR = _DIRECTORY.begin(); _DIR_ITERATOR != _DIRECTORY.end(); _DIR_ITERATOR++) {
		//Create the directory
		_cur_dir = string(path) + "\\" + _DIR_ITERATOR->first;
		CreateDirectory(_cur_dir.c_str(), NULL);

		//File Traversal
		for (_FILE_ITERATOR = _DIR_ITERATOR->second.begin(); _FILE_ITERATOR != _DIR_ITERATOR->second.end(); _FILE_ITERATOR++) {
			//Load packaged file into memory and write directly to the new file.
			//TODO: Find a faster way to do this...
			FILE* token = fopen((_cur_dir + "\\" + _FILE_ITERATOR->first).c_str(), "wb");
			fseek(fp, _FILE_ITERATOR->second.pos, SEEK_SET);
			cn_byte* BYTE = (cn_byte *) malloc(_FILE_ITERATOR->second.size);
			fread (BYTE, 1, _FILE_ITERATOR->second.size, fp);
			fwrite(BYTE, 1, _FILE_ITERATOR->second.size, token);
			free(BYTE);
			fclose(token);
		}
	}

	fclose(fp);
}

//Secret functions only I can use because I am special
string XEN::get_rest_of_stringstream(istringstream& _SS) {
	string str;
	getline(_SS, str); //YUP CHEATING
	int str_cheat = 0;

	while (str_cheat < str.length() && str[str_cheat] == ' ')
		str_cheat++;

	if (str_cheat > 0)
		str.erase(0, str_cheat);

	return str;
}

XEN::_FILE::~_FILE() {
	//Because malloc can be a pain sometimes.
	if ( path != NULL) free( path);
	if (_BYTE != NULL) free(_BYTE);
}