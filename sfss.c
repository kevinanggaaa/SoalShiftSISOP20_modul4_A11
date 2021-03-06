/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Minor modifications and note by Andy Sayler (2012) <www.andysayler.com>
  Source: fuse-2.8.7.tar.gz examples directory
  http://sourceforge.net/projects/fuse/files/fuse-2.X/
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall `pkg-config fuse --cflags` fusexmp.c -o fusexmp `pkg-config fuse --libs`
  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.
	https://github.com/asayler/CU-CS3753-PA5/blob/master/fusexmp.c
*/

// /home/zenados/Documents/FuseDebugging
// Command to start fuse
/* 
./compile.sh soal1.c 1.exe
./1.exe -d testlink/ -o modules=subdir,subdir=/home/zenados/Documents/4shift/FuseDebugging
./compile.sh soal1.c 1.exe && ./1.exe -d testlink/ -o modules=subdir,subdir=/home/zenados/Documents/4shift/FuseDebugging
cd /home/zenados/Documents/4shift/testlink/testDir
rm -r dirindir2/
rm -r dirindir3/
rm -r tmp/
rm -r sync_tmp/
cp -r dirindir/ dirindir2/
cp -r dirindir/ dirindir3/
mv dirindir2/ tmp/ && mv dirindir3/ sync_tmp/
 */
#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <sys/types.h>

// Switched dot '.' character with space ' ' character since . is an extension specific character
char charlist[] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";

void encrypt1(const char *filepathIn, int offset,	char *outBuff) {
  int startIdx = 0;
	char *tmpChrP;

	strcpy(outBuff,filepathIn);
	if ((tmpChrP = strstr(outBuff,"/encv1_"))!=NULL && (tmpChrP = strrchr(tmpChrP+1,'/'))!=NULL) {
	// if ((tmpChrP = strrchr(filepathIn,'/'))!=NULL) {
		startIdx = tmpChrP-outBuff;
	} else {
	// Else, this is the /encv1_ itself, there's nothing to encrypt or decrypt
		return;
	}

	int fileExtIdx;
	if ((tmpChrP = strrchr(outBuff, '.'))!=NULL) {
		fileExtIdx = tmpChrP-outBuff;
	} else {
		fileExtIdx = strlen(outBuff);
	}

	int tempIdx;
  while(startIdx != fileExtIdx) {
	// Iterate until file extension
		if (outBuff[startIdx] != '/') {
			tempIdx = strchr(charlist,outBuff[startIdx]) - charlist + offset;
			outBuff[startIdx] = charlist[tempIdx];
		}
		startIdx++;
  }
	printf("ENCoutput:%s\n",outBuff);
}

void encryptRaw1(char *inStr, int offset) {
  int startIdx = 0;

	int fileExtIdx;
	char *tmpChrP;
	if ((tmpChrP = strrchr(inStr, '.'))!=NULL) {
		fileExtIdx = tmpChrP-inStr;
	} else {
		fileExtIdx = strlen(inStr);
	}

	int tempIdx;
  while(startIdx != fileExtIdx) {
	// Iterate until file extension
		if (inStr[startIdx] != '/') {
			tempIdx = strchr(charlist,inStr[startIdx]) - charlist + offset;
			inStr[startIdx] = charlist[tempIdx];
		}
		startIdx++;
  }
}

void encrypt1Rename(const char *filepathIn, int offset) {
	char outBuff[500];
	encrypt1(filepathIn,offset,outBuff);
	printf("\n\nfrom: %s |to| %s\n\n\n",filepathIn,outBuff);
	rename(filepathIn,outBuff);
}

