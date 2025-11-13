#include "json_parser.h"

int 		BUFFER[BUFFER_SIZE];
int 		buffer_ptr = -1;
FILE* 		JSON_FILE = NULL;


bool push_char_buffer( const int c ){
	if( buffer_ptr + 1 >= BUFFER_SIZE ){
		fprintf( stderr, BUFFER_LIMITED_ERROR );
		return false;
	}
	// else push the char to buffer stack
	BUFFER[++buffer_ptr] = c;
}
// another option is top push a string
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
// if buffer is not empty then return the value
char _fgetc( void ){
	return ( buffer_ptr > -1 ) ? BUFFER[ buffer_ptr-- ]: fgetc( JSON_FILE );
}

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
	}
	nnode->childrens = NULL;
	return nnode;
}

bool is_key_filled(int _flag){
	return _flag & KEY_IS_ENTERED;
}
bool is_value_filled(int _flag){
	return _flag & VALUE_ENTERED;
}
bool is_in_filling_mode( int _flag){
	return _flag & SOMETHING_INPUTING;
}
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
bool is_list_inputing(int _flag){
	return _flag & INPUTING_LIST;
}

int turn_on_off(int _flag, int _t_flag) {
	return _flag ^= _t_flag;
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

// insertion at end single linked list
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

// beautiful method for printing a treeeeee :) ! 
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

void recursivily_build(Node *parent, int _flag){
	char 		key[MAX_KEY_SIZE]		;
	char 		value[MAX_VALUE_SIZE]	;
	int 		list_index = 0			; // for list cases
	int 		k_index					;
	int 		v_index					;
	int 		c_char					;
	
	k_index = v_index 	= 0;
	Node *node_to_push 	= NULL;

	while( ( c_char = _fgetc() ) != EOF ){
		if( c_char == CLOSE_C || c_char == CLOSE_S ){
			// in list last value does not seperate with comma
			if( c_char == CLOSE_S ) 
				c_char = COMMA;
			// just checking if next is comma or not
			int temp = _fgetc();
			// if not comma then put comma to trigger
			push_char_buffer( temp );
			if( temp != COMMA )
				push_char_buffer( COMMA );
			push_char_buffer( EOF ); // EOF TRIGGERING
			// check if value is in write mode
			// if it is then change the modes
			if(	is_key_filled( _flag ) && is_value_filling( _flag ) ){
				// toogle the corresponding bits
				_flag = turn_on_off(_flag, SOMETHING_INPUTING | VALUE_ENTERED);
				value[ v_index ] = '\0';
			}
		}
		// if it is not a json syntax
		if(!is_json_syntax(c_char)){
			// i am skiping space case...
			if(isspace(c_char) && !is_in_filling_mode(_flag)) continue;
			//if not space then it will be a key or value
			// fill accordinly 
			if( !is_in_filling_mode( _flag ) )
				_flag = turn_on_off(_flag, SOMETHING_INPUTING);
			// if key is not filled then fill the key first
			if( !is_key_filled( _flag ) ){
				if( k_index == MAX_KEY_SIZE - 1 )
					key[ k_index ] = '\0';
				else key[ k_index++ ] = c_char;
			}
			// else fill the value
			else if( !is_value_filled( _flag ) ){
				if( v_index == MAX_VALUE_SIZE - 1 )
					value[ v_index ] = '\0';
				else value[ v_index++ ] = c_char;
			}
		}
		// this is the case where the key_value mode is created
		// APPO represents the end of key or value
		else if( c_char == APPO ){
			// if not in filling mode then change it.
			if( !is_in_filling_mode( _flag ) )
				_flag = turn_on_off(_flag, SOMETHING_INPUTING);
			else{
				// if it is in filling mode and key is not completed then now its complete
				if( !is_key_filled( _flag ) ){
					_flag = turn_on_off(_flag, KEY_IS_ENTERED | SOMETHING_INPUTING);
					key[ k_index ] = '\0';
				}
				// if key is filled that means value is filling... 
				else{
					_flag = turn_on_off(_flag, VALUE_ENTERED | SOMETHING_INPUTING);
					value[ v_index ] = '\0';
				}
			}
		}
		// if its a json syntanx other than closing or appo
		else if( is_json_syntax( c_char ) ){
			// if we encounter semi but it is a part of value then skip it
			if( c_char == SEMI && is_value_filling( _flag ) ){
				// also check if value reached its limit
				if( v_index == MAX_VALUE_SIZE - 1 )
					value[ v_index ] = '\0';
				else value[ v_index++ ] = c_char;
				continue;
			}
			if( c_char == OPEN_C ){
				// fresh new recursive
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
			// if it is opening square (LIST) index will become key/
			else if(c_char == OPEN_S){
				char 	index_to_key[MAX_KEY_SIZE];
				Node 	*key_node = NULL;
				// convert index number into digit
				sprintf(index_to_key, "%d", list_index);
				// if key is not filled and not currently filling then 
				// high chance it is a list
				if( !is_key_filled( _flag ) && !is_in_filling_mode( _flag ) ){
					strcpy( key, "LIST" );
				}
				// get the key_node
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
			// COMMA represents the final stage. key and value is ready
			// but there is some cases comma is not important 
			else if( c_char == COMMA ){
				if( is_in_filling_mode( _flag ) ){
					_flag = turn_on_off(_flag, VALUE_ENTERED | SOMETHING_INPUTING);
					value[ v_index ] = '\0';
				}
				if( node_to_push ){
					add_child_to_parent( parent, node_to_push );
					node_to_push = NULL;
					k_index = v_index = 0;
					_flag = ( is_list_inputing( _flag ) ) ? 0b1001: 0b0000; 
				}
				if( is_list_inputing( _flag ) ){
					list_index += 1;
					char index_to_key[ MAX_KEY_SIZE ];
					sprintf( index_to_key, "%d", list_index );
					push_char_buffer( '"' );
					push_buffer( index_to_key );
				}
			}
		}
		// at finaly key_value_pair mode is activated then 
		// it is the time to push to parent node
		if( is_value_pair( _flag ) ){
			if( is_list_inputing( _flag ) && c_char != COMMA )
				continue;
			Node* new_node = get_new_node( key, value );
			if( !new_node ) break;
			add_child_to_parent( parent, new_node );
			k_index = v_index = 0;
			_flag = ( is_list_inputing( _flag ) ) ? 0b1001: 0b0000;
		}
		
	}
}

// it is better to free entire tree 
// after execution to prevent mem leak
void free_entire_tree( Node *root ){
	if( !root ) return;

	ChD* children = root->childrens;
	while( children ){
		ChD *next = children->next;
		free_entire_tree( children->child );
		// childrens allocated seperate mem so free it.
		free( children );
		children = next;
	}

	free( root->key  );
	free( root->value);
	root->childrens = NULL;
	free( root );
	
}
