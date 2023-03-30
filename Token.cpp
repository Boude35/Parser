

#include "Token.hpp"

#include <fstream>
#include <iomanip>

using namespace std;

// the promised global for string equivalents of TokenType enumeration
string TokStr[]=
{ "ERROR", "EOF_TOK", "NUM_INT", "NUM_REAL", "ADDOP", "MULOP", "ID", "RELOP", "ASSIGNOP", "LPAREN", "RPAREN",  "SEMICOLON",  "LBRACK", "RBRACK", "COMMA", "AND", "OR", "INTEGER", "FLOAT", "WHILE", "IF", "THEN", "ELSE", "VOID", "BEGIN", "END"};

// This is a "list" of the keywords. Note that they are in the same order
//   as found on the TokenType enumaration. 
static string reserved[]={"int" , "float", "while", "if", "then", "else", "void", "begin", "end" };

//Array that will determine the type of the token depending the position of the final state.
//the index of each type correspond the finl state in the FDA
TokenType Final[]=
{ ERROR, ID, NUM_INT, ERROR, NUM_REAL, ADDOP, MULOP, RELOP, RELOP, ASSIGNOP, LPAREN, RPAREN, ERROR, AND,ERROR, OR, SEMICOLON, LBRACK, RBRACK, COMMA};


ostream&
Token::print(ostream& os) const
{
  os
     << "{ Type:"   << left << setw(10) << TokStr[_type] 
     << " Value:"   << left << setw(10) << _value
     << " Line Number:" << _line_num
     << " }";
  return os;
}


 //create the array that will contain the states table
static int **DFA = NULL;
//Variable that will keep the track of the line number. Starting at 1
int line = 1;
void Token::get(istream &is)
{
	//if the array is not yet inicializated
	if(DFA == NULL)
	{
		//add 20 rows to it, as 20 states exist
		DFA = new int*[20];
		//create 259 columns and inicializate them to -1(ERROR)
		for(int state = 0; state <= 19;state++)
		{
			DFA[state] = new int[256];
			for(int ascii=0; ascii<256; ascii++)
				DFA[state][ascii] = -1;
		}
	}
	
	//If we are in state 0 and receive alpha value, got to state 1
	for(char ascii='a'; ascii<'z'; ascii++)
		DFA[0][(int)ascii] = 1;
	for(char ascii='A'; ascii<'Z'; ascii++)
		DFA[0][(int) ascii] = 1;
	//If we are in state 0 and receive a digit, got to state 2
	for(char ascii='0'; ascii<='9'; ascii++)
		DFA[0][(int) ascii] = 2;
	
	//if we receive any special character go to the corresponding value in the DFA
	DFA[0]['+'] = 5;
	DFA[0]['-'] = 5;
	DFA[0]['*'] = 6;
	DFA[0]['/'] = 6;
	DFA[0]['<'] = 7;
	DFA[0]['>'] = 7;
	DFA[0]['='] = 9;
	DFA[0]['('] = 10;
	DFA[0][')'] = 10;
	DFA[0]['&'] = 12;
	DFA[0]['|'] = 14;
	DFA[0][';'] = 16;
	DFA[0]['['] = 17;
	DFA[0][']'] = 18;
	DFA[0][','] = 19;
  
  	//If we are in state 1 and receive an alphanumeric value, go back to state 1
	for(char ch='0'; ch<='9'; ch++)
		DFA[1][(int) ch] = 1;
	for(char ch='A'; ch<'Z'; ch++)
		DFA[1][(int) ch] = 1;
	for(char ch='a'; ch<'z'; ch++)
		DFA[1][(int) ch] = 1;
	
	//If we are in state 2 and receive an numeric value, go back to state 2
	for(char ch='0'; ch<='9'; ch++)
		DFA[2][(int) ch] = 2; 
	//If we are in state 2 and receive an dot character, go to state 3
	DFA[2]['.'] = 3;
	
	//If we are in state 3 and receive an numeric character, go to state 4
	for(char ch='0'; ch<='9'; ch++)
		DFA[3][(int) ch] = 4;
	
	//If we are in state 4 and receive an numeric value, go back to state 4
	for(char ch='0'; ch<='9'; ch++)
		DFA[4][(int) ch] = 4;
	
	//if we are in states 9 or 7 and receive an assign character go to state 8
	DFA[9]['='] = 8;
	DFA[7]['='] = 8;
	
	//go to state 13 or 15 if & or | char is received respectively and we are in states 12 or 14 respectively
	DFA[12]['&'] = 13;
	DFA[14]['|'] = 15;
	
  	//inicialize with default values
  	_value = "";
	_type =  ERROR;
  	
  	//inicialize curr with state 0 and prev with -1(error)
  	int prev = -1;
  	int curr = 0;
  	
  	//inicialize char and get the first one
  	char ch;
  	ch = is.get();
  	
  	//Will store comments in case there are comments	
  	string comment;
  	
  	//if there is comments or white spaces
  	while(isspace(ch) || ch == '#')
  	{
  		//if there is a white space
	  	if(isspace(ch))
	  	{
	  		//and the white space drops a line
	  		if(ch == '\n')
	  			line++;//add one to the line number
	  	}
	  	if(ch == '#')//if there is a comment
	  	{
	  		getline(is, comment);//read in the comment and store it into comment (it is not going to be used)
	  		line++;//add one line number
	  	
	  	}
	  	ch = is.get();//get the next character
	  	if(is.eof())//if this character is the end of the line
	  	{
	  		_type = EOF_TOK;//change its type to EOF_TOK that will exit the program
	  		return;
	  	}
	  }
	  is.unget();//unget the last reade character
	
  	while(curr != -1)//while the is no error
	{	
		ch = is.get();//get the character
		prev = curr;//make the current state the previous one 
		curr=DFA[curr][(int) ch];//read the next state
		if(curr != -1)//if this state is not an error
		{
			_value+=ch;//keep adding character to the value of this token
		}
		_line_num = line;//after having completed the value change the line number of the token
	}

	is.unget(); //if it is not the same token, unget the last character(first character of the next token)
	_type = Final[prev];//change type using the array created at the start of the file and get the type based on the final state of the previous token
	if(_type == Final[1])//if the token is an ID
	{
		//change its type according to its value for the given keywords
		if(_value=="void")
			_type=VOID;
		else if(_value=="int")
			_type=INTEGER;
		else if(_value=="float")
			_type=FLOAT;
		else if(_value=="begin")
			_type=BEGIN;
		else if(_value=="end")
			_type=END;
		else if(_value=="if")
			_type=IF;
		else if(_value=="then")
			_type=THEN;
		else if(_value=="else")
			_type=ELSE;
		else if(_value=="while")
			_type=WHILE;
	}
  	
  

}
