#ifndef MACHINE_H
#define MACHINE_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include "class.h"
#include "lexer.h"

void Rat20F(std::ofstream& out, std::ifstream& source);
Reader OFD(std::ofstream& out, std::ifstream& source);
Reader IDs(std::ofstream& out, std::ifstream& source, Reader latest);
Reader IDs(std::ofstream& out, std::ifstream& source, Reader latest, bool make, std::string a);
Reader IDs_Cont(std::ofstream& out, std::ifstream& source);
Reader IDs_Cont(std::ofstream& out, std::ifstream& source, bool make, std::string a);
void Compound(std::ofstream& out, std::ifstream& source);
void Statement(std::ofstream& out, std::ifstream& source, Reader latest);
Reader State_List(std::ofstream& out, std::ifstream& source, Reader latest);
Reader Expression(std::ofstream& out, std::ifstream& source, Reader latest);
Reader Parameter_List(std::ofstream& out, std::ifstream& source, Reader latest);
Reader Declare_List(std::ofstream& out, std::ifstream& source, Reader latest);
Reader Func_Def(std::ofstream& out, std::ifstream& source);
void Assign(std::ofstream& out, std::ifstream& source, Reader latest);
void If(std::ofstream& out, std::ifstream& source);
void Return(std::ofstream& out, std::ifstream& source);
void If_Prime(std::ofstream& out, std::ifstream& source, Reader latest);
void Return_Prime(std::ofstream& out, std::ifstream& source);

Reader Primary_prime(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Primary>' ::= ( <IDs> ) | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "(") {
		return latest;
	}
	else{
		latest = IDs(out, source, Lexer_call(out, source), false, "");
	}
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	else{
		return Lexer_call(out, source);
	}
}

Reader Primary(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Primary> ::= <Identifier> <Primary>' | <Integer> | ( <Expression> ) | <Real> | true | false\n";
	}

	if (latest.getToken() == "identifier") {
		arithmetic_Table.push_back(latest);
		general_instr("PUSHM", get_address(latest.getLexeme()));
		return Primary_prime(out, source);
	}

	else if (latest.getLexeme() == "(") {
		latest = Expression(out, source, Lexer_call(out, source));
		if (latest.getLexeme() != ")") {
			Syntax_Error(latest, out, ")");
		}
		else{
			return Lexer_call(out, source);
		}
	}

	else if (latest.getToken() == "int") {
		arithmetic_Table.push_back(latest);
		general_instr("PUSHI", latest.getLexeme());
		return Lexer_call(out, source);
	}

	else if (latest.getToken() == "real"){
		return Lexer_call(out, source);
	}

	else if (latest.getLexeme() == "true" || latest.getLexeme() == "false") {
		if (latest.getLexeme() == "true"){
			general_instr("PUSHM", "1");
		}
		else{
			general_instr("PUSHM", "0");
		}
		return Lexer_call(out, source);
	}
	else{
		Syntax_Error(latest, out, "identifier or int or ( or real or true or false");
	}
}

Reader Factor(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Factor> ::= - <Primary> | <Primary>\n";
	}

	if (latest.getLexeme() == "-"){
		return Primary(out, source, Lexer_call(out, source));
	}
	else{
		return Primary(out, source, latest);
	}
}

Reader Term_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Term>' ::= * <Factor> <Term>' | / <Factor> <Term>' | <Empty>\n";
	}
	Reader save = latest;
	if (latest.getLexeme() == "*" || latest.getLexeme() == "/") {
		latest = Factor(out, source, Lexer_call(out, source));
		arithmetic_Check();
		if (save.getLexeme() == "*"){
			general_instr("MUL", "null");
		}
		else if (save.getLexeme() == "/"){
			general_instr("DIV", "null");
		}
		return Term_Prime(out, source, latest);
	}
	else{
		return latest;
	}
}
Reader Term(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Term> ::= <Factor> <Term>'\n";
	}
	latest = Factor(out, source, latest);
	return Term_Prime(out, source, latest);
}

Reader Expression_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Expression>' ::= + <Term> <Expression>' | - <Term> <Expression>' | <Empty>\n";
	}
	std::string save = latest.getLexeme();
	if (latest.getLexeme() == "+" || latest.getLexeme() == "-") {
		latest = Term(out, source, Lexer_call(out, source));
		arithmetic_Check();
		if (save == "+") {
			general_instr("ADD", "null");
		}
		else if (save == "-"){
			 general_instr("SUB", "null");
		}
		return Expression_Prime(out, source, latest);
	}
	else{
		return latest;
	}
}

Reader Expression(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Expression> ::= <Term> <Expression>'\n";
	}
	latest = Term(out, source, latest);
	return Expression_Prime(out, source, latest);
}

void Relop(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Relop> ::= == | != | > | < | <= | =>\n";
	}

	if (latest.getLexeme() == "==" || latest.getLexeme() == "!=" || latest.getLexeme() == ">" || latest.getLexeme() == "<" || latest.getLexeme() == "<=" || latest.getLexeme() == "=>")
		return;
}

