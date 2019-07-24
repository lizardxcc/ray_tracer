#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <dirent.h>
#include <vector>
#include <string>

class FileBrowser {
	public:
		FileBrowser(void);
		FileBrowser(const char *dir);
		~FileBrowser(void);
		bool OpenDir(const char *dir);
		std::string Render(void);
	private:
		std::string current_dir;
		DIR *dirp = nullptr;
		std::vector<std::string> entries;
};

#endif