void decrypt1(const char *filepathIn, int offset,	char *outBuff) {
  int startIdx = 0;
	char *tmpChrP;

	strcpy(outBuff,filepathIn);
	if ((tmpChrP = strstr(outBuff,"/encv1_"))!=NULL && (tmpChrP = strrchr(tmpChrP+1,'/'))!=NULL) {
	// if ((tmpChrP = strrchr(filepathIn,'/'))!=NULL) {
		startIdx = tmpChrP-outBuff;
	} else {
	// Else, this is the /encv1_ itself, there's nothing to encrypt or decrypt
		return;
	}

	int fileExtIdx;
	if ((tmpChrP = strrchr(outBuff, '.'))!=NULL) {
		fileExtIdx = tmpChrP-outBuff;
	} else {
		fileExtIdx = strlen(outBuff);
	}

	int tempIdx;
  while(startIdx != fileExtIdx) {
	// Iterate until file extension
		if (outBuff[startIdx] != '/') {
			tempIdx = strrchr(charlist,outBuff[startIdx]) - charlist - offset;
			outBuff[startIdx] = charlist[tempIdx];
		}
		startIdx++;
  }
	printf("DECoutput:%s\n",outBuff);
}

void decryptRaw1(char *inStr, int offset) {
  int startIdx = 0;

	int fileExtIdx;
	char *tmpChrP;
	if ((tmpChrP = strrchr(inStr, '.'))!=NULL) {
		fileExtIdx = tmpChrP-inStr;
	} else {
		fileExtIdx = strlen(inStr);
	}

	int tempIdx;
  while(startIdx != fileExtIdx) {
	// Iterate until file extension
		if (inStr[startIdx] != '/') {
			tempIdx = strrchr(charlist,inStr[startIdx]) - charlist - offset;
			inStr[startIdx] = charlist[tempIdx];
		}
		startIdx++;
  }
}

void decrypt1Rename(const char *filepathIn, int offset) {
	char outBuff[500];
	decrypt1(filepathIn,offset,outBuff);
	rename(filepathIn,outBuff);
}

void encryptDir1(const char *dirPath) {
	printf("encryptDir1 called: %s\n",dirPath);
	char currDir[500];
	strcpy(currDir,dirPath);

	printf("currDir: %s",currDir);
	DIR *dp;
	struct dirent *de;
	dp = opendir(currDir);
	if (dp == NULL)
		return;

	printf("Starting read\n");
	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		}
		printf("\nencryptdir called\n");
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		char fullpath[500];
		sprintf(fullpath,"%s/%s",currDir,de->d_name);
		printf("fullpath:%s\n",fullpath);
		if (S_ISREG(st.st_mode)) {
		// If is regular file
			encrypt1Rename(fullpath,10);
		} else if (S_ISDIR(st.st_mode)) {
			encryptDir1(fullpath);
			encrypt1Rename(fullpath,10);
		} else {
			continue;
		}
	}

	closedir(dp);
	return;
}

void decryptDir1(const char *dirPath) {
	char currDir[500];
	strcpy(currDir,dirPath);

	DIR *dp;
	struct dirent *de;

	dp = opendir(currDir);
	if (dp == NULL)
		return;

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		}
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		char fullpath[500];
		sprintf(fullpath,"%s/%s",currDir,de->d_name);
		if (S_ISREG(st.st_mode)) {
		// If is regular file
			decrypt1Rename(fullpath,10);
		} else if (S_ISDIR(st.st_mode)) {
			decryptDir1(fullpath);
			decrypt1Rename(fullpath,10);
		} else {
			continue;
		}
	}

	closedir(dp);
	return;
}


// #define bufSize 32
#define bufSize 1024
void encrypt2(const char *filepath) {
	char buff[1025];
	int bytes_read;
	FILE *output;
	FILE *input;
	int fileCount = 0;
	
	// if filepath already have numbers at the end then the file is encrypted, don't re encrypt it.
	int startIdx;
	char *tmpChrP;
	if ((tmpChrP = strrchr(filepath,'.'))!=NULL) {
	// Get last dot found on file
		startIdx = tmpChrP-filepath;
		int isNum = 1;
		while(filepath[++startIdx]!='\0') {
		// If rest of string are not numbers then file is not encrypted, encrypt it.
			if(!(filepath[startIdx]>='0' && filepath[startIdx]<='9')) {
				isNum = 0;
				break;
			}
		}
		// If rest of string are numbers then file is encrypted, don't re encrypt it.
		if (isNum) {
			return;
		}
	} 

	input = fopen(filepath, "r");
	if (input == NULL) 
		printf("Error opening file '%s' !",filepath);
	while ((bytes_read = fread(buff, sizeof(char), bufSize, input)) > 0) {
		buff[bytes_read] = '\0';

		// Write to new file
		char newFilePath[500];
		sprintf(newFilePath,"%s.%03d",filepath,fileCount++);

		output = fopen(newFilePath, "w+");
		if (output == NULL) 
			printf("Error opening file '%s' !",newFilePath);
		fwrite(buff, bytes_read, 1, output);
		fclose(output);
		memset(buff, 0, bufSize);
	}
	fclose(input);
	// Remove file
	if (remove(filepath) == 0) 
		printf("Deleted successfully"); 
	else
		printf("Unable to delete the file"); 
}