Reader Condition(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Condition> ::= <Expression> <Relop> <Expression>\n";
	}
	Reader latest = Expression(out, source, Lexer_call(out, source));
	Relop(out, source, latest);
	Reader save = Expression(out, source, Lexer_call(out, source));
	if (latest.getLexeme() == "<") {
		general_instr("LES", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	else if (latest.getLexeme() == "<=") {
		general_instr("LES", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	else if (latest.getLexeme() == ">") {
		general_instr("GRT", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	else if (latest.getLexeme() == ">=") {
		general_instr("GRT", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	else if (latest.getLexeme() == "==") {
		general_instr("EQU", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	else if (latest.getLexeme() == "!=") {
		general_instr("NEQ", "null");
		push_jumpStack(instr_address - 1);
		general_instr("JUMPZ", "null");
	}

	return save;
}

void While(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<While> ::= while ( <Condition> ) <Statement>\n";
	}
	std::string addr = std::to_string(instr_address);
	general_instr("LABEL", "null");
	Lexeme_Check(out, source, "(");
	Reader latest = Condition(out, source);
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	Statement(out, source, Lexer_call(out, source));
	general_instr("JUMP", addr);
	back_patch(instr_address);
	return;
}

void Scan(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Scan> ::= get ( <IDs> );\n";
	}

	Lexeme_Check(out, source, "(");
	Reader latest = Lexer_call(out, source);
	if (file){
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	}
	if (latest.getToken() != "identifier"){
		Syntax_Error(latest, out, "an identifier");
	}
	while (latest.getToken() == "identifier" || latest.getLexeme() == ",") {
		if (latest.getToken() == "identifier") {
			general_instr("STDIN", "null");
			general_instr("POPM", get_address(latest.getLexeme()));
		}
		latest = Lexer_call(out, source);
	}

	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}

	Lexeme_Check(out, source, ";");
}

void Print(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Print> ::= put ( <Expression> );\n";
	}

	Lexeme_Check(out, source, "(");
	Reader latest = Expression(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	general_instr("STDOUT", "null");
	Lexeme_Check(out, source, ";");

}

Reader IDs_Cont(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return IDs(out, source, Lexer_call(out, source));
	}
	else{
		return latest;
	}
}

Reader IDs_Cont(std::ofstream& out, std::ifstream& source, bool make, std::string a) {
	if (file){
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return IDs(out, source, Lexer_call(out, source), make, a);
	}
	else{
		return latest;
	}
}

Reader IDs(std::ofstream& out, std::ifstream& source, Reader latest, bool make, std::string a) {
	if (file){
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	}
	if (latest.getToken() != "identifier"){
		Syntax_Error(latest, out, "an identifier");
	}
	else if (make) {
		std::vector<std::string>::const_iterator word;
		word = std::find(keywords.begin(), keywords.end(), a);
		latest.setType(*word);
		make_Sym(latest);
	}
	else {
		arithmetic_Table.push_back(latest);
		general_instr("PUSHM", get_address(latest.getLexeme()));
	}
	return IDs_Cont(out, source, make, a);
}
Reader IDs(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	}
	if (latest.getToken() != "identifier"){
		Syntax_Error(latest, out, "an identifier");
	}
	return IDs_Cont(out, source);
}

void Body(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Body> ::= { < Statement List> }\n";
	}
	if (latest.getLexeme() != "{") {
		Syntax_Error(latest, out, "{");
	}
	latest = Lexer_call(out, source);
	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "}") {
		Syntax_Error(latest, out, "}");
	}
}

void Qualifier(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Qualifier> ::= int | boolean | real\n";
	}
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real"){
		return;
	}
	else{
		Syntax_Error(latest, out, "int, boolean, or real");
	}
}

void Parameter(std::ofstream& out, std::ifstream& source, Reader a) {
	if (file){
		out << "\t<Parameter> ::= <IDs> <Qualifier>\n";
	}
	Reader latest = IDs(out, source, a);
	Qualifier(out, source, latest);
}

Reader Decla(std::ofstream& out, std::ifstream& source, Reader a) {
	if (file){
		out << "\t<Parameter> ::= <Qualifier> <IDs>\n";
	}
	Qualifier(out, source, a);
	Reader latest = Lexer_call(out, source);

	return IDs(out, source, latest, true, a.getLexeme());
}

Reader Parameter_List_Cont(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Parameter List>\' ::= ,  <Parameter List>  |  <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return Parameter_List(out, source, Lexer_call(out, source));
	}
	else{
		return latest;
	}
}

Reader Parameter_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Parameter List> ::= <Parameter> <Parameter List>'\n";
	}
	Parameter(out, source, latest);
	return Parameter_List_Cont(out, source);
}

