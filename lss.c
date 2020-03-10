#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

struct File
{
	char name[512];				//pathname
	char permissions[512];		//permissions
	char size[512];				//size of file
	char UID[512];				//User ID
	char GID[512];				//Group ID 
	char numLinks[512];			//number of hard links
	char time[512];				//time of last modification
};

struct File fileArray[1024];	//An array of Files
int fileArrayIndex = 0;			//Index for this array of structs


void generatePermissions()
{
	//Function that generates the permissions[] member variable of each File
	//permission[] stores the write, read, and execute permissions of each File
	int loop;
	struct stat fileStat;
	
	for(loop = 0; loop < fileArrayIndex; loop++)		//Iterate over the array of Files
	{
		char perm[512];
		int pIndex = 0;
		if (stat(fileArray[loop].name, &fileStat) < 0)
		{
			perror("ERROR: In function 'generatePermissions(...)', unable to open file\n");
			continue;
		}

		if (S_ISDIR(fileStat.st_mode)) perm[pIndex++]  = 'd'; else perm[pIndex++] = '-'; //Check if it is a directory
		if (fileStat.st_mode & S_IRUSR) perm[pIndex++] = 'r'; else perm[pIndex++] = '-'; //USER rights
		if (fileStat.st_mode & S_IWUSR) perm[pIndex++] = 'w'; else perm[pIndex++] = '-';
		if (fileStat.st_mode & S_IXUSR) perm[pIndex++] = 'x'; else perm[pIndex++] = '-';
		if (fileStat.st_mode & S_IRGRP) perm[pIndex++] = 'r'; else perm[pIndex++] = '-'; //GROUP rights
		if (fileStat.st_mode & S_IWGRP) perm[pIndex++] = 'w'; else perm[pIndex++] = '-';
		if (fileStat.st_mode & S_IXGRP) perm[pIndex++] = 'x'; else perm[pIndex++] = '-';
		if (fileStat.st_mode & S_IROTH) perm[pIndex++] = 'r'; else perm[pIndex++] = '-'; //OTHER rights
		if (fileStat.st_mode & S_IWOTH) perm[pIndex++] = 'w'; else perm[pIndex++] = '-';
		if (fileStat.st_mode & S_IXOTH) perm[pIndex++] = 'x'; else perm[pIndex++] = '-';
	
		perm[pIndex] = '\0';															 //Add a null terminator to permissions[]
		strcpy(fileArray[loop].permissions, perm);
	}
	return;
}

void setUID()
{
	//This function will set the UID[] member variable of each File
	//UID[] stores the name of the user of each file
	int loop;
	struct stat fileStat;
	struct passwd * pwd;
   	char tempID[512];	

	for (loop = 0; loop < fileArrayIndex; loop++)			//Loop through all of the Files
	{
		if (stat(fileArray[loop].name, &fileStat) < 0)
		{
			perror("Error: In function 'setUID()', unable to open file\n");
			continue;
		}

		if ( (pwd = getpwuid(fileStat.st_uid)) != NULL)	    //Get the name of the user using the user's ID	
		{
			//printf("pwd->pw_name: %s\n", pwd->pw_name);
			strcpy(tempID, pwd->pw_name);
			strcpy(fileArray[loop].UID, tempID);
		}
		else
		{
			perror("ERROR: In function 'setUID', getpwuid() failed\n");
			continue;
		}
	}
}

void numLinks()
{
	//Generates the numLinks[] variable of each File
	//numLinks[] contains the number of hard links each File has
	int loop;
	struct stat fileStat;
	char buf[512];

	for (loop = 0; loop < fileArrayIndex; loop++)			//Iterate over the array of files
	{
		if (stat(fileArray[loop].name, &fileStat) < 0)
		{
			perror("ERROR: In function 'numLinks()', unable to open file\n");
			continue;
		}
		sprintf(buf, "%llu", (unsigned long long)fileStat.st_nlink);
		strcpy(fileArray[loop].numLinks, buf);
	}
}

void getGroupID()
{
	//Generates the GUID{} variable of each File
	//GUID[] contains the name of the group of which the file belongs to 
	int loop;
	struct stat fileStat;
	struct group * grp;
	gid_t gid;
	char tempGID[512];

	for (loop = 0; loop < fileArrayIndex; loop++)				//Iterate over the array of files
	{
		if (stat(fileArray[loop].name, &fileStat) < 0)
		{
			perror ("ERROR: In function 'getGroupName()', unable to open file\n");
			continue;
		}
	
		if ( (grp = getgrgid(fileStat.st_gid)) != NULL)			//Get the group name by using the group ID
		{
			strcpy(tempGID, grp->gr_name);
			strcpy(fileArray[loop].GID, tempGID);
		}
		else
		{
			perror("ERROR: In function 'getGroupID', getgrgid() failed\n");
			continue;
		}

		///This function is giving me problems now
	}
}