// filepath is one of the partial files to concatenate
void decrypt2(const char *filenameIn) {
	char buff[1025];
	int bytes_read;
	FILE *output;
	FILE *input;
	int fileCount = 0;
	// if filepath doesn't have all numbers at the end then the file is not encrypted.
	int startIdx;
	char *tmpChrP;
	if ((tmpChrP = strrchr(filenameIn,'.'))!=NULL) {
	// Get last dot found on file
		startIdx = tmpChrP-filenameIn;
		int isNum = 1;
		while(filenameIn[++startIdx]!='\0') {
		// If rest of string are numbers then file is encrypted, decrypt it.
			if(!(filenameIn[startIdx]>='0' && filenameIn[startIdx]<='9')) {
				isNum = 0;
				break;
			}
		}
		// If rest of string are numbers then file is encrypted, don't re encrypt it.
		if (!isNum) {
			return;
		}
	} 

	int fileExtIdx = strrchr(filenameIn,'.')-filenameIn;
	char filename[500];
	sprintf(filename,"%s",filenameIn);
	filename[fileExtIdx] = '\0';
	output = fopen(filename, "a");
	if (output == NULL) 
		printf("Error opening file '%s' !",filename);

	while (1) {
		// get curfile
		char curFilename[500];
		sprintf(curFilename,"%s.%03d",filename,fileCount++);
		
		// check if file doesn't exist
		if (access(curFilename,F_OK)==-1) break;
		input = fopen(curFilename, "r");
		if (input == NULL)
			printf("Error opening file '%s' !",curFilename);
		
		bytes_read = fread(buff, sizeof(char), bufSize, input);
		fclose(input);
		printf("\n\nread %d:%s\n\n\n", bytes_read, buff);
		fwrite(buff, bytes_read, 1, output);
		memset(buff, 0, bufSize);
		// remove file
		remove(curFilename);
	}
	fclose(output);
}

void encryptDir2(const char *dirPath) {
	char currDir[500];
	strcpy(currDir,dirPath);

	DIR *dp;
	struct dirent *de;

	dp = opendir(currDir);
	if (dp == NULL)
		return;

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		}
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		char fullpath[500];
		sprintf(fullpath,"%s/%s",currDir,de->d_name);
		if (S_ISREG(st.st_mode)) {
		// If is regular file
			encrypt2(fullpath);
		} else if (S_ISDIR(st.st_mode)) {
			encryptDir2(fullpath);
		} else {
			continue;
		}
	}

	closedir(dp);
	return;
}

void decryptDir2(const char *dirPath) {
	char currDir[500];
	strcpy(currDir,dirPath);

	DIR *dp;
	struct dirent *de;

	dp = opendir(currDir);
	if (dp == NULL)
		return;

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		}
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		char fullpath[500];
		sprintf(fullpath,"%s/%s",currDir,de->d_name);
		if (S_ISREG(st.st_mode)) {
		// If is regular file
			decrypt2(fullpath);
		} else if (S_ISDIR(st.st_mode)) {
			decryptDir2(fullpath);
		} else {
			continue;
		}
	}

	closedir(dp);
	return;
}

