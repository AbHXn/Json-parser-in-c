/* 
	Author: AbHXn
	Created at: 20 oct. 2025
	Description: Simple Json Parser in C
	Disclaimer:
	 	This code is designed to work only with properly formatted JSON files (inside text).
	 	It assumes that input data follows correct JSON syntax and structure.
	  
		No validation is performed for security, malicious input, or malformed data.
		The code does NOT include checks for:
		- Injection attacks 
		- File access vulnerabilities
		- Data integrity or trustworthiness

		Use at your own risk. The author assumes no responsibility for damage,
		data loss, or security breaches resulting from the use of this code.
	
	Conditions of Use:
	 	For educational or non-commercial use only.
	 	Not suitable for use in systems requiring secure or validated input.
	 	Redistribution must retain this disclaimer.
*/

#include <stdio.h>
#include "json_parser.h"

extern FILE* JSON_FILE;

int main( int argc, char *argv[] ){
	if( argc != 2 ){
		fprintf( stderr, FILE_NOT_PROVIDED );
		return -1;
	}
	const char*	file = argv[ 1 ];
	JSON_FILE = fopen( file, "r" );
    
	if( !JSON_FILE ) return -1;
	
	Node *parent = get_new_node( "/", NULL );
	char space[ MAX_SPACE ];
	
	recursivily_build( parent, 0b0000 );
	traverse_tree( parent, space, 0 );
	
	fclose( JSON_FILE );
	free_entire_tree( parent );
	return 0;
}
