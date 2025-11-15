#include <stdio.h>
#include "json_parser.h"

extern FILE* JSON_FILE;

/**
 * Entry point for the Simple JSON Parser.
 *
 * Reads a JSON file, builds a tree of Nodes, prints the tree, and frees memory.
 *
 * Usage:
 *   ./program <json_file>
 *
 * Steps:
 * 1. Validates command-line arguments.
 * 2. Opens the specified JSON file.
 * 3. Creates the root Node for the JSON tree.
 * 4. Recursively builds the tree from the file content.
 * 5. Traverses and prints the tree in a structured format.
 * 6. Closes the file and frees all allocated memory.
 *
 * Notes:
 * - Assumes the input JSON file is properly formatted.
 * - Exits with -1 on errors (invalid arguments or file open failure).
 */

int main( int argc, char *argv[] ){
	if( argc != 2 ){
		fprintf( stderr, FILE_NOT_PROVIDED );
		return -1;
	}
	const char*	file = argv[ 1 ];
	JSON_FILE = fopen( file, "r" );

	if( !JSON_FILE ) {
		fprintf(stderr, FILE_FAILED_OPEN);
		return -1;
	}

	Node *parent  = get_new_node( "/", NULL );
	if( !parent ){
		fprintf(stderr, TREE_ALLOC_ERROR);
		fclose( JSON_FILE );
		return -1;
	}
	char space[ MAX_SPACE ] = "";
	
	recursivily_build( parent, 0b0000 );
	traverse_tree( parent, space, 0 );

	fclose( JSON_FILE );
	free_memory();
	return 0;
}