// Use caps for level input
// Use empty string if arg2 does not exist
char logFilepath[] = "/home/zenados/fs.log";
void logging(char levelIn[], const char *arg1, const char *arg2) {
	char *level="INFO";
	if ( strcmp(levelIn,"RMDIR")==0 || strcmp(levelIn,"UNLINK")==0 ) {
		level = "WARNING";
	}

	time_t rawtime;
	time (&rawtime);
	char timestring[25];
	struct tm *timeis = localtime(&rawtime);
	sprintf(timestring,"%02d%02d%02d-%02d:%02d:%02d",timeis->tm_year-100,timeis->tm_mon,timeis->tm_mday,timeis->tm_hour,timeis->tm_min,timeis->tm_sec);

	char finalString[500];
	sprintf(finalString,"%s::%s::%s::%s",level,timestring,levelIn,arg1);
	if (strcmp(arg2,"")!=0){
		sprintf(finalString,"%s::%s",finalString,arg2);
	}

	FILE *logfs;

	logfs = fopen(logFilepath, "a");
	if (logfs == NULL)
		printf("Error opening log file!");
	fprintf(logfs,"%s\n",finalString);
	fclose(logfs);
}

char encLogFilepath[] = "/home/zenados/database.log";
void loggingCustom(char action[], char type[], const char *arg1, const char *arg2) {
	FILE *logfs;

	time_t rawtime;
	time (&rawtime);
	char timestring[25];
	struct tm *timeis = localtime(&rawtime);
	sprintf(timestring,"%02d%02d%02d-%02d:%02d:%02d",timeis->tm_year-100,timeis->tm_mon,timeis->tm_mday,timeis->tm_hour,timeis->tm_min,timeis->tm_sec);

	logfs = fopen(encLogFilepath, "a");
	if (logfs == NULL)
		printf("Error opening log file!");
	
	if (strcmp(action,"CREATED")==0) {
		fprintf(logfs,"%s|%s %s:\n\tfolder: %s\n",timestring,action,type,arg1);
	} else {
		if (strcmp(type,"") == 0) {
			fprintf(logfs,"%s|%s:\n\tfrom: %s\n\tto: %s\n",timestring,action,arg1,arg2);
		} else {
			fprintf(logfs,"%s|%s %s:\n\tfrom: %s\n\tto: %s\n",timestring,action,type,arg1,arg2);
		}
	}
	fclose(logfs);
}

char *syncFolderGetSyncedPath(const char *folderPathIn) {
	// printf("\n\nsyncFolderGetSyncedPath Called\n");
	char folderPath[500];
	strcpy(folderPath,folderPathIn);
	char parentPath[500];
	strcpy(parentPath,folderPathIn);

	size_t size=500; char *buff = malloc(size*sizeof(char));
	int found = 0;
	while (strcmp(parentPath,"")!=0) {
		// printf("currDir:%s\n",parentPath);
		if (getxattr(parentPath,"user.xsync_",buff,size)!=-1) {
			found = 1;
			break;
		}
		*(strrchr(parentPath,'/')) = '\0'; //
	}

	if (found) {
		char *subPath = folderPath+strlen(parentPath);
		char *endP;
		endP = buff+strlen(buff);
		strcpy(endP,subPath);
		return buff;
	} else {
		return NULL;
	}
}

void syncFolderSet(const char *from, const char *to) {
	char dirPath[500]; //path of directory without the sync
	char syncPath[500]; //path of directory with the sync
	// Get syncPath
	strcpy(syncPath,to);
	// Get dirPath
	strcpy(dirPath,to);
	char *tmpChrP1;
	char *tmpChrP2;
	tmpChrP1 = strstr(dirPath,"/sync_")+1;
	tmpChrP2 = (strchr(tmpChrP1,'_')+1);
	strcpy(tmpChrP1,tmpChrP2); //replace string from foo/sync_bar to foo/bar
	loggingCustom("SYNCED","",to,dirPath);

	size_t size=500; char buff[size];
	strcpy(buff,dirPath);
	printf("\nSetting x Attribute: %s\n",from);
	if (setxattr(from,"user.xsync_",buff,(strlen(buff)+1)*sizeof(char),XATTR_CREATE)==-1) {
		perror("\n\nThere was an error setting the sync attribute\n\n");
	}
	printf("\nSetting x Attribute: %s\n",from);
	strcpy(buff,syncPath);
	if (setxattr(dirPath,"user.xsync_",buff,(strlen(buff)+1)*sizeof(char),XATTR_CREATE)==-1) {
		perror("\n\nThere was an error setting the sync attribute\n\n");
	}
}

