/* 
	Author: Abhin A S
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEY_SIZE 250
#define MAX_VALUE_SIZE 250
#define INDEX_KEY_LEN 250
#define BUFFER_SIZE 500
#define MAX_SPACE 25

typedef enum _j_syntax{
	OPEN_C = '{',
	CLOSE_C = '}',
	OPEN_S = '[',
	CLOSE_S = ']',
	APPO = '"',
	SEMI = ':',
	COMMA = ','
}J_SYNTAX;

typedef struct next_children{
	struct _node *child;
	struct next_children* next;
}ChD;

typedef struct _node{
	char *key, *value;
	ChD *childrens;
}Node;

char BUFFER[BUFFER_SIZE];
int buffer_ptr = -1;
FILE* JSON_FILE = NULL;

bool push_char_buffer(const char c){
	if(buffer_ptr + 1 >= BUFFER_SIZE)
		return false;
	BUFFER[++buffer_ptr] = c;
}

bool push_buffer(const char* buf){
	int buff_len = strlen(buf);
	if(buffer_ptr + buff_len >= BUFFER_SIZE){
		fprintf(stderr, "buffering failed...\n");
		return false;
	}
	buff_len--;
	while(buff_len > -1){
		BUFFER[++buffer_ptr] = buf[buff_len--];
	}
	return true;
}
char _fgetc(){
	return (buffer_ptr > -1) ? BUFFER[buffer_ptr--]: fgetc(JSON_FILE);
}

Node *get_new_node(const char *key, const char *value){
	Node *nnode = (Node *) malloc(sizeof(Node));
	if(nnode == NULL){
		fprintf(stderr, "failed to create node");
		return NULL;
	}
	nnode->key = (char *) malloc(strlen(key) + 1);
	if(!nnode->key){
		fprintf(stderr, "Failed to create index or value");
		free(nnode);
		return NULL;
	}
	strcpy(nnode->key, key);
	if(value){
		nnode->value = (char *) malloc(strlen(value) + 1);
		strcpy(nnode->value, value);
	}
	nnode->childrens = NULL;
	return nnode;
}

bool flag_info(int __flag, int shift){
	return (__flag >> shift) & 1;
}
int turn_on_off(int __flag, int shift){
	__flag ^= (1 << shift);
	return __flag;
}
bool is_key_filled(int __flag){
	return flag_info(__flag, 2);
}
bool is_value_filled(int __flag){
	return flag_info(__flag, 1);
}
bool is_value_filling(int __flag){
	return !flag_info(__flag, 1) && flag_info(__flag, 0);
}
bool is_value_pair(int __flag){
	return flag_info(__flag, 2) && flag_info(__flag, 1) && !flag_info(__flag, 0);
}
bool is_filling_mode(int __flag){
	return flag_info(__flag, 0);
}
bool is_list(int __flag){
	return flag_info(__flag, 3);
}

bool is_json_syntax(char c){
	return (c == OPEN_C) || (c == CLOSE_C) || \
	(c == OPEN_S) || (c == APPO) || (c == SEMI) || \
	(c == CLOSE_S) || (c == COMMA);
}

void add_child_to_parent(Node* parent, Node *child){
	ChD *new_child = (ChD *) malloc(sizeof(ChD));
	if(new_child == NULL){
		fprintf(stderr, "failed to create child ptr\n");
		return;
	}
	new_child->next = NULL;
	new_child->child = child;
	if(parent->childrens == NULL){
		parent->childrens = new_child;
	}else{
		ChD* temp = parent->childrens;
		while(temp->next != NULL)
			temp = temp->next;
		temp->next = new_child;
	}
}

void traverse_tree(Node *root, char *space, int index){
	if(root){
		printf("%s => %s : %s\n", space, root->key, root->value);
		ChD* child = root->childrens;
		while(child){
			if(child->child){
				if(index + 5 < MAX_SPACE)
					strcat(space, "     ");
				traverse_tree(child->child, space, index + 5);
				space[index - 5] = '\0';
			}
			child = child->next;
		}
	}
}
void recursivily_build(Node *parent, int _flag){
	char key[MAX_KEY_SIZE], value[MAX_VALUE_SIZE];
	int list_index = 0;
	int k_index, v_index, c;
	k_index = v_index = 0;

	Node *node_to_push = NULL;

	while((c = _fgetc()) != EOF){
		if(c == CLOSE_C || c == CLOSE_S){
			if(c == CLOSE_S)
				c = COMMA;
			int temp = _fgetc();
			push_char_buffer(temp);
			if(temp != COMMA) // comma trigerring
				push_char_buffer(COMMA);
			push_char_buffer(EOF);
			if(is_key_filled(_flag) && is_value_filling(_flag)){
				_flag = turn_on_off(_flag, 1);
				_flag = turn_on_off(_flag, 0);
				value[v_index] = '\0';
			}
		}
		if(!is_json_syntax(c)){
			if(isspace(c)) continue;
			if(!is_filling_mode(_flag))
				_flag = turn_on_off(_flag, 0);
			if(!is_key_filled(_flag))
				key[k_index++] = c;
			else if(!is_value_filled(_flag)){
				value[v_index++] = c;
			}
		}
		else if(c == APPO){
			if(!is_filling_mode(_flag))
				_flag = turn_on_off(_flag, 0);
			else{
				if(!is_key_filled(_flag)){
					key[k_index] = '\0';
					_flag = turn_on_off(_flag, 2);
					_flag = turn_on_off(_flag, 0);	
				}else{
					value[v_index] = '\0';
					_flag = turn_on_off(_flag, 1);
					_flag = turn_on_off(_flag, 0);
				}
			}
		}
		else if(is_json_syntax(c)){
			if(c == OPEN_C){
				if(is_key_filled(_flag)){
					Node *new_node = get_new_node(key, NULL);
					recursivily_build(new_node, 0b0000);
					node_to_push = new_node;
				}
			}
			else if(c == OPEN_S){
				char index_to_key[MAX_KEY_SIZE];
				sprintf(index_to_key, "%d", list_index);
				Node *key_node = NULL;
				key_node = get_new_node(key, NULL);					
				push_char_buffer('"');
				push_buffer(index_to_key);
				recursivily_build(key_node, 0b1001);
				node_to_push = key_node;
			}
			else if(c == COMMA){
				if(is_filling_mode(_flag)){
					_flag = turn_on_off(_flag, 1);
					_flag = turn_on_off(_flag, 0);
					value[v_index] = '\0';
				}
				if(node_to_push){
					add_child_to_parent(parent, node_to_push);
					node_to_push = NULL;
					k_index = v_index = 0;
					_flag = (is_list(_flag)) ? 0b1001: 0b0000;
				}
				if(is_list(_flag)){
					list_index += 1;
					char index_to_key[MAX_KEY_SIZE];
					sprintf(index_to_key, "%d", list_index);
					push_char_buffer('"');
					push_buffer(index_to_key);
				}
			}
		}
		if(is_value_pair(_flag)){
			if(is_list(_flag) && c != COMMA)
				continue;
			Node* new_node = get_new_node(key, value);
			add_child_to_parent(parent, new_node);
			k_index = v_index = 0;
			_flag = (is_list(_flag)) ? 0b1001: 0b0000;
		}
		
	}
}

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "Provide json file\n");
		return -1;
	}
	const char *file = argv[1];
	JSON_FILE = fopen(file, "r");
	Node *parent = get_new_node("/", NULL);
	char space[MAX_SPACE];
	if(JSON_FILE == NULL){
		return -1;
	}else{
		recursivily_build(parent, 0b0000);
		traverse_tree(parent, space, 0);
		puts("\n*******************END*******************");
	}
	fclose(JSON_FILE);
	return 0;
}
