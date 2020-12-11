#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include "class.h"

int Mem_address = 5000;
int line = 1;
bool file = false;
const std::vector<std::string> keywords = { "int", "float", "return", "function", "while", "if", "fi", "put" , "boolean" , "get" , "true" , "false" };
const std::vector<std::string> separator_state = { "[", "]", "(", ")", "{", "}", ";", ":"  , "$$" , ",", "$" };
const std::vector<std::string> operator_state = { "=", "!", "<", ">", "-", "+", "*", "/" , "<=", ">=", "!=", "-=", "+=", "==", "*=" , "/=" };

std::string get_address(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym) {
			return i->getAddress();
			}
		i++;
	}
	std::cerr << "Error: undeclared Identifier used in line " << line << ".\n";
	exit(2);

}

Reader retrieve_system(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym) {
			return i->getSym();
			}
		i++;
	}
	std::cerr << "Error: undeclared Identifier used in line " << line << ".\n";
	exit(2);
}

Reader retrieve_system_by_address(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getAddress() == sym) {
			return i->getSym();
			}
		i++;
	}
	std::cerr << "Error: undeclared Identifier used in line " << line << ".\n";
	exit(2);
}

void make_Sym(Reader sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym.getLexeme()) {
			std::cerr << "Error: Identifier illegally redeclared in line " << line << ".\n";
			exit(2);
			 }
		i++;
	}
	table.push_back(Character(sym, Mem_address));
	Mem_address++;
}

void print_Symbols(std::ofstream& out) {
	out << "Symbol Table:\n\n";
	out << "Identifier\t" << "MemoryLocation\t\t\tType\n";
	std::vector<Character>::iterator i = table.begin();
	while (i != table.end()) {
		out << "\t" << i->getSym().getLexeme() << "\t\t\t\t\t\t" << i->getAddress() << "\t\t\t\t\t" << i->getSym().getToken() << "\n";
		i++;
	}
}

int instr_address = 1;
std::vector<instr> token;

void general_instr(std::string op, std::string oprnd){
	token.push_back(instr(instr_address, op, oprnd));
	instr_address++;
};

void print_instr(std::ofstream& out) {
	out << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
	out << "Assembly Code:\n";
	std::vector<instr>::iterator i = token.begin();
	while (i != token.end()) {
		out << i->getAddress() << "\t" << i->getOp() << "\t" << i->getOprnd() << "\n";
		i++;
	}
}

std::vector<Reader> arithmetic_Table;

void arithmetic_Check() {
	if (arithmetic_Table.empty()) { 
		std::cerr << "Error: illegal arithmetic op on line: " << line << ".\n";
		exit(2);
		}

	std::string save = arithmetic_Table.back().getLexeme();
	if (arithmetic_Table.back().getLexeme() == "identifier") {
		arithmetic_Table.pop_back();
		if (retrieve_system_by_address(save).getType() == "boolean" || retrieve_system_by_address(arithmetic_Table.back().getLexeme()).getType() == "boolean"){
		std::cerr << "Error: illegal arithmetic op on line: " << line << ".\n";
		exit(2);
		}

		if (retrieve_system_by_address(save).getType() != retrieve_system_by_address(arithmetic_Table.back().getLexeme()).getType()){
			std::cerr << "Error: illegal arithmetic op on line: " << line << ".\n";
			exit(2);

		}
	}
	else if (arithmetic_Table.back().getLexeme() == "int") {
		if (retrieve_system_by_address(save).getType() != "int"){
			std::cerr << "Error: illegal arithmetic op on line: " << line << ".\n";
			exit(2);

		}
	}
}

std::vector<int> jumpStackPosition;

void push_jumpStack(int address) {
	jumpStackPosition.push_back(address);
}

int pop_jumpStack() {
	int save = jumpStackPosition.back();
	jumpStackPosition.pop_back();
	return save;
}

void back_patch(int jump_addr)
{
	int addr = pop_jumpStack();
	token[addr].setOprnd(std::to_string(jump_addr));
}

bool FSM(std::string& state, char input, std::string& lexeme) {
	std::string c;
	if (state != "comments"){
		c.push_back(input);
	}
	if (state == "start") {
		if (isalpha(input)) {
			state = "identifier";
			}
		else if (isdigit(input)) {
			state = "int";
			}
		else if (std::find(operator_state.begin(), operator_state.end(), c) != operator_state.end()) {
			state = "operator";
			}
		else if (std::find(separator_state.begin(), separator_state.end(), c) != separator_state.end()) {
			state = "separator";
			}
		else if (input == EOF) {
			state = "fileend";
			return true;
		}
	}

	else if (state == "identifier" && !isalnum(input) && input != '_') {
		return true;
	}
	else if (state == "int") {
		if (!isdigit(input)) {
			return true;
		}
	}

	else if (state != "comments" && lexeme == "/*") {
		state = "comments";
		lexeme = "";
	}
	else if (state == "operator" && std::find(operator_state.begin(), operator_state.end(), lexeme + c) == operator_state.end()) {
		if (lexeme == "<" || lexeme == "=" || lexeme == "!" || lexeme == "<" || lexeme == ">" || lexeme == "-" || lexeme == "+" || lexeme == "*" || lexeme == "/") {
		return true;
		}
	}
	else if (state == "separator" && std::find(separator_state.begin(), separator_state.end(), lexeme + c) == separator_state.end()) {
		return true;
	}
	return false;
}

Reader Lexer_call(std::ofstream& out, std::ifstream& source) {
	std::string state = "start", lexeme = "";
	int done = 0;
	char c;
	while (done != 1) {
		c = source.get();

		if (FSM(state, c, lexeme) == true) {
			done = 1;
			source.unget();
		}

		if (state != "comments" && lexeme == "/") {
			if ((source.get()) == '*') {
				state = "comments";
				lexeme = "";
				done = 0;
			}
			else source.unget();
		}

		else if (state == "comments" && c == '*' && (source.get()) == '/') {
			state = "start";
			lexeme = "";
			c = source.get();
		}

		if (done == 1) {
			Reader latest;
			if (state == "identifier" && std::find(keywords.begin(), keywords.end(), lexeme) != keywords.end()) {
				state = "keyword";

			}

			latest.setLexeme(lexeme);
			latest.setToken(state);

			if (latest.getToken() != "fileend" && file) {
				out << std::left << std::setw(10) << "Token:" << latest.getToken() << "\t:\t" << std::setw(10) << "Lexeme:" << latest.getLexeme() << "\n";
			}
			return latest;
		}
		else if (!isspace(c) && state != "comments" && done != 1) {
			lexeme.push_back(c);
			}
		else if (c == '\n') {
			line += 1;
			}

	}
}

void Syntax_Error(Reader latest, std::ofstream& out, std::string expected) {
	std::cerr << "Syntax Error: Expected " << expected << " on line " << line << "\n";
	std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
	exit(1);
}

void Lexeme_Check(std::ofstream& out, std::ifstream& source, std::string lexeme) {
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != lexeme) {
		Syntax_Error(latest, out, lexeme);
	}
}

#endif