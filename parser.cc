//
// Created by: Jordan Pladgeman 
// ASU ID: 1210269642
// Date: 01/24/2017
// CSE 340 - Assignment 1
// Bazzi M-W-F (9:40 am) 
// Description:    Overall goal for this assingment was to add to/change provide code
// //              to suit our task of lexical analysis on the input string. 
// //              For a high level review this required:
// //              * the creation of data structures to model regular expression graphs in
// //              parse expression (CHAR,UNDERSCORE (empty string),OR, STAR (Kleen Star))
// //              * taking that graph of a particular grammar rule and it's identifier to
// //              iterate through an input string char by char and matching or returning
// //    		   if no match or the char is a space which means that str is fully consumed
// //              * out of all the grammar rules or regular expression graphs finding the
// //              longest possible match, if nothing matches there was a syntax error in 
// //              the input string. 
// //              * Print out the longest matching prefix/'s till the input string is fully
// //              matched

/*
* Copyright (C) Rida Bazzi, 2017
*
* Do not share this file with anyone
*
* Do not post this file or derivatives of
* of this file online
*
*/
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <algorithm>
#include <set>
#include "parser.h"



void Parser::syntax_error()
{
	cout << "Syntax Error\n";
	exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected_type)
		syntax_error();
	return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
	Token t = lexer.GetToken();
	lexer.UngetToken(t);
	return t;
}

// Parsing

void Parser::parse_input()
{
	//input -> tokens_section INPUT_TEXT
	parse_tokens_section();
	Token t = expect(INPUT_TEXT); // Saves the input_text here (t.lexeme)
	GetToken(t);				  // Finding longest matching prefix entry point
}

void Parser::parse_tokens_section()
{
	// tokens_section -> token_list HASH
	parse_token_list();
	expect(HASH);   // A hash represents the end of the regular expression rule/'s
}

void Parser::parse_token_list()
{
	// token_list -> token
	// token_list -> token COMMA token_list

	reg_vector.push_back(parse_token()); // Add the regExp's to the reg_vector
	Token t = peek();                    // used to check if the grammar section has more rules
	if (t.token_type == COMMA)          // <- comma will mean more rules
	{
		// token_list -> token COMMA token_list
		expect(COMMA);
		parse_token_list();
	}
	else if (t.token_type == HASH)  // At this point the program will return to parse_input()
	{
		// token_list -> token
	}
	else
	{
		syntax_error();
	}

}

struct regExp* Parser::parse_token() 
{
	// token -> ID expr
	regExp* exp = new regExp(); // Creating memory for a new regExp
	exp->token_name = (expect(ID)).lexeme; // saves token_name
	exp->reg = parse_expr(); // saves reg 
	return exp; // return the regExp to be added to the vector

}

struct REG* Parser::parse_expr()
{
	// expr -> CHAR
	// expr -> LPAREN expr RPAREN DOT LPAREN expr RPAREN
	// expr -> LPAREN expr RPAREN OR LPAREN expr RPAREN
	// expr -> LPAREN expr RPAREN STAR
	// expr -> UNDERSCORE

