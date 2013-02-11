/********************************************************************
	created:	2005/10/17											
	created:	17:10:2005   18:20									
	filename: 	ini_manage.h											
	file path:	..\INIManipulate											
	file base:	ini_manage											
	file ext:	h													
	author:		Chanchai Jaiyutitam									
																				
	purpose:	Header for manipulate ini file					

	updated:	2013/02/11
	updated by: 173210
*********************************************************************/

#ifndef INI_MANAGE_H
#define INI_MANAGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMENT		'#'
#define INI_ERROR	-1

#ifndef __cplusplus
typedef enum 
{
	true = 1,
	false = 0
}bool;
#endif

/************************************************************************/
/* type def                                                             */
/************************************************************************/
typedef enum
{
	REPLACE = 1,
	NON_REPLACE = 0
}REPLACE_FLAG;

/************************************************************************/
/* for record of ini content                                            */
/************************************************************************/
struct record
{
	char comments[255];
	char key[255];
	char value[255];
	struct record *next;
};

/************************************************************************/
/* for content of file                                                  */
/************************************************************************/
struct section
{
	struct record *data_first;
	struct record *data_last;
	unsigned int data_size;
	char comments[255];
	char name[255];
	struct section *next;
};
/************************************************************************/
/* for content list                                                     */
/************************************************************************/
typedef struct
{
	unsigned int section_size;
	struct section *first;
	struct section *last;
}content;

// global variable [10/18/2005]
content *ini_content;
char gfilename[255];
char *error_msg;
REPLACE_FLAG w_flag;

/************************************************************************/
/* main ini manage function                                             */
/************************************************************************/
/**
 *	function: ini_start														
 *	parameter:														
 *		@filename															
 *	return:	true if success														
 *	purpose: for start ini manipulate file														
 */
bool ini_start(const char* filename);

/**
 *	function:	load													
 *	parameter:														
 *		@filename															
 *	return:		true if success													
 *	purpose:	for load ini file to content													
 */

bool load(const char *filename);

/**
 *	function:	_save													
 *	parameter:														
 *		@filename															
 *	return:		true if success													
 *	purpose:	save content to ini file(save as)													
 */
bool save();	// save to load filebool 
bool save_as(const char *filename);

/**
 *	function:	get_value													
 *	parameter:														
 *		@															
 *	return:		value
 *	purpose:	§													
 */
char *get_value (const char *sec,const char *key);
char *_get_value(const char *sec,const char *key,		// return data and comment
				 char *comment);
/**
 *	function:	set_value													
 *	parameter:														
 *		@															
 *	return:		true if success
 *	purpose:			
 */
bool set_value  (const char *sec,const char *key,		// will auto replace
				 const char *value);	
bool _set_value (const char *sec,const char *key,		// select replace or not replace
				 const char *value,const char *comment,REPLACE_FLAG flag);

int  remove_sel (const char *sec,char *key);
int  remove_all (const char * sec);				// remove all record in section

// add/remove section
void add_section(const char *sec,const char *comment);	// add section
int  remove_section(char *sec);				// remove section (remove all record in section if not empty)

void clear();								// clear all content

// size of section
int  content_size();
int  section_size(char *sec);

// for console display use stdio.h stdout
void print_content();						// print all content
void print_section();						// print all only section
void print_record(char *sec,char *key);		// print selection record
void print_allrecord(char *sec);			// print all record in section

/**
 *	function: ini_end													
 *	parameter:														
 *		none																	
 *	return:	void														
 *	purpose: for end ini manipulate file													
 */
void ini_end();
void _ini_end(REPLACE_FLAG flag);

/**
 *	function: get_last_error														
 *	parameter:														
 *		none															
 *	return:	type of error														
 *	purpose:	for get error													
 */
char *get_last_error();

/************************************************************************/
/*Helper function                                                       */
/************************************************************************/ 
// initaial data/save
void init_content();
bool _save(const char *filename);
// add/remove record
void _append(const char *sec,const char *key,			// append data to section
			 const char *value,const char *comment);											
int  _remove(const char *sec,const char *key);
int  _remove_all(struct section *sec);
// display function
void _print_allrecord(struct record *sec);	// print all record in section
// search section
struct section *get_section(const char *sec);		// search section
struct record *get_record(struct section *sec,const char *key);	// get record
// trim
void trim(char *buffer);
#endif