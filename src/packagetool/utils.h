/*
    Copyright (C) 2015 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

     This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    BSD_License.txt file for more details.

    You should have received a copy of the BSD_License.txt file
    along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdlib.h>

#include <mlle_cr_decrypt.h>

#define NO_OF_LVE 4
#define MAX_PATH_LENGTH 2048
#define LONG_FILE_NAME_MAX 4096
#define MAX_STRING 200

#if defined(DEBUG) || defined(_DEBUG)
    #define DEBUG_PRINT(...) fprintf( stderr, __VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif

#if defined(WIN32) && !defined(__func__)
    #define __func__ __FUNCTION__
#endif


/*************************
 * Free allocated space.
 ************************/
void cleanUp();

/********************************************
 * Returns path to the .library directory.
 *******************************************/
char *getDotLibraryPath();

/**************************************************************
 * Returns array with the lve's that was copied to .library.
 *************************************************************/
char **getLveList();


/***********************************
 * Returns path to the icon file.
 **********************************/
char *getIconPath();


/**********************************************
 * Create array to add copied file names.
 *
 * Returns:
 *      1 - allocating array was a succes.
 *      0 - failed to allocate array.
 *********************************************/
int createCopyArray();

/*********************************************************
 * Check if the a path is a directory.
 *
 * Parameters:
 *      path - path to check.
 *
 * Returns:
 *      1 - path is a directory.
 *      0 - path is not a directory.
 ********************************************************/
int isDirectory(char *path);


/********************************************************
 * Validate a path.
 *
 * Parameters:
 *      path - path to check.
 *
 * Returns:
 *      1 - path is valid.
 *      0 - path is not valid.
 *******************************************************/
int validatePath(char *path);


/***********************************************
 * Check if encryption of files is activated.
 *
 * Returns:
 *      1 - encryption is activated.
 *      0 - encryption is not activated.
 **********************************************/
int usingEncryption();


/***********************************************************
 * Create the .library folder in the top-level directory.
 * The manifest.xml file and the LVE executables will
 * be copied to this folder.
 *
 * Returns:
 *      1 - creating directory was successful.
 *      0 - creating directory failed.
 **********************************************************/
int createLibraryFolder();

/**************************************
 * Convert a string to lower case.
 *
 * Parameters:
 *      str - the string to convert.
 *
 * Returns:
 *      The converted string.
 *************************************/
char *stringToLower(char *str);


/*********************************************************
 * Get the current working directory.
 *
 * Parameters:
 *      cwd - array to store path to working directory.
 *
 * Returns:
 *      1 - working directory was found.
 *      0 - failed to get working directory.
 *********************************************************/
int getCurrentDirectory(char **cwd);


/**************************************************************
 * Get the directory where the executable is running from.
 *
 * Returns:
 * 		Path to executable directory or NULL.
 *************************************************************/
char *getExecutableDirectory();

/******************************************
 * Count number of LVE's.
 *
 * Returns:
 *      Number of LVE's that was found.
 *****************************************/
int countLVE();


/**************************************************************
 * Copy all the LVE's from one folder to the .library folder.
 *
 * Returns:
 *      1 - copying of files was successful.
 *      0 - copying files failed.
 **************************************************************/
int copyLVE();


/*********************************************
 * Copy a file from one location to another.
 *
 * Parameters:
 *		filename - name of file to copy.
 * 		pathFrom - path and filename to copy.
 *
 * Returns:
 *      1 - copying of files was successful.
 *      0 - copying files failed.
 *********************************************/
int copyFile(char *filename, char *pathFrom);

/***************************************************************
 * Check if a file or directory exists.
 *
 * Parameters:
 *      path- path with or without filename.
 *
 * Returns:
 *      1 - file/directory exists
 *      0 - file/directory doesn't exist.
 ***************************************************************/
int fileExists(char *pahtAndFilename);


/***********************************************************
 * Prepares the icon file by finding it and remove the
 * tmp folder path and library folder name from the path.
 *
 * Returns:
 *      1 - the icon file was found and path was fixed.
 *      0 - failed to find the file or to fix the path.
 **********************************************************/
int prepareIconFile();

/*************************************************************
 * Locate the icon file in the source directory.
 * The icon path in the argument can point to a folder anywhere.
 * We must check that this icon file is in the source folder.
 *
 * Returns:
 *      1 - the file was found.
 *      0 - failed to find the file.
 *************************************************************/
int locateIconFile();


/*************************************************************
 * Remove temp path and library name from the icon file path.
 *
 * Returns:
 *      1 - the file was found.
 *      0 - failed to find the file.
 **********************************************************/
int cleanUpIconPath();


/****************************************************************
 * Traverse a directory structure on Windows/Linux and find the file.
 *
 * Parameters:
 *      path - the path to search from.
 *      filename     - the file to find.
 *
 * Returns:
 *      1 - the file was found.
 *      0 - failed to find the file.
 ****************************************************************/
int findFileWin32(const char *filename, const char *path);
int findFileLinux(const char *filename, const char *path);