Reader Declare_List_Cont(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Declaration List>\' ::= <Declaration List>  |  <Empty>\n";
	}
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real"){
		return Declare_List(out, source, latest);
	}
	else{
		return latest;
	}
}

Reader Declare_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Declaration List> ::= <Declaration> ; <Declaration List>\'\n";
	}
	Reader l = Decla(out, source, latest);
	if (l.getLexeme() != ";"){
		Syntax_Error(l, out, ";");
	}
	return Declare_List_Cont(out, source, Lexer_call(out, source));
}

Reader State_List_Cont(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement List>\' ::= <Statement List>  |  <Empty>\n";
	}
	if (latest.getLexeme() == "{") {
		return State_List(out, source, latest);
	}
	else if (latest.getToken() == "identifier"){
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "if") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "return") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "put") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "get") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "while") {
		return State_List(out, source, latest);
	}
	else{
		return latest;
	}
}

Reader State_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement List> ::= <Statement> <Statement List>\'\n";
	}
	Statement(out, source, latest);
	return State_List_Cont(out, source, Lexer_call(out, source));
}

Reader OPL(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Opt Parameter List> ::= <Parameter List> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getToken() != "identifier") {
		return latest;
	}
	return Parameter_List(out, source, latest);

}

Reader ODL(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Opt Declaration List> ::= <Declaration List> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real") {
		return Declare_List(out, source, latest);
	}
	else{
		return latest;
	}

}

void Func(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Function> ::= function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>\n";
	}

	Reader latest = Lexer_call(out, source);
	latest = IDs(out, source, latest);

	if (latest.getLexeme() != "(") {
		Syntax_Error(latest, out, "(");
	}
	latest = OPL(out, source);
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	latest = ODL(out, source);
	Body(out, source, latest);

}

Reader Func_Def_Cont(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Function Definitions>' ::= <Function Definitions> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else{
		return Func_Def(out, source);
	}
}

Reader Func_Def(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Function Definitions> ::= <Function> <Function Definitions>'\n";
	}
	Func(out, source);
	return Func_Def_Cont(out, source);
}
Reader OFD(std::ofstream& out, std::ifstream& source) {

	if (file){
		out << "\t<Opt Function Definitions> ::= <Function Definitions> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else{
		return Func_Def(out, source);
	}
}

void Statement(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement ::= <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>\n";
	}

	if (latest.getLexeme() == "{") {
		Compound(out, source);
	}
	else if (latest.getToken() == "identifier"){
		Assign(out, source, latest);
	}
	else if (latest.getLexeme() == "if") {
		If(out, source);
	}
	else if (latest.getLexeme() == "return") {
		Return(out, source);
	}
	else if (latest.getLexeme() == "put") {
		Print(out, source);

	}
	else if (latest.getLexeme() == "get") {
		Scan(out, source);
	}
	else if (latest.getLexeme() == "while") {
		While(out, source);
	}
	else{
		Syntax_Error(latest, out, "{ or identifier or if or return or put or get or while");
	}

}

void Compound(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Compound> ::= { <Statement List> }\n";
	}
	Reader latest = State_List(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != "}"){
		Syntax_Error(latest, out, "}");
	}
}

void Assign(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Assign> :: <Identifier> = <Expression>; \n";
	}

	std::string save = latest.getLexeme();

	Lexeme_Check(out, source, "=");

	latest = Expression(out, source, Lexer_call(out, source));
	general_instr("POPM", get_address(save));
	if (latest.getLexeme() != ";"){
		Syntax_Error(latest, out, ";");
	}
}

void If(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<If> ::= if ( <Condition> ) <Statement> <If>'\n";
	}
	Lexeme_Check(out, source, "(");
	Reader latest = Condition(out, source);
	if (latest.getLexeme() != ")"){
		Syntax_Error(latest, out, ")");
	}

	Statement(out, source, Lexer_call(out, source));
	int addr = instr_address;
	back_patch(addr);
	If_Prime(out, source, Lexer_call(out, source));
	general_instr("LABEL", "");

}

void If_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<If>' ::= fi | else <Statement> fi\n";
	}

	if (latest.getLexeme() == "fi"){
		return;
	}

	else if (latest.getLexeme() == "else") {
		Statement(out, source, Lexer_call(out, source));
		Lexeme_Check(out, source, "fi");
	}
}

void Return(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Return> ::= return <Return>'\n";
	}
	Return_Prime(out, source);
	return;
}

void Return_Prime(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Return>' ::= ; | <Expression>;\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ";"){
		return;
	}
	else{
		latest = Expression(out, source, latest);
	}

	if (latest.getLexeme() != ";") {
		Syntax_Error(latest, out, ";");
	}

}

void Rat20F(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Rat20F> ::= $$ <Opt Declaration List> <Statement List> $$\n";
	}

	Lexeme_Check(out, source, "$$");

	Reader latest = ODL(out, source);

	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "$$"){
		Syntax_Error(latest, out, "$$");
	}

}
#endif