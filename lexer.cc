/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
                      "COMMA", "LPAREN", "RPAREN",
                        "DOT", "ID", "ERROR", "INPUT_TEXT", "HASH",
                      "CHAR", "UNDERSCORE", "OR", "SYMBOL", "STAR" // TODO: Add labels for new token types here (as string)
};



void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}


Token LexicalAnalyzer::ScanIdOrChar()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        int no = 0;

        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
            no++;
        }
        tmp.line_no = line_no;
        tmp.token_type = ID;
        if(no == 1)
            tmp.token_type = CHAR;

        if (!input.EndOfInput()) {

            input.UngetChar(c);
        }

    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}


Token LexicalAnalyzer::ScanInput()
{
    char c;
    input.GetChar(c);
    string lexeme = "";
    if (c == '"') {
        tmp.lexeme = "";
        //lexeme += '"';
        Token symbol = ScanSymbol();
        while (symbol.token_type == SYMBOL) {
            lexeme += symbol.lexeme;
            symbol = ScanSymbol();
        }
        if (!input.EndOfInput()) {
            input.GetChar(c);
            if (c == '"') {
                //lexeme += c;
                tmp.lexeme += lexeme;
                tmp.token_type = INPUT_TEXT;
            }
            else{
                tmp.lexeme = "";
                tmp.token_type = ERROR;
            }

        }
        else{
            tmp.lexeme = "";
            tmp.token_type = ERROR;
        }

        tmp.line_no = line_no;

    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

Token LexicalAnalyzer::ScanSymbol()
{
    char c;
    input.GetChar(c);
    tmp.lexeme = "";
    if (isspace(c) || isalnum(c)) {
        //tmp.lexeme += c;
        while (!input.EndOfInput() && (isspace(c) || isalnum(c))) {
            line_no += (c == '\n');
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;

        tmp.token_type = SYMBOL;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}




// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '*':
            tmp.token_type = STAR;
            return tmp;

        case ',':
            tmp.token_type = COMMA;
            return tmp;

        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;

        //UNDERSCORE
        case '_':
            tmp.token_type = UNDERSCORE;
            return tmp;

        //HASH
        case '#':
            tmp.token_type = HASH;
            return tmp;
        //OR
        case '|':
            tmp.token_type = OR;
            return tmp;
        //INPUT
        case '"':
            input.UngetChar(c);
            return ScanInput();
        default:
            if (isdigit(c)) {
                //CHAR
                //input.UngetChar(c);
                //return ScanNumber();
                tmp.token_type = CHAR;
                tmp.lexeme = c;
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrChar();
            }
            else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

/*int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}*/