/****************************************************************
* Traverse a directory structure on Windows/Linux and encrypt modelica 
* files.
*
* Parameters:
*      topLevelPath - base directory for the library.
*      relPath      - the subdirectory to process.
*      context_in - context for encryption (NULL for top level when relPath is NULL)
*
* Returns:
*      1 - encryption succeeded.
*      0 - failed .
****************************************************************/
int encryptDirectoryWin32(const char *topLevelPath, const char* relPath, mlle_cr_context* context_in);
int encryptDirectoryLinux(const char *topLevelPath, const char* relPath, mlle_cr_context* context_in);

/**********************************************
 * Extract the filename from a path.
 *
 * Parameters:
 *      path - the path including filename.
 *
 * Returns:
 *      Filename if success, NULL otherwise.
 *********************************************/
char *extractFilename(char *path);

/**********************************************
 * Removes filename from a path.
 *
 * Parameters:
 *      path - the path including filename.
 *
 * Returns:
 *      Path if success, NULL otherwise.
 *********************************************/
char *extractPath(char *pathAndFilename);



/************************************
 * Encrypt files.
 *
 * Returns:
 *      1 - encryption successful.
 *      0 - encryption failed.
 ***********************************/
int encryptFiles();


/*************************************************************
 * Check if a file is a Modelica file (file extension .mo).
 *
 * Parameters:
 *      filename - name of file to check.
 *
 * Returns:
 *      1 - it's a Modelica file.
 *      0 - it' not a Modelica file.
 ************************************************************/
int isModelicaFile(char *filename);


/**************************************************
 * Check if a file is an encrypted Modelica file,
 * i.e. file extension is .moc.
 *
 * Parameters:
 *      filename - name of file to check.
 *
 * Returns:
 *      1 - the file is encrypted.
 *      0 - the file is not encrypted.
 *************************************************/
int isEncryptedFile(char *filename);


/*****************************************************
 * Encrypt a Modelica file.
 *
 * Parameters:
 *      context - encryption context
 *      basedir - top level library directory
 *      rel_file_path - path and filename of file to encrypt.
 *
 * Returns:
 *      1 - encryption was successful.
 *      0 - encryption failed.
 ****************************************************/
int encryptFile(mlle_cr_context*context, const char* basedir, const char *rel_file_path);



/**********************************************
 * Create a zipped archive.
 * The file extension on the archive is .mol
 * and not .zip.
 *
 * Returns.
 *      1 - successfully created the archive.
 *      0 - creating archive failed.
 *********************************************/
int createZipArchive();


/******************************************************************
 * Creates a zipped archive of a directory on Windows.
 *
 * Parameters:
 *      path - path to the top-level directory.
 *      archiveName - the name of the archive.
 *      encrypted - does the archive contains encrypted files (1)
 *                  or not (0).
 *
 * Returns:
 *      1 - successfully created the archive.
 *      0 - creating archive failed.
 *****************************************************************/
int zipDirectoryWin32(char *path, char *archiveName, int encrypted);

/******************************************************************
 * Creates a zipped archive of a directory on Linux.
 *
 * Parameters:
 *      path - path to the top-level directory.
 *      archiveName - the name of the archive.
 *      encrypted - does the archive contains encrypted files (1)
 *                  or not (0).
 *
 * Returns:
 *      1 - successfully created the archive.
 *      0 - creating archive failed.
 *****************************************************************/
int zipDirectoryLinux(char *path, char *archiveName, int encrypted);



/**************************************************************
 * Removes the tmp folder name from the incoming path.
 * This needs to be done before adding path to zip container.
 *
 * Parameters:
 *      searchPath - path to remove from.
 *      zipPath    - a path without the tmp folder in it.
 *      zipPathLen - length of zipPath buffer
 *
 * Returns:
 *      1 - successfully created the path.
 *      0 - creating path failed.
 *************************************************************/
int removeTmpFolderName(char *searchPath, char *zipPath, size_t zipPathLen);




/*************************************************************************
 * Creates a copy of the folder structure we are making a container of.
 * The folder structure is copied to the users TEMP folder.
 *
 * Returns:
 *      1 if successful, 0 otherwise.
 ************************************************************************/
int copyFolderStructure();

/*************************************************************
 * Copy a Windows directory including files and subdirectory
 * from one location to another.
 *
 * Parameters:
 *      fromPath - the path to copy from.
 *      toPath   - the path to copy to.
 *
 * Returns:
 *      1 on success, 0 otherwise.
 ***********************************************************/
int copyDirectoryWin32(char *fromPath, char *toPath);


/*************************************************************
 * Copy a Linux directory including files and subdirectory
 * from one location to another.
 *
 * Parameters:
 *      fromPath - the path to copy from.
 *      toPath   - the path to copy to.
 *
 * Returns:
 *      1 on success, 0 otherwise.
 ***********************************************************/
int copyDirectoryLinux(char *fromPath, char *toPath);



/***********************************************
 * Delete the temporary source folder that was
 * previously copied to TEMP folder.
 *
 * Returns:
 *      1 on success, 0 otherwise.
 **********************************************/
int deleteTemporaryStagingFolder();