void syncFolderUnset(const char *from) {
	char dirPath[500]; //path of directory without the sync
	// Get dirPath
	strcpy(dirPath,from);
	char *tmpChrP1;
	char *tmpChrP2;
	tmpChrP1 = strstr(dirPath,"/sync_")+1;
	tmpChrP2 = (strchr(tmpChrP1,'_')+1);
	strcpy(tmpChrP1,tmpChrP2); //replace string from foo/sync_bar to foo/bar

	loggingCustom("UNSYNCED","",from,dirPath);
	removexattr(from,"user.xsync_");
	removexattr(dirPath,"user.xsync_");
}

int syncFolderDirReq(const char *path1, const char *path2) {
	DIR *dp;
	struct dirent *de;

	dp = opendir(path1);
	if (dp == NULL)
		return 0;
	printf("checking dir:%s\n",path1);
	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		}

		char fullpath1[500],fullpath2[500];
		sprintf(fullpath1,"%s/%s",path1,de->d_name);
		sprintf(fullpath2,"%s/%s",path2,de->d_name);
		printf("\tcurPath checked:\n\t\t%s\n\t\t%s\n",fullpath1,fullpath2);
		struct stat stmp1,stmp2;
		stat(fullpath1, &stmp1);
		stat(fullpath2, &stmp2);
		printf("\tTime1:\t%ld | %ld\n",stmp1.st_mtime,stmp1.st_mtimensec);
		unsigned long long int ms1 = stmp1.st_mtime*1e3 + stmp1.st_mtimensec/1e6;
		printf("\tTime2:\t%ld | %ld\n",stmp2.st_mtime,stmp2.st_mtimensec);
		unsigned long long int ms2 = stmp2.st_mtime*1e3 + stmp2.st_mtimensec/1e6;
		// if requirements not met
		printf("\tTime:\t%lld | %lld\n",ms1,ms2);
		printf("\nAccess:%d\n",access(fullpath2,F_OK));
		printf("TimeDiff:%d\n",abs((int)(ms1-ms2)));
		if (access(fullpath2,F_OK)==-1 || abs((int)(ms1-ms2)) > 100) {
			return 0;
		}

		if (S_ISDIR(stmp1.st_mode)) {
			if (!syncFolderDirReq(fullpath1,fullpath2)) return 0;
		} else {
			continue;
		}

	}

	closedir(dp);
	printf("Closing dir returning true");
	return 1;
}

int syncFolderReq(const char *from, const char *to) {
	char parentDirectory[500];
	char dirPath[500]; //path of directory without the sync
	char syncPath[500]; //path of directory with the sync
	// Get parent dir path
	strcpy(parentDirectory,to);
	int tmpI = strrchr(parentDirectory,'/')-parentDirectory;
	parentDirectory[tmpI] = '\0';
	// Get syncPath
	strcpy(syncPath,to);
	// Get dirPath
	strcpy(dirPath,to);
	char *tmpChrP1;
	char *tmpChrP2;
	tmpChrP1 = strstr(dirPath,"/sync_")+1;
	tmpChrP2 = (strchr(tmpChrP1,'_')+1);
	strcpy(tmpChrP1,tmpChrP2); //replace string from foo/sync_bar to foo/bar
	// If parent dir is not similar
	printf("From:%s\nAccessing:%s\n",from,dirPath);
	if (access(dirPath,F_OK)==-1) return 0;

	// If path is already synced
	size_t size=500; char buff[size];
	printf("getting x attr from:%s\n",from);
	if (getxattr(from,"user.xsync_",buff,size)!=-1) return 0;
	printf("getting x attr from:%s\n",dirPath);
	if (getxattr(dirPath,"user.xsync_",buff,size)!=-1) return 0;
	
	// If contents is not similar or last modified time is not > 0.1 s
	printf("Checking syncFolderDirReq %s | %s\n",dirPath,from);
	if (!syncFolderDirReq(dirPath,from)) return 0;
	printf("Done checking\n");
	printf("Checking syncFolderDirReq %s | %s\n",dirPath,from);
	if (!syncFolderDirReq(from,dirPath)) return 0;

	return 1;
}

