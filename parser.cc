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
	Token t = expect(INPUT_TEXT);
	GetToken(t);
}

void Parser::parse_tokens_section()
{
	// tokens_section -> token_list HASH
	parse_token_list();
	expect(HASH);
}

void Parser::parse_token_list()
{
	// token_list -> token
	// token_list -> token COMMA token_list

	regList.push_back(parse_token());
	Token t = peek();
	if (t.token_type == COMMA)
	{
		// token_list -> token COMMA token_list
		expect(COMMA);
		parse_token_list();
	}
	else if (t.token_type == HASH)
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
	regExp* exp = new regExp();
	exp->token_name = (expect(ID)).lexeme; // token_name
	exp->reg = parse_expr();
	return exp;

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
		return makeNode(t);
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

				r1->accepting->firstNeighbor = r2->starting;
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


				r1->accepting->firstNeighbor = bothREG->accepting;
				r2->accepting->firstNeighbor = bothREG->accepting;

				bothREG->starting->firstNeighbor = r1->starting;
				bothREG->starting->secondNeighbor = r2->starting;
				return bothREG;

			}
		}

		else if (t2.token_type == STAR)
		{
			// Create REG
			REG* r2 = new REG();
			// Create two new nodes
			REG_node* starting = new REG_node;
			REG_node* accepting = new REG_node;


			// Link the new starting node3 to the old starting node1
			starting->firstNeighbor = r1->starting;
			starting->first_label = '_';

			starting->secondNeighbor = accepting; // Create a path from new starting node3 to accepting node4
			starting->second_label = '_';     // update label

			r1->accepting->firstNeighbor = r1->starting;   // Link up old accepting node2 to new accepting node4
			r1->accepting->first_label = '_';		// upadate label

													// Create a path from node2 back to node1
			r1->accepting->secondNeighbor = accepting;
			r1->accepting->second_label = '_';            // update label

			r2->starting = starting;     // Update starting and accepting nodes
			r2->accepting = accepting;

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
	REG_node* node1 = new REG_node();
	REG_node* node2 = new REG_node();

	// Establish the link between node1 and node2
	node1->firstNeighbor = node2;
	if (t.token_type == UNDERSCORE)
	{
		node1->first_label = '_';

	}
	else
	{
		//Set the label for node 1 with the char that gets passed in
		node1->first_label = *t.lexeme.c_str();

	}
	// Set REG starting and accepting nodes
	r1->starting = node1;
	r1->accepting = node2;

	// Return the REG
	return r1;
}

void Parser::ParseProgram()
{
	parse_input();
//	expect(END_OF_FILE);
//    GetToken(t);

}

void Parser::GetToken(Token t)
{

	int p = 0;
	size_t startpos = t.lexeme.find_first_not_of(" ");
	size_t endpos = t.lexeme.find_last_not_of(" ");
	t.lexeme = t.lexeme.substr(startpos, endpos - startpos + 1);
	// This is the loop that makes sure the whole string was ran through
	while (p <= (t.lexeme.length() - 1))
	{
		// Removes white space from the input text

		// This vector contains the longest matching prefix for each reg-ex
		vector <matched*> matchedVec;

		// This loop address each reg-ex in the regList vector individually 
		for (int i = 0; i <= regList.size() - 1; i++)
		{
			// Save the longest matching prefix for this reg-ex in tmp
			struct lastMatched* tmp = match(regList[i]->reg, t.lexeme, p);
			// as long as there is at least a one char match
			if (tmp!=NULL)
			{
				// add the token_name and lexeme string to the matched vector
				matched* match = new matched();
				match->token_name = regList.at(i)->token_name;
				match->matched = tmp;
				matchedVec.push_back(match);
			}
		}
		
		
		if (matchedVec.size() != 0)
		{
			// if there was at least one match
			int length = 0;
			int index = 0;
			// Find longest matched
			for (int x = 0; x <= matchedVec.size() - 1; x++)
			{
				if (matchedVec.at(x)->matched->lexeme.length() > length)
				{
					length = matchedVec.at(x)->matched->lexeme.length();
					index = x;
				}
			}
			
			p = matchedVec[index]->matched->str_position+1;
			
			
			cout << matchedVec[index]->token_name << " , \"" << matchedVec[index]->matched->lexeme << "\"" << endl;
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
    vector<REG_node*> matched_vector;
	vector <REG_node*> intial_vector;
	intial_vector.push_back(reg->starting);
	// Find all the nodes that can be reached by consuming one a '_'
	find_open_paths(reg->starting, &intial_vector);
	
	// skip spaces in front of strings
	char c = s.at(p);
	while(isspace(c))
	{
		p++;
		c = s.at(p);
	}
	int start_pos = p; // this is just in case you skip spaces to know where your at
    int last_pos;
	while(p <= s.length()-1)
	{
	   
	
       vector <REG_node*> tmp_vector;
       for(int i = 0; i< intial_vector.size(); i++) 
       {
         find_consume_paths(intial_vector.at(i), &tmp_vector, c);	
       }  
       for(int i = 0; i< tmp_vector.size(); i++) 
       {
         find_open_paths(tmp_vector.at(i), &tmp_vector);	
       }
       
       
       
	   if(tmp_vector.empty())  
	   {
	    if(matched_vector.size() > 0)
        {
         lastMatched* last = new lastMatched;
         last->lexeme = s.substr(start_pos, last_pos-start_pos+1);
         last->str_position = last_pos; 
         return last;      
        }
        return NULL;
	   } 
	   for(int i=0; i < tmp_vector.size(); i++)
	   {
	    if(tmp_vector.at(i) == reg->accepting)
	    {
	        last_pos = p; 
	        matched_vector = tmp_vector;  
	    }
	
	   } 
	    intial_vector = tmp_vector;
	    
	    p++;
        c = s[p]; 

        if(c==' ')
        {
            if(matched_vector.size() > 0)
            {
                lastMatched* last = new lastMatched;
                last->lexeme = s.substr(start_pos, last_pos-start_pos+1);
                last->str_position = last_pos; 
                return last;      
            }
            return NULL; 
        }
    	
    }
    if(matched_vector.size() > 0)
    {
      lastMatched* last = new lastMatched;
      last->lexeme = s.substr(start_pos, last_pos-start_pos+1);
      last->str_position = last_pos; 
      return last;      
     }
     return NULL;
}




void Parser::find_open_paths(REG_node* node, vector <REG_node*>* vec)
{
	// if the first label - '_' add to vector recall function with that node
	if (node->first_label == '_')
	{
		if (find(vec->begin(), vec->end(), node->firstNeighbor) == vec->end())
		{
			vec->push_back(node->firstNeighbor);
			find_open_paths(node->firstNeighbor, vec);
		}
	}
	// if the second nieghbor's label - '_' add to vector recall function with that node
	if (node->second_label == '_')
	{
		if (find(vec->begin(), vec->end(), node->secondNeighbor) == vec->end())
		{
			vec->push_back(node->secondNeighbor);
			find_open_paths(node->secondNeighbor, vec);
		}
	}

}

void Parser::find_consume_paths(REG_node* node, vector <REG_node*>* vec, char c)
{
 if (node->first_label == c)
 {
    if (find(vec->begin(), vec->end(), node->firstNeighbor) == vec->end())
		{
			vec->push_back(node->firstNeighbor);		
		}
 
 } 
 if (node->second_label == c)
 {
    if (find(vec->begin(), vec->end(), node->secondNeighbor) == vec->end())
		{
			vec->push_back(node->secondNeighbor);		
		}
 
 } 
 
	
}

int main()
{
	Parser parser;
	parser.ParseProgram();
}
