#include "json_parser.h"

int 		BUFFER[BUFFER_SIZE];
int 		buffer_ptr = -1;
FILE* 		JSON_FILE = NULL;

bool is_value_filling( int _flag ){
	return !is_value_filled ( _flag ) && \
			is_in_filling_mode	( _flag );
}	
bool is_value_pair( int _flag ){	
	return (
			is_key_filled	( _flag ) && 
		   is_value_filled	( _flag ) && 
		   !is_in_filling_mode	( _flag )
		   );
}
void safe_push( char *str, int* index, 
				char cchar, const size_t MAX_SIZE ){
		if ( *index < MAX_SIZE - 1 )
			str[(*index)++] = cchar;
		else str[ *index ]= '\0';
}
void terminate_string( char *str, const int index ){
	if( *( str  + index ) != '\0' )
		*( str + index ) = '\0';
}

/**
 * Buffered character input and push functions.
 * These functions provide a simple character buffer mechanism similar to `ungetc`:
 *   - `push_char_buffer(int c)`: Pushes a single character onto the buffer.
 *  
 	 - `push_buffer(const char* buf)`: Pushes a string onto the buffer in reverse order, 
 *                                     so that characters are read in correct sequence.
 *  
 	 - `_fgetc(void)`: Reads a character from the buffer if available; 
 *                     otherwise, reads from the input file.
 *
 * Notes:
 * - Buffer overflow is checked, and an error is printed if the buffer exceeds its limit.
 * - Allows recursive parsing or lookahead by pushing characters back into the input stream.
 */

bool push_char_buffer( const int c ){
	if( buffer_ptr + 1 >= BUFFER_SIZE ){
		fprintf( stderr, BUFFER_LIMITED_ERROR );
		return false;
	}
	BUFFER[++buffer_ptr] = c;
	return true;
}

bool push_buffer( const char* buf ){
	int buff_len = strlen(buf);
	if( buffer_ptr + buff_len >= BUFFER_SIZE ){
		fprintf(stderr, BUFFER_LIMITED_ERROR );
		return false;
	}
	buff_len--;
	while( buff_len > -1 )
		BUFFER[ ++buffer_ptr ] = buf[ buff_len-- ];
	return true;
}

char _fgetc( void ){
	return ( buffer_ptr > -1 ) ? \
		BUFFER[ buffer_ptr-- ]: fgetc( JSON_FILE );
}

/**
 * Adds a child Node to the end of a parent's linked list of children.
 *
 * Allocates a new `ChD` structure to link the child Node. If the parent has no children,
 * the new child becomes the first; otherwise, it is appended at the end of the list.
 *
 * @param parent Pointer to the parent Node.
 * @param child  Pointer to the child Node to be added.
 *
 * Notes:
 * - Prints an error if memory allocation fails.
 * - Uses a singly linked list for child nodes.
 */


Node *get_new_node( const char *key, const char *value ){
	Node *nnode = (Node *) malloc( sizeof( Node ) );
	if( !nnode ){
		fprintf( stderr, TREE_ALLOC_ERROR );
		return NULL;
	}
	nnode->key = (char *) malloc( strlen(key) + 1 );
	if( !nnode->key ){
		fprintf( stderr, KEY_VALUE_ALLOC_ERROR );
		free( nnode );
		return NULL;
	}
	strcpy( nnode->key, key );
	if( value ){
		nnode->value = (char *) malloc( strlen(value) + 1 );
		strcpy( nnode->value, value );
	} else nnode->value = NULL;
	nnode->childrens = NULL;
	return nnode;
}

bool is_json_syntax( char c ){
	switch( c ) {
		case OPEN_C	: case CLOSE_C	:
		case OPEN_S	: case APPO		:
		case SEMI	: case CLOSE_S	:
		case COMMA	:
			return true;
		default:
			return false;
	}
}