void getTime()
{
	//This function generates the time[] variable of each File
	int loop;
	struct stat fileStat;
	char buffer[512];

	for (loop = 0; loop < fileArrayIndex; loop++)			//Iterate over the array of files
	{
		if (stat(fileArray[loop].name, &fileStat) < 0)
		{
			perror("ERROR: In function 'getTime()', unable to open file\n");
			continue;
		}
		if (strftime(buffer, 20, "%b %d %H:%M", localtime(&fileStat.st_mtime)) == 0) //Get the desired format of time
		{
			perror("ERROR: In function 'getTime()', strftime() did not work properly\n");
			continue;
		}
		strcpy(fileArray[loop].time, buffer);
	}
}

int largestSize()
{
	//This function will look at the sizes of the files and return the length of the largest file so that it can be
	//used to format the output nicely.
	
	int max = strlen(fileArray[0].size);			//Consider the first file's size as the max
	int loop;

	for (loop = 1; loop < fileArrayIndex; loop++)
	{
		if (max < strlen(fileArray[loop].size))
		{
			max = strlen(fileArray[loop].size);
		}
		else
		{
			//max > fileArray[loop]
			//max = fileArray[loop]
			continue;
		}
	}
	return max;

}

int compare(const void * a, const void * b)
{
	//Compare function used by qsort()
	const struct File * num1 = a;
	const struct File * num2 = b;
	
	//Ensure that the tyoes are ints
	int value1, value2;
	value1 = atoi(num1->size);
	value2 = atoi(num2->size);

	//Reverse the comparisons so that we get descending order
	if (value1 < value2) return 1;
	if (value1 > value2) return -1;
	if (value1 == value2) return 0;	
}

int main()
{
	struct dirent * de;		//Pointer for directory entry
	DIR * dr = opendir(".");	//returns a pointer of DIR type
	if (dr == NULL)		//opendir() opens a directory stream 
	{	
		perror("Could not open current directory\n");
		return 0;
	}

	char buffer[512];
	memset(buffer, 0, 512);
	struct File f;
	struct stat fileStat;
	char SIZE[512];

	while ( (de = readdir(dr)) != NULL )
	{
		if (lstat(de->d_name, &fileStat) < 0)
		{
			perror("ERROR: 'lstat(...)' failed\n");
			continue;
		}

		if (S_ISLNK(fileStat.st_mode) == 1)
		{
			char * linkName;				//This will store the pathname that the symbolic link is referencing
			ssize_t readBytes;				//Will be used for error checking, will store the bytes returned by readlink()
			linkName = malloc(fileStat.st_size + 1);
			if (!linkName)
			{
				perror("ERROR: malloc(...) failed\n");
				printf("'%s' is a broken link\n", de->d_name);
				free(linkName);
				continue;
			}
			readBytes = readlink(de->d_name, linkName, fileStat.st_size + 1);
			linkName[fileStat.st_size] = '\0';
			if (readBytes == -1)
			{
				perror("ERROR: readlink(...) failed\n");
				printf("'%s' is a broken link\n", de->d_name);
				free(linkName);
				continue;
			}
			else
			{
				struct stat target;			//This will be what the symbolic link points to
				if (stat(linkName, &target) < 0)
				{
					//Symbolic points to nowhere (i.e. a file that does not exist)
					perror("ERROR");
					printf("...'%s' is a broken link\n", de->d_name);
				}
				else
				{
					//Symbolic points to a valid file
					strcpy(f.name, de->d_name);
					sprintf(SIZE, "%llu", (unsigned long long)fileStat.st_size);
					strcpy(f.size, SIZE);
					fileArray[fileArrayIndex++] = f;
				}

				free(linkName);
			}
		}
		else
		{
			strcpy(f.name, de->d_name);
			sprintf(SIZE, "%llu", (unsigned long long)fileStat.st_size);
			strcpy(f.size, SIZE);
			fileArray[fileArrayIndex++] = f;
		}
	}
	closedir(dr);

	/*
	while ( (de = readdir(dr)) != NULL )
	{
		if (stat(de->d_name, &fileStat) < 0)
		{
			perror("ERROR: Could not open file");
			printf("'%s'\n", de->d_name);
			continue;
		}
		if (S_ISLNK(fileStat.st_mode))
			printf("THIS IS a YSMBOLIC LINK");

		strcpy(f.name, de->d_name);
		sprintf(SIZE, "%llu", (unsigned long long)fileStat.st_size);
		strcpy(f.size, SIZE); 
		fileArray[fileArrayIndex++] = f;
	}
	closedir(dr);
	*/

	generatePermissions();
	setUID();
	numLinks();
	getGroupID();
	getTime();

	int maxChars = largestSize();
	int x = 0;

	qsort(fileArray, fileArrayIndex, sizeof(struct File), compare);

	//"After sorting"
	//Print out each file in long-listing format
	for (x = 0; x < fileArrayIndex; x++)
	{
		printf("%s %s %s %s %*s %s %s\n", fileArray[x].permissions, fileArray[x].numLinks, fileArray[x].UID, fileArray[x].GID, maxChars, fileArray[x].size, fileArray[x].time, fileArray[x].name);
	}
	return 0;
}

