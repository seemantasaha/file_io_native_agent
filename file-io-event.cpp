#include <iostream>
#include <fstream>
#include <ctime>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "file-io-event.h"

using namespace std;

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


FileIOEvent :: FileIOEvent(time_t now, EventType tp) {
	timestamp = ctime(&now);
	type = tp;
}

FileIOEvent :: ~FileIOEvent() {
}

void FileIOEvent :: set_file_info (const char *name, const char *abs_loc) {
	file_info = new FileInfo;
	file_info->name = name;
	file_info->abs_loc = abs_loc;
}

void FileIOEvent :: set_content_info (char *content, int off, int len) {
	content_info = new ContentInfo;
	content_info->content = content;
	content_info->offset = off;
	content_info->len = len;
}

void FileIOEvent :: save_to_csv () {
	
	stringstream ss;
	ss << to_string(type) << ",";
	ss << file_info->name << ",";
	ss << file_info->abs_loc << ",";
	ss << content_info->content << ",";
	ss << to_string(content_info->offset) << ",";
	ss << to_string(content_info->len) << ",";
	
	string row = ss.str();
	replace(row, "\n", "\\N");
	myfile << row;
	myfile << timestamp;
}