static int xmp_rename(const char *from, const char *to) {
	logging("RENAME",from,to);
	// Find last occurence of 
	char *tmpChrPFrom = strrchr(from,'/');
	char *tmpChrPTo = strrchr(to,'/');
	if (strstr(tmpChrPFrom,"/encv1_")!=NULL && strstr(tmpChrPTo,"/encv1_")==NULL) {
	// If directory is now decrypted
		loggingCustom("DECRYPTED","Type 1",from,to);
		decryptDir1(from);
	}
	if (strstr(tmpChrPFrom,"/encv2_")==NULL && strstr(tmpChrPTo,"/encv2_")!=NULL) {
	// If directory is now encrypted
		loggingCustom("ENCRYPTED","Type 2",from,to);
		encryptDir2(from);
	}
	if (strstr(tmpChrPFrom,"/encv2_")!=NULL && strstr(tmpChrPTo,"/encv2_")==NULL) {
	// If directory is now decrypted
		loggingCustom("DECRYPTED","Type 2",from,to);
		decryptDir2(from);
	}
	if (strstr(tmpChrPFrom,"/sync_")==NULL && strstr(tmpChrPTo,"/sync_")!=NULL) {
	// If directory is now a sync directory
		printf("\n\nRenaming directory to a sync directory\n");
		if (syncFolderReq(from,to)) {
			printf("Directory eligible to ba a sync directory\n");
			syncFolderSet(from,to);
		} else {
			// Cancel rename
			return -1;
		}
	} else if (strstr(tmpChrPFrom,"/sync_")!=NULL && strstr(tmpChrPTo,"/sync_")==NULL) {
	// If directory is now not a sync directory
		syncFolderUnset(from);
	} else {
		char *syncedPathFrom = syncFolderGetSyncedPath(from);
		char *syncedPathTo = syncFolderGetSyncedPath(to);
		if (syncedPathFrom!=NULL || syncedPathTo!=NULL) {
			printf("\n\nrename sync path:%s\n%s\n",syncedPathFrom,syncedPathTo);
			if (syncedPathFrom==NULL || syncedPathTo==NULL) {
				// Cannot move between synced and unsyced folder
				printf("already renamed\n");
				return -1;
			} else {
				printf("Synced path is being renamed\n");
				int res;
				res = rename(syncedPathFrom,syncedPathTo);
				if (res == -1)
					return -errno;
				printf("Synced path is renamed\n\n\n");
			}
		}
	}


	int res;
	res = rename(from, to);
	if (res == -1)
		return -errno;

	printf("\n\n\n start encrypt dir1\n");
	if (strstr(tmpChrPFrom,"/encv1_")==NULL && strstr(tmpChrPTo,"/encv1_")!=NULL) {
	// If directory is now encrypted
		loggingCustom("ENCRYPTED","Type 1",from,to);
		encryptDir1(to);
	}

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
	logging("MKDIR",path,"");
	char *tmpChrP = strrchr(path,'/');
	if (strstr(tmpChrP,"/encv1_")!=NULL) {
	// If new directory is encrypted
		loggingCustom("CREATED","Type 1",path,"");
	}
	if (strstr(tmpChrP,"/encv2_")!=NULL) {
	// If new directory is encrypted
		loggingCustom("CREATED","Type 2",path,"");
	}
	char *syncedPathFrom;
	printf("\n\nSyncpath check\n");
	if ((syncedPathFrom=syncFolderGetSyncedPath(path))!=NULL) {
		printf("Syncpath not null\n");
		int res;
		res = mkdir(syncedPathFrom, mode);
		if (res == -1)
			return -errno;
	}

	int res;
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	logging("WRITE",path,"");

	char *syncedPathFrom;
	if ((syncedPathFrom=syncFolderGetSyncedPath(path))!=NULL) {
		int fd;
		int res;

		(void) fi;
		fd = open(syncedPathFrom, O_WRONLY);
		if (fd == -1)
			return -errno;

		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;

		close(fd);
		if (strstr(syncedPathFrom,"/encv2_")!=NULL) {
			encrypt2(syncedPathFrom);
		}

	}

	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	if (strstr(path,"/encv2_")!=NULL) {
		encrypt2(path);
	}
	return res;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	logging("CREATE",path,"");

	char *syncedPathFrom;
	if ((syncedPathFrom=syncFolderGetSyncedPath(path))!=NULL) {
		(void) fi;

		int res;
		res = creat(syncedPathFrom, mode);
		if(res == -1)
			return -errno;

		close(res);
		
		if (strstr(syncedPathFrom,"/encv1_")!=NULL) {
			encrypt1Rename(syncedPathFrom,10);
		}
	}

	(void) fi;

	int res;
	res = creat(path, mode);
	if(res == -1)
		return -errno;

	close(res);
	
	if (strstr(path,"/encv1_")!=NULL) {
		encrypt1Rename(path,10);
	}

	return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	// logging("GETATTR",path,"");

	int res;

	// printf("\ninputATTR:%s\n",path);
	char bufPath[500];
	strcpy(bufPath,path);
	char *tmpChrP;
	if ((tmpChrP = strstr(bufPath,"/encv1_"))!=NULL && (tmpChrP = strchr(tmpChrP+1,'/'))!=NULL){
		encryptRaw1(tmpChrP, 10);
	}

	// printf("getATTR:%s\n",bufPath);
	res = lstat(bufPath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	// logging("READDIR",path,"");

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	char bufPath[500];
	strcpy(bufPath,path);
	char *tmpChrP;
	if ((tmpChrP = strstr(bufPath,"/encv1_"))!=NULL && (tmpChrP = strchr(tmpChrP+1,'/'))!=NULL){
		encryptRaw1(tmpChrP, 10);
	}
	printf("\n\nopenDirPath:%s\n",bufPath);
	dp = opendir(bufPath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) {
			continue;
		} 
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		if (strstr(path,"/encv1_")!=NULL) {
			decryptRaw1(de->d_name,10);
		}
		printf("fillerName:%s\n\n",de->d_name);
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	// logging("ACCESS",path,"");

	// printf("\ninputAccESS:%s\n",path);
	char bufPath[500];
	strcpy(bufPath,path);
	char *tmpChrP;
	if ((tmpChrP = strstr(bufPath,"/encv1_"))!=NULL && (tmpChrP = strchr(tmpChrP+1,'/'))!=NULL){
		encryptRaw1(tmpChrP, 10);
	}


	// printf("AccESS:%s\n",bufPath);
	int res;
	res = access(bufPath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	logging("UNLINK",path,"");

	int res;
	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	logging("READLINK",path,"");

	int res;
	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	logging("MKNOD",path,"");

	int res;
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	logging("RMDIR",path,"");

	int res;
	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	logging("SYMLINK",from,to);

	int res;
	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	logging("LINK",from,to);

	int res;
	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	logging("CHMOD",path,"");

	int res;
	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	logging("CHOWN",path,"");

	int res;
	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	logging("TRUNCATE",path,"");

	int res;
	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	logging("UTIMENS",path,"");

	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	logging("OPEN",path,"");

	int res;
	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	logging("READ",path,"");

	int fd;
	int res;

	(void) fi;
	
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	logging("STATFS",path,"");

	int res;
	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	logging("RELEASE",path,"");

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	logging("FSYNC",path,"");

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
	logging("SETXATTR",path,"");

	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value, size_t size)
{
	logging("GETXATTR",path,"");

	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	logging("LISTXATTR",path,"");

	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	logging("REMOVEXATTR",path,"");

	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link			= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open			= xmp_open,
	.read			= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create 	= xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
