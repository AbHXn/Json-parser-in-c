#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include 	<string.h>
#include 	<ctype.h>
#include 	<stdbool.h>
#include 	<stdio.h>
#include 	<stdlib.h>

#define 	MAX_KEY_SIZE 	 250
#define 	MAX_VALUE_SIZE	 1024
#define 	INDEX_KEY_LEN 	 10
#define 	BUFFER_SIZE 	 250
#define 	MAX_SPACE 		 100
#define 	_EACH_BLOCK_SIZE (1024 * 2024 * 10)

typedef enum status_flags INPUT_STATUS;
typedef struct _memory _memory;


#define is_key_filled(flag) 	  ((flag) & KEY_ENTERED)
#define is_value_filled(flag) 	  ((flag) & VALUE_ENTERED)
#define is_in_filling_mode(flag)  ((flag) & SOMETHING_INPUTING)
#define is_list_filling(flag) 	  ((flag) & LIST_INPUTING)
#define turn_on_off(flag1, flag2) ((flag1) ^ (flag2))

// buffer is int to track of EOF
extern int 		BUFFER[BUFFER_SIZE];
extern int 		buffer_ptr;
extern FILE* 	JSON_FILE;
static _memory* _mem_head = NULL;

typedef enum {
	OPEN_C 		= '{',
	CLOSE_C		= '}',
	OPEN_S 		= '[',
	CLOSE_S 	= ']',
	APPO 		= '"',
	SEMI	 	= ':',
	COMMA 		= ','
}J_SYNTAX;


typedef struct _memory{
	unsigned char* mem;
	size_t curr_alloc;
	struct _memory* next_stack;
}_memory;

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
static const char* FILE_FAILED_OPEN		 = "Failed to open json File\n";
static const char* TREE_ALLOC_ERROR 	 = "Failed in tree creation\n";
static const char* BUFFER_LIMITED_ERROR  = "Buffer is full\n";
static const char* KEY_VALUE_ALLOC_ERROR = "Failed to create key/value\n";


bool push_char_buffer	( const int c );
bool push_buffer 		( const char* buf );

bool is_json_syntax			( char c );
void add_child_to_parent	( Node* parent, Node *child );
void traverse_tree			( Node *root, char *prefix, int is_last );
void recursivily_build		( Node *parent, int _flag );
Node *get_new_node			( const char *key, const char *value );


static inline bool is_value_filling( int _flag ){
	return !is_value_filled ( _flag ) && \
			is_in_filling_mode	( _flag );
}	
static inline bool is_value_pair( int _flag ){	
	return (
			is_key_filled	( _flag ) && 
		   is_value_filled	( _flag ) && 
		   !is_in_filling_mode	( _flag )
		   );
}
static inline void safe_push( char *str, int* index, 
				char cchar, const size_t MAX_SIZE ){
		if ( *index < MAX_SIZE - 1 )
			str[(*index)++] = cchar;
		else str[ *index ]= '\0';
}
static inline void terminate_string( char *str, const int index ){
	if( *( str  + index ) != '\0' )
		*( str + index ) = '\0';
}

static inline size_t align_up(size_t offset, size_t alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

static bool alloc_next( void ){
	_memory* new_block = malloc( sizeof( _memory ) );
	if( !new_block ) return false;

	new_block->mem = ( unsigned char* ) malloc( sizeof (unsigned char) * _EACH_BLOCK_SIZE );

	if( !new_block->mem ){
		free( new_block );
		return false;
	}

	new_block->curr_alloc = 0;
	new_block->next_stack = NULL;

	if ( !_mem_head  )
		_mem_head = new_block;
	else{
		new_block->next_stack = _mem_head;
		_mem_head = new_block;
	}

	return true;
}

static void* _malloc(size_t alloc_size){
	if( !_mem_head ||  _EACH_BLOCK_SIZE - _mem_head->curr_alloc < alloc_size )
		if( !alloc_next() )
			return NULL;

	size_t offset = align_up( _mem_head->curr_alloc, 8 );

	if( alloc_size > _EACH_BLOCK_SIZE - offset ){
		if( !alloc_next() )
			return NULL;
		offset = align_up( _mem_head->curr_alloc, 8 );
	}

	void* new_allc_ptr = _mem_head->mem + offset;
	_mem_head->curr_alloc = offset + alloc_size;
	
	return new_allc_ptr;	
}

static void free_memory( void ){
	while ( _mem_head ){
		_memory* next_node = _mem_head->next_stack;
		free ( _mem_head->mem );
		free ( _mem_head );
		_mem_head = next_node;
	}
}

#endif // JSON_PARSER_H
