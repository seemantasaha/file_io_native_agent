#include <iostream>
#include <fstream>

using namespace std;

extern ofstream myfile;

struct FileInfo {
	const char *name;
	const char *abs_loc;
};

struct ContentInfo {
	char *content;
	int offset;
	int len;
};

enum EventType {
	close = 0,
	open = 1,
	read = 2,
	write = 3
};

class FileIOEvent {
	private:
		char *timestamp;
		EventType type;
		FileInfo *file_info;
		ContentInfo *content_info;

	public:

		FileIOEvent (time_t now, EventType tp) ;

		~FileIOEvent () ;

		void set_file_info (const char *name, const char *abs_loc);

		void set_content_info (char *content, int off, int len);

		void save_to_csv ();
};