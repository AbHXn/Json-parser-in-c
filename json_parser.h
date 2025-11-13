#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include 	<string.h>
#include 	<ctype.h>
#include 	<stdbool.h>
#include 	<stdio.h>
#include 	<stdlib.h>

#define 	MAX_KEY_SIZE 	250
#define 	MAX_VALUE_SIZE	1024
#define 	INDEX_KEY_LEN 	10
#define 	BUFFER_SIZE 	250
#define 	MAX_SPACE 		100

typedef enum status_flags INPUT_STATUS;

#define is_key_filled(flag) 	  ((flag) & KEY_ENTERED)
#define is_value_filled(flag) 	  ((flag) & VALUE_ENTERED)
#define is_in_filling_mode(flag)  ((flag) & SOMETHING_INPUTING)
#define is_list_filling(flag) 	  ((flag) & LIST_INPUTING)
#define turn_on_off(flag1, flag2) ((flag1) ^ (flag2))

// buffer is int to track of EOF
extern int 		BUFFER[BUFFER_SIZE];
extern int 		buffer_ptr;
extern FILE* 	JSON_FILE;

typedef enum {
	OPEN_C 		= '{',
	CLOSE_C		= '}',
	OPEN_S 		= '[',
	CLOSE_S 	= ']',
	APPO 		= '"',
	SEMI	 	= ':',
	COMMA 		= ','
}J_SYNTAX;

typedef enum status_flags{
	SOMETHING_INPUTING		= 1 << 0,
	VALUE_ENTERED			= 1 << 1,
	KEY_ENTERED 			= 1 << 2,
	LIST_INPUTING			= 1 << 3
}INPUT_STATUS;

typedef struct next_children{
	struct _node*		  child;
	struct next_children* next;
}ChD;

typedef struct _node{
	char*	key;
   	char*	value;
	ChD*	childrens;
}Node;

static const char* FILE_NOT_PROVIDED	 = "Provide Json File\n";
static const char* TREE_ALLOC_ERROR 	 = "Failed in tree creation\n";
static const char* BUFFER_LIMITED_ERROR  = "Buffer is full\n";
static const char* KEY_VALUE_ALLOC_ERROR = "Failed to create key/value\n";


bool push_char_buffer	( const int c );
bool push_buffer 		( const char* buf );
char _fgetc 			( void );

bool is_value_filling	( int _flag );
bool is_value_pair		( int _flag );

bool is_json_syntax			( char c );
void add_child_to_parent	( Node* parent, Node *child );
void traverse_tree			( Node *root, char *prefix, int is_last );
void recursivily_build		( Node *parent, int _flag );
void free_entire_tree		( Node *root );
Node *get_new_node			( const char *key, const char *value );


#endif // JSON_PARSER_H
