/*
* Copyright (C) Rida Bazzi, 2017
*
* Do not share this file with anyone
*/
#ifndef __PARSER_H__
#define __PARSER_H__
#include "lexer.h"
#include <string>
#include <list>
using namespace std;

struct REG_node
{
	struct REG_node * firstNeighbor;
	char first_label;
	struct REG_node * secondNeighbor;
	char second_label;

};

struct REG
{
	struct REG_node * starting;
	struct REG_node * accepting;
};

struct regExp
{
	string token_name;
	REG* reg;
};

struct lastMatched
{
    string lexeme;
    int str_position;
};

struct matched
{
	string token_name;
	lastMatched *matched;

};
using namespace std;
class Parser {
public:
	vector <regExp*> regList;
	
	vector <REG_node*> reg_prime;
	vector <REG_node*> tmp;
private:
	LexicalAnalyzer lexer;

	void syntax_error();
	Token expect(TokenType expected_type);
	Token peek();
	void GetToken(Token t);

	struct lastMatched* match(REG* reg, string s, int p);
	void parse_input();
	//void parse_char_list();
	void parse_tokens_section();
	void parse_token_list();
	struct regExp* parse_token();
	struct REG* parse_expr();
	struct REG* makeNode(Token t);
	void find_open_paths(REG_node* node, vector <REG_node*>* vec);
	void find_consume_paths(REG_node* node,vector <REG_node*>* vec, char c);
	
public:
	void ParseProgram();
};

#endif

