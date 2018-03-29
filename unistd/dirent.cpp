// dirent.cpp
// Created by Robin Rowe on 2016/8/30
// Copyright (c) 2016 Robin.Rowe@CinePaint.org
// License open source MIT

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <list>
#include <string>
#include "dirent.h"

#define SUFFIX	'*'
#define	SLASH	'\\'

DIR* opendir(const char *path)
{	errno = 0;
	if(!path)
	{	errno = EFAULT;
		return 0;
	}
	if(!path[0])
	{	errno = ENOTDIR;
		return 0;
	}
	DIR* dir = new DIR;
	if (!dir)
	{	errno = ENOMEM;
		return 0;
	}
	if(!dir->FindFirst(path))
	{	errno = ENOENT;
		//		errno = ENOTDIR;
		delete dir;
		return 0;
	}
	return dir;
}

struct dirent* readdir(DIR* dir)
{	errno = 0;
	if(!dir || !*dir)
	{	errno = EFAULT;
		return nullptr;
	}
	return dir->Find();
}

int readdir_r(DIR *dir, struct dirent *entry, struct dirent** result)
{	entry = readdir(dir);
	if(!entry)
	{	return -1;
	}
	*result = entry;
	return 0;
}

int closedir(DIR* dir)
{	errno = 0;
	if (!dir)
	{	errno = EFAULT;
		return -1;
	}
	int retval = dir->Close();
	delete dir;
	return retval;
}

#if 0
void rewinddir(DIR * dir)
{	errno = 0;
	if (!dir)
	{	errno = EFAULT;
		return;
	}
	if(dir->dd_handle != -1)
	{	_findclose (dir->dd_handle);
	}
	dir->dd_handle = -1;
	dir->dd_stat = 0;
}

long telldir(DIR * dir)
{	errno = 0;
	if (!dir)
	{	errno = EFAULT;
		return -1;
	}
	return dir->dd_stat;
}

void seekdir(DIR * dir, long lPos)
{	errno = 0;
	if (!dir)
	{	errno = EFAULT;
		return;
	}
	if(lPos < -1)
	{	errno = EINVAL;
		return;
	}
	if(lPos == -1)
	{	if(dir->dd_handle != -1)
		{	_findclose (dir->dd_handle);
		}
		dir->dd_handle = -1;
		dir->dd_stat = -1;
	}
	else
	{	rewinddir (dir);
		while ((dir->dd_stat < lPos) && readdir (dir))
		;	
	}
}
#endif

int alphaqsort(const void* v1, const void* v2)
{	dirent** d1 = (dirent**)v1;
	dirent** d2 = (dirent**)v2;
	const char* name1 = (*d1)->d_name;
	const char* name2 = (*d2)->d_name;
	return strcmp(name1,name2);
}

unsigned GetFileCount(DIR* dir,scandir_f selector)
{	unsigned count = 0;
	for(;;)
	{	dirent* entry = readdir(dir);
		if(!entry)
		{	break;
		}
		if(selector != NULL && !(*selector)(entry))
		{	continue;
		}
		count++;
	}
	return count;
}

int scandir(const char* dirname, dirent*** namesList, scandir_f selector, scandir_alphasort sorter)
{	DIR* dir = opendir(dirname);
	if(!dir)
	{	return -1;
	}
	const unsigned count = GetFileCount(dir,selector);
	if(!count)
	{	return -1;
	}
	closedir(dir);
	dirent** names = (dirent**) malloc(count * sizeof(dirent*));
	if(!names)
	{	return -1;
	}
	dir = opendir(dirname);
	if (!dir)
	{	return -1;
	}
	*namesList = names;
	int matches = 0;
	for(unsigned i = 0; i < count; i++)
	{	dirent* temp = readdir(dir);
		if(!temp)
		{	break;
		}
		if(selector != NULL && !(*selector)(temp))
		{	continue;
		}
		const size_t size = sizeof(dirent) + strlen(temp->d_name);
		dirent* entry = (dirent*) malloc(size);
		if(!entry)
		{	return -1;
		}
		*entry = *temp;
		if(temp->d_name)
		{	strcpy(entry->buffer,temp->d_name);
		}
		else
		{	entry->buffer[0] = 0;
		}
		entry->d_name = entry->buffer;
		names[matches] = entry;
		matches++;
	}
	closedir(dir);
	if(sorter) 
	{	qsort(names, matches, sizeof(dirent*),alphaqsort);
	}
	return matches;
}