/*
	Insertion at end (single linked list)
*/ 
void add_child_to_parent( Node* parent, Node *child ){
	ChD *new_child = (ChD *) malloc( sizeof(ChD) );
	if( new_child == NULL ){
		fprintf( stderr, TREE_ALLOC_ERROR );
		return;
	}
	new_child->next = NULL;
	new_child->child = child;

	if( !parent->childrens )
		parent->childrens = new_child;
	else{
		ChD* temp = parent->childrens;
		while( temp->next )
			temp = temp->next;
		temp->next = new_child;
	}
}

/**
 * Recursively prints a tree of Nodes in a structured, visual format.
 * Each node is displayed with its key and value, using ASCII tree branches
 * ("├──", "└──") to represent hierarchy. The function calculates child positions
 * to determine which nodes are last in their sibling list.
 *
 * @param root     Pointer to the root Node to print.
 * @param prefix   String used for indentation and branch formatting.
 * @param is_last  Flag indicating whether the current node is the last among its siblings.
 * Notes:
 * - Nodes with no value are displayed as "/".
 * - Handles any tree depth recursively.
 */

void traverse_tree(Node *root, char *prefix, int is_last) {
    if ( !root ) return;

	char 	new_prefix[MAX_SPACE];
	int 	count = 0;
	int 	index = 0;

	printf("%s%s%s : %s\n"				 		, 
			prefix						 		, 
			( is_last ) ? "└── " : "├── "		, 
			root->key							, 
			( root->value ) ? root->value : "/"
	);
	snprintf(new_prefix, MAX_SPACE, "%s%s", prefix, is_last ? "    " : "│   ");
	ChD *tmp = root->childrens;
	while ( tmp ) {
		count++;
		tmp = tmp->next;
	}
	ChD *child = root->childrens;
	while ( child ) {
		traverse_tree( child->child, new_prefix, index == count - 1 );
		child = child->next;
		index++;
	}
    
}

/**
 * Recursively builds a tree of Nodes from a JSON-like input stream.
 *
 * Reads characters and constructs a hierarchical tree of key-value pairs and lists.
 * Supports nested objects `{}` and arrays `[]`, automatically assigning numeric keys for list elements.
 * Tracks parsing state using `_flag` to indicate:
 *   - whether a key or value is being filled or completed,
 *   - whether the parser is inside a list,
 *   - and other filling modes.
 * Finalizes nodes on commas or closing brackets, ensuring proper null-termination of key and value buffers.
 *
 * @param parent Pointer to the parent Node to which children are added.
 * @param _flag  Bitmask representing the current parsing state.
 *
 * Notes:
 * - Input must be well-formed; no strict JSON validation is performed.
 * - Recursion occurs for nested objects or lists.
*/

