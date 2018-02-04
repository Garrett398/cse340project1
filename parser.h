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
// REG_node(Regular Expression Graph_node) represents a physical circle from the REG graphs in project description
// Using first_label and second_labels is the mechanism for which we represent the
// lines between nodes such as a particular char or '_' which represent's empty string
struct REG_node
{
	struct REG_node * first_neighbor;
	char first_label;
	struct REG_node * second_neighbor;
	char second_label;
};
// REG (Regular Expression Graph)
// At the end of the recursive construction of the graphs in parse_expr() this REG struct will serve
// as a reference point to the particular regular expression grammar rule
struct REG
{
	struct REG_node * starting;
	struct REG_node * accepting;
};
// This struct is the container for a particular REG and it's identifier
struct regExp
{
	string token_name;
	REG* reg;
};
// Used in match function and in my getToken()
// keeps track of str position and longest matching
// lexeme
struct lastMatched
{
    string lexeme;
    int str_position;
};
// This the longest mathing substring (and position) with the additional
// field of of the REG for which matched it
struct matched
{
	string token_name;
	lastMatched *matched;
};

using namespace std;
class Parser {
public:
	vector <regExp*> reg_vector; // Holds regs and reg names (token_name)
private:
	LexicalAnalyzer lexer;

	void syntax_error(); // print's syntax_error
	Token expect(TokenType expected_type); // checks to see if the lexer's getToken returns the "expected type"
	Token peek(); // looks at a token via lexer's getToken(), but doesnt consume char via ungetToken()
	
	struct lastMatched* match(REG* reg, string s, int p); // Finds longest matched substr through searching '_'(current char c)'_'..
	void parse_input();			  // Calls parse_tokens_section(), gets INPUT_TEXT through expect function which calls lexer's getToken()
								  // then calls my input getToken()
	void parse_tokens_section();  // When parse_token_list returns this expects(HASH), which is the end of the grammar rules
	void parse_token_list();      // Adds the regExp's to the reg_vector vector
	struct regExp* parse_token(); // Here is where the current regExp is created and passed back to parse_token_list()
	struct REG* parse_expr(); // Changed the return type so I could get back the REG graph for each rule
	
	// Methods I created
	void GetToken(Token t); // This is where, after the match function is called, finds longest matched substr
	struct REG* makeNode(Token t); // There was redundent code in the parse_expr() so I made a function
	void locate_underscore_route(REG_node* node, vector <REG_node*>* vec); // Find all possible routes that have '_'
	void match_one_char(REG_node* node,vector <REG_node*>* vec, char c);   // Find all possible routes for 
																		   // the current char c in INPUT_TEXT
	
public:
	void ParseProgram(); // Entry point for this program
};

#endif