	Token t = lexer.GetToken();
	if (t.token_type == CHAR) {

		// expr -> CHAR
		return makeNode(t); // Trying to keep this code dry with the makeNode()
	}
	else if (t.token_type == UNDERSCORE) {

		// expr -> UNDERSCORE
		return makeNode(t);
	}
	else if (t.token_type == LPAREN) {
		// expr -> LPAREN expr RPAREN DOT LPAREN expr RPAREN
		// expr -> LPAREN expr RPAREN OR LPAREN expr RPAREN
		// expr -> LPAREN expr RPAREN STAR
		REG* r1 = parse_expr();
		expect(RPAREN);
		Token t2 = lexer.GetToken();
		if (t2.token_type == DOT || t2.token_type == OR)
		{
			REG* r2 = new REG();
			expect(LPAREN);
			r2 = parse_expr();
			expect(RPAREN);
			REG* bothREG = new REG;

			if (t2.token_type == DOT)
			{

				r1->accepting->first_neighbor = r2->starting;
				r1->accepting->first_label = '_';
				bothREG->starting = r1->starting;
				bothREG->accepting = r2->accepting;
				return bothREG;
			}
			else // OR
			{
				bothREG->starting = new REG_node();
				bothREG->accepting = new REG_node();


				bothREG->starting->first_label = '_';
				bothREG->starting->second_label = '_';

				r1->accepting->first_label = '_';
				r2->accepting->first_label = '_';


				r1->accepting->first_neighbor = bothREG->accepting;
				r2->accepting->first_neighbor = bothREG->accepting;

				bothREG->starting->first_neighbor = r1->starting;
				bothREG->starting->second_neighbor = r2->starting;
				return bothREG;

			}
		}

		else if (t2.token_type == STAR)
		{
			// Create REG
			REG* r2 = new REG();
			// Create two new nodes
			REG_node* node_LHS = new REG_node;
			REG_node* node_RHS = new REG_node;


			// Link the new starting node3 to the old starting node1
			node_LHS->first_neighbor = r1->starting;
			node_LHS->first_label = '_';

			node_LHS->second_neighbor = node_RHS; // Create a path from new starting node3 to accepting node4
			node_LHS->second_label = '_';     // update label

			r1->accepting->first_neighbor = r1->starting;   // Link up old accepting node2 to new accepting node4
			r1->accepting->first_label = '_';		// upadate label

													// Create a path from node2 back to node1
			r1->accepting->second_neighbor = node_RHS;
			r1->accepting->second_label = '_';            // update label

			r2->starting = node_LHS;     // Update starting and accepting nodes
			r2->accepting = node_RHS;

			return r2;
		}

	}
	else
	{
		syntax_error();
	}
}

struct REG* Parser::makeNode(Token t) {
	// Create two nodes
	REG* r1 = new REG();
	REG_node* node_LHS = new REG_node();
	REG_node* node_RHS = new REG_node();

	// Establish the link between node_LHS and node_RHS
	node_LHS->first_neighbor = node_RHS;
	if (t.token_type == UNDERSCORE)
	{
		node_LHS->first_label = '_';

	}
	else
	{
		//Set the label for node_LHS with the char that gets passed in
		node_LHS->first_label = *t.lexeme.c_str();

	}
	// Set REG starting and accepting nodes
	r1->starting = node_LHS;
	r1->accepting = node_RHS;

	// Return the REG
	return r1;
}

void Parser::ParseProgram()
{
	parse_input();
//	expect(END_OF_FILE);
//    GetToken(t);

}

void Parser::GetToken(Token t) // Small summary of this function in header file
{

	int p = 0; 
	// Trims spaces
	size_t startpos = t.lexeme.find_first_not_of(" ");
	size_t endpos = t.lexeme.find_last_not_of(" ");
	t.lexeme = t.lexeme.substr(startpos, endpos - startpos + 1);
	// This is the loop that makes sure the whole string was ran through
	while (p <= (t.lexeme.length() - 1))
	{
		// This vector contains the longest matching prefix for each reg
		vector <matched*> matched_vec;

		// This loop address each reg in the reg_vector
		for (int i = 0; i <= reg_vector.size() - 1; i++)
		{
			// Save the longest matching prefix for this reg in tmp
			struct lastMatched* tmp = match(reg_vector[i]->reg, t.lexeme, p);
			// as long as there is at least a one char match
			if (tmp!=NULL)
			{
				// add the token_name and lexeme string to the matched vector
				matched* match = new matched();
				match->token_name = reg_vector.at(i)->token_name;
				match->matched = tmp;
				matched_vec.push_back(match);
			}
		}

		if (matched_vec.size() != 0) // if there was at least one match
		{
			int length = -1;
			int index = -1;
			// Go through matched_vec and find longest matched
			for (int x = 0; x <= matched_vec.size() - 1; x++)
			{
				if (matched_vec.at(x)->matched->lexeme.length() > length)
				{
					length = matched_vec.at(x)->matched->lexeme.length();
					index = x;
				}
			}
			
			p = matched_vec[index]->matched->str_position+1;
			
			
			cout << matched_vec[index]->token_name << " , \"" << matched_vec[index]->matched->lexeme << "\"" << endl;
		}
		else
		{
		    cout << "ERROR" << endl;
			exit(1);
		}
	}
}