void recursivily_build(Node *parent, int _flag){
	char 	key[MAX_KEY_SIZE];
	char 	value[MAX_VALUE_SIZE];
	int 	list_index = 0; 
	int 	k_index;
	int 	v_index;
	int 	c_char;
	
	k_index = v_index 	= 0;
	Node *node_to_push 	= NULL;

	while( ( c_char = _fgetc() ) != EOF ){
		if( c_char == CLOSE_C || c_char == CLOSE_S ){
			if( c_char == CLOSE_S ) c_char = COMMA;
			
			int temp = _fgetc();
			push_char_buffer( temp );
			
			if( temp != COMMA )
				push_char_buffer( COMMA );
			push_char_buffer( EOF ); 
			
			if(	is_key_filled( _flag ) && is_value_filling( _flag ) ){
				_flag = turn_on_off(_flag, SOMETHING_INPUTING | VALUE_ENTERED);
				terminate_string( value, v_index );
			}
		}
		if( !is_json_syntax(c_char) ){
			if(isspace(c_char) && !is_in_filling_mode(_flag)) 
				continue;

			if( !is_in_filling_mode( _flag ) )
				_flag = turn_on_off( _flag, SOMETHING_INPUTING );
			
			if( !is_key_filled( _flag ) )
				safe_push( key, &k_index, c_char, MAX_KEY_SIZE );
			else if( !is_value_filled( _flag ) )
				safe_push( value, &v_index, c_char, MAX_KEY_SIZE );
		}
		else if( c_char == APPO ){
			if( !is_in_filling_mode( _flag ) )
				_flag = turn_on_off(_flag, SOMETHING_INPUTING);
			else{
				if( !is_key_filled( _flag ) ){
					_flag = turn_on_off(
						_flag, KEY_ENTERED | SOMETHING_INPUTING);
					terminate_string( key, k_index );
				}
				else{
					_flag = turn_on_off(
						_flag, VALUE_ENTERED | SOMETHING_INPUTING);
					terminate_string( value, v_index );
				}
			}
		}
		else if( is_json_syntax( c_char ) ){
			if( c_char == SEMI && is_value_filling( _flag ) ){
				safe_push( value, &v_index, c_char, MAX_KEY_SIZE );
				continue;
			}
			if( c_char == OPEN_C ){
				if( is_key_filled( _flag ) ){
					Node *new_node = get_new_node(key, NULL);
					if( !new_node )
						push_char_buffer(EOF);
					else{
						recursivily_build(new_node, 0b0000);
						node_to_push = new_node;
					}
				}
			}
			else if(c_char == OPEN_S){
				char 	index_to_key[MAX_KEY_SIZE];
				Node 	*key_node = NULL;
				sprintf(index_to_key, "%d", list_index);

				if( !is_key_filled( _flag ) && !is_in_filling_mode( _flag ) ){
					strcpy( key, "LIST" );
				}
				key_node = get_new_node( key, NULL );	
				
				if( !key_node )
					push_char_buffer( EOF );
				else{
					push_char_buffer( '"' );
					push_buffer( index_to_key );
					recursivily_build( key_node, 0b1001 );
					node_to_push = key_node;
				}
			}
			else if( c_char == COMMA ){
				if( is_in_filling_mode( _flag ) ){
					_flag = turn_on_off(_flag, 
						VALUE_ENTERED | SOMETHING_INPUTING);
					terminate_string( value, v_index );
				}
				if( node_to_push ){
					add_child_to_parent( parent, node_to_push );
					node_to_push = NULL;
					k_index = v_index = 0;
					_flag = ( is_list_filling( _flag ) ) ? 0b1001: 0b0000; 
				}
				if( is_list_filling( _flag ) ){
					list_index += 1;
					char index_to_key[ MAX_KEY_SIZE ];
					sprintf( index_to_key, "%d", list_index );
					push_char_buffer( '"' );
					push_buffer( index_to_key );
				}
			}
		}

		if( is_value_pair( _flag ) ){
			if( is_list_filling( _flag ) && c_char != COMMA )
				continue;

			Node* new_node = get_new_node( key, value );
			if( !new_node ) break;
			
			add_child_to_parent( parent, new_node );
			k_index = v_index = 0;
			_flag = ( is_list_filling( _flag ) ) ? 0b1001: 0b0000;
		}
		
	}
}

/**
 * Recursively frees a tree of Nodes to prevent memory leaks.
 *
 * Traverses all child nodes and deallocates their memory, including:
 *   - the key and value strings,
 *   - child nodes,
 *   - and the linked list of children.
 *
 * @param root Pointer to the root Node of the tree to free.
 *
 * Notes:
 * - Safe to call with NULL (does nothing).
 * - After execution, all memory associated with the tree is released.
 */

void free_entire_tree( Node *root ){
	if( !root ) return;

	ChD* children = root->childrens;
	while( children ){
		ChD *next = children->next;
		free_entire_tree( children->child );
		free( children );
		children = next;
	}

	if ( root->key ) free( root->key  );
	if ( root->value ) free( root->value );
	root->childrens = NULL;
	free( root );
}
