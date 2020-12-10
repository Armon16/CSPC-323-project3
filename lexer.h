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
const std::vector<std::string> seps = { "[", "]", "(", ")", "{", "}", ";", ":"  , "$$" , ",", "$" };
const std::vector<std::string> ops = { "=", "!", "<", ">", "-", "+", "*", "/" , "<=", ">=", "!=", "-=", "+=", "==", "*=" , "/=" };



void Undefined_Identifier() {
	std::cerr << "Error: undeclared Identifier used in line " << line << ".\n";
	exit(2);
}

void Error_redeclare() {
	std::cerr << "Error: Identifier illegally redeclared in line " << line << ".\n";
	exit(2);
}


std::string get_address(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym) { return i->getAddress(); }
		i++;
	}
	Undefined_Identifier();

}

Reader retrieve_system(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym) { return i->getSym(); }
		i++;
	}
	Undefined_Identifier();
}

Reader retrieve_system_by_address(std::string sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getAddress() == sym) { return i->getSym(); }
		i++;
	}
	Undefined_Identifier();
}

void make_Sym(Reader sym) {
	std::vector<Character>::iterator i;
	i = table.begin();
	while (!table.empty() && i != table.end()) {
		if (i->getSym().getLexeme() == sym.getLexeme()) { Error_redeclare(); }
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
	out << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
	out << "Assembly Code:\n";
}


int instr_address = 1;
std::vector<instr> Instr_table;

void general_instr(std::string op, std::string oprnd){
	Instr_table.push_back(instr(instr_address, op, oprnd));
	instr_address++;
};

void print_instr(std::ofstream& out) {
	out << "\n";
	std::vector<instr>::iterator i = Instr_table.begin();
	while (i != Instr_table.end()) {
		out << i->getAddress() << "\t" << i->getOp() << "\t" << i->getOprnd() << "\n";
		i++;
	}
}

std::vector<Reader> arithmetic_Table;

void arithmetic_Error() {
	std::cerr << "Error: illegal arithmetic op on line: " << line << ".\n";
	exit(2);
}

void arithmetic_Check() {
	if (arithmetic_Table.empty()) { arithmetic_Error(); }
	std::string save = arithmetic_Table.back().getLexeme();
	if (arithmetic_Table.back().getLexeme() == "identifier") {
		arithmetic_Table.pop_back();
		if (retrieve_system_by_address(save).getType() == "boolean" || retrieve_system_by_address(arithmetic_Table.back().getLexeme()).getType() == "boolean")
			arithmetic_Error();
		if (retrieve_system_by_address(save).getType() != retrieve_system_by_address(arithmetic_Table.back().getLexeme()).getType())
			arithmetic_Error();
	}
	else if (arithmetic_Table.back().getLexeme() == "int") {
		if (retrieve_system_by_address(save).getType() != "int")
			arithmetic_Error();
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
	Instr_table[addr].setOprnd(std::to_string(jump_addr));
}

bool FSM(std::string& state, char input, std::string& lexeme) {
	std::string c;
	if (state != "comments")
		c.push_back(input);
	if (state == "start") {
		if (isalpha(input)) { state = "identifier"; }
		else if (isdigit(input)) { state = "int"; }
		else if (std::find(ops.begin(), ops.end(), c) != ops.end()) { state = "operator"; }
		else if (std::find(seps.begin(), seps.end(), c) != seps.end()) { state = "separator"; }
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
	else if (state == "operator" && std::find(ops.begin(), ops.end(), lexeme + c) == ops.end()) {
		if (lexeme == "<" || lexeme == "=" || lexeme == "!" || lexeme == "<" || lexeme == ">" || lexeme == "-" || lexeme == "+" || lexeme == "*" || lexeme == "/") {}
		return true;
	}
	else if (state == "separator" && std::find(seps.begin(), seps.end(), lexeme + c) == seps.end()) {
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



			if (latest.getToken() != "fileend" && file) out << std::left << std::setw(10) << "Token:" << latest.getToken() << "\t:\t" << std::setw(10) << "Lexeme:" << latest.getLexeme() << "\n";
			return latest;
		}
		else if (!isspace(c) && state != "comments" && done != 1) { lexeme.push_back(c); }
		else if (c == '\n') { line += 1; }

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