struct lastMatched* Parser::match(REG* reg, string s, int p)
{
    vector<REG_node*> matched_vec;
	vector <REG_node*> initial_vec;
	initial_vec.push_back(reg->starting);
	// Find all the nodes that can be reached by consuming one a '_'
	locate_underscore_route(reg->starting, &initial_vec);
	// skip spaces in front of strings
	char c = s.at(p);
	while(isspace(c))
	{
		p++;
		c = s.at(p);
	}
	int start_pos = p; // this is just in case you skip spaces to know where the substr is
    int last_pos;
	while(p <= s.length()-1)
	{
        vector <REG_node*> tmp_vector;
		// Fill the tmp_vector with the nodes that are reachable by consuming the char c
        for(int i = 0; i< initial_vec.size(); i++) 
        {
     		match_one_char(initial_vec.at(i), &tmp_vector, c);	
        }  
		// Fill the tmp vector with nodes that are reachable by '_'
        for(int i = 0; i< tmp_vector.size(); i++) 
        {
         	locate_underscore_route(tmp_vector.at(i), &tmp_vector);	
        }     
       // If there were reachable nodes
	    if(tmp_vector.empty())  
	    {
			// check matched_vec size
	   		if(matched_vec.size() > 0)
        	{  // break the while loop
		 		break;
       		 }
        return NULL; // or return null
	    } 
	    for(int i=0; i < tmp_vector.size(); i++)
	    { // check for accepting node in the tmp_vector
	    	if(tmp_vector.at(i) == reg->accepting)
	    	{
	        	last_pos = p; 
	        	matched_vec = tmp_vector;  
	    	}	
	    } 
		// update initial_vec
	    initial_vec = tmp_vector;
	    // increase position
		p++;
		// update char c with the new char at the new position
        c = s.at(p); 
		// If char c is a space, check matched_vec then break the loop
        if(c==' ')
        {
            if(matched_vec.size() > 0)
            {  
				break;   
            }
            return NULL; 
        }
    	
    }
	// Add the substring, and substring position to a new created last
	// and return (if there was matches)
    if(matched_vec.size() > 0)
    {
    	lastMatched* last = new lastMatched;
    	last->lexeme = s.substr(start_pos, last_pos-start_pos+1);
    	last->str_position = last_pos; 
    	return last;      
    }
    return NULL; 
}

void Parser::locate_underscore_route(REG_node* node, vector <REG_node*>* vec)
{
	// if the first label - '_' add to vector recall function with that node
	if (node->first_label == '_')
	{
		if (find(vec->begin(), vec->end(), node->first_neighbor) == vec->end()) // Find function explained bellow in match_one_char()
		{
			vec->push_back(node->first_neighbor);
			locate_underscore_route(node->first_neighbor, vec); // Recursively call till the node has been added already
		}
	}
	// if the second nieghbor's label - '_' add to vector recall function with that node
	if (node->second_label == '_')
	{
		if (find(vec->begin(), vec->end(), node->second_neighbor) == vec->end())
		{
			vec->push_back(node->second_neighbor);
			locate_underscore_route(node->second_neighbor, vec);
		}
	}

}
// Matching one char that gets passed in to the node's first or second nieghbor
void Parser::match_one_char(REG_node* node, vector <REG_node*>* vec, char c)
{
 	if (node->first_label == c) // If the char matches go ahead and see if the node's
	{                           // first nieghbor is in the vec already
    	if (find(vec->begin(), vec->end(), node->first_neighbor) == vec->end()) // If the function returns vec->end()
		{                                                                      // this means it went through the vec and
			vec->push_back(node->first_neighbor);		                       // go to the end without finding the node's
		}                                                                      // first neighbor
 	} 
 	if (node->second_label == c) // Same as above only with the node's second neighbor
 	{
    	if (find(vec->begin(), vec->end(), node->second_neighbor) == vec->end())
		{
			vec->push_back(node->second_neighbor);		
		} 
 	} 	
}

int main()
{
	Parser parser;
	parser.ParseProgram();
}
