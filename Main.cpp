#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iomanip>

const std::vector<std::string> keywords = { "int", "float", "return", "function", "while", "if", "fi", "put" , "boolean" , "get" , "true" , "false" };
const std::vector<std::string> seps = { "[", "]", "(", ")", "{", "}", ";", ":"  , "$$" , ",", "$" };
const std::vector<std::string> ops = { "=", "!", "<", ">", "-", "+", "*", "/" , "<=", ">=", "!=", "-=", "+=", "==", "*=" , "/=" };

bool file = false;
int line = 1;
std::vector<Character> table;
int Mem_address = 5000;

class Reader {
private:
	std::string token, lexeme, type;
public:
	std::string getToken() { return this->token; }
	std::string getLexeme() { return this->lexeme; }
	std::string getType() { return this->type; }

	void setLexeme(std::string s) { this->lexeme = s; }
	void setType(std::string s) { this->type = s; }
	void setToken(std::string s) { this->token = s; }
};

class Character {
private:
	Reader sym;
	std::string address;
public:
	Reader getSym() { return this->sym; }
	std::string getAddress() { return this->address; }

	void setSym(Reader s) { this->sym = s; }
	void setAddress(int s) { this->address = std::to_string(s); }

	Character(Reader sym, int address) {
		setSym(sym);
		setAddress(address);
	}
};

class instr {
private:
	std::string op, oprnd;
	int address;
public:
	std::string getOp() {
		return this->op;
	}

	std::string getOprnd() {
		return this->oprnd;
	}

	int getAddress() {
		return this->address;
	}

	void setOprnd(std::string s) {
		this->oprnd = s;
	}

	void setOp(std::string s) {
		this->op = s;
	}

	void setAddress(int s) {
		this->address = s;
	}

	instr(int i, std::string s, std::string t) {
		setOprnd(t);
		setOp(s);
		setAddress(i);
	}
};

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
	out << "Identifier\t" << "MemoryLocation\tType\n";
	std::vector<Character>::iterator i = table.begin();
	while (i != table.end()) {
		out << i->getSym().getLexeme() << "\t\t" << i->getAddress() << "\t\t" << i->getSym().getToken() << "\n";
		i++;
	}
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

void arithmetic_Error() {
	std::cerr << "Error: Illegal arithmetic operation in line " << line << ".\n";
	exit(2);
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



			if (latest.getToken() != "fileend" && display) out << std::left << std::setw(10) << "Token:" << latest.getToken() << "\t:\t" << std::setw(10) << "Lexeme:" << latest.getLexeme() << "\n";
			return latest;
		}
		else if (!isspace(c) && state != "comments" && done != 1) { lexeme.push_back(c); }
		else if (c == '\n') { line += 1; }

	}

}

void Lexeme_Check(std::ofstream& out, std::ifstream& source, std::string lexeme) {
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != lexeme) {
		Syntax_Error(latest, out, lexeme);
	}
}

void Syntax_Error(Reader latest, std::ofstream& out, std::string expected) {
	std::cerr << "Syntax Error: Expected " << expected << " on line " << line << "\n";
	std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
	exit(1);
}

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

int main(int argc, const char* argv[]) {
	char c;
	if (argv[1] == nullptr) {
		std::cerr << "No Input File Detected\n";
		std::cin >> c;
		return 2;
	}
	if (argv[2] == nullptr) {
		std::cerr << "No Output File Detected\n";
		std::cin >> c;
		return 2;
	}
	std::ifstream source(argv[1]);
	std::ofstream out(argv[2]);
	if (!out.is_open()) {
		std::cout << "Output file failed to open\n";
		std::cin >> c;
		return 2;
	}

	if (!source.is_open()) {
		std::cout << "Input file failed to open\n";
		std::cin >> c;
		return 2;
	}

	Rat20F(out, source);

	print_Symbols(out);
	print_instr(out);

	out.close();
	source.close();
	return 0;
}

void Empty(std::ofstream& out, std::ifstream& source) {
}

Reader Primary(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Primary> ::= <Identifier> <Primary>' | <Integer> | ( <Expression> ) | <Real> | true | false\n";

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
		else
			return Lexer_call(out, source);
	}

	else if (latest.getToken() == "int") {
		arithmetic_Table.push_back(latest);
		general_instr("PUSHI", latest.getLexeme());
		return Lexer_call(out, source);
	}

	else if (latest.getToken() == "real")
		return Lexer_call(out, source);

	else if (latest.getLexeme() == "true" || latest.getLexeme() == "false") {
		if (latest.getLexeme() == "true")
			general_instr("PUSHM", "1");
		else
			general_instr("PUSHM", "0");
		return Lexer_call(out, source);
	}
	else
		Syntax_Error(latest, out, "identifier or int or ( or real or true or false");
}

Reader Primary_prime(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Primary>' ::= ( <IDs> ) | <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "(") {
		return latest;
	}
	else
		latest = IDs(out, source, Lexer_call(out, source), false, "");
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	else
		return Lexer_call(out, source);
}


Reader Factor(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Factor> ::= - <Primary> | <Primary>\n";

	if (latest.getLexeme() == "-")
		return Primary(out, source, Lexer_call(out, source));
	else
		return Primary(out, source, latest);
}

Reader Term(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Term> ::= <Factor> <Term>'\n";
	latest = Factor(out, source, latest);
	return Term_Prime(out, source, latest);
}

Reader Term_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Term>' ::= * <Factor> <Term>' | / <Factor> <Term>' | <Empty>\n";
	Reader save = latest;
	if (latest.getLexeme() == "*" || latest.getLexeme() == "/") {
		latest = Factor(out, source, Lexer_call(out, source));
		arithmetic_Check();
		if (save.getLexeme() == "*")
			general_instr("MUL", "null");
		else if (save.getLexeme() == "/")
			general_instr("DIV", "null");
		return Term_Prime(out, source, latest);
	}
	else
		return latest;
}

Reader Expression(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Expression> ::= <Term> <Expression>'\n";
	latest = Term(out, source, latest);
	return Expression_Prime(out, source, latest);
}

Reader Expression_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Expression>' ::= + <Term> <Expression>' | - <Term> <Expression>' | <Empty>\n";
	std::string save = latest.getLexeme();
	if (latest.getLexeme() == "+" || latest.getLexeme() == "-") {
		latest = Term(out, source, Lexer_call(out, source));
		arithmetic_Check();
		if (save == "+") general_instr("ADD", "null");
		else if (save == "-") general_instr("SUB", "null");
		return Expression_Prime(out, source, latest);
	}
	else
		return latest;
}

void Relop(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Relop> ::= == | != | > | < | <= | =>\n";

	if (latest.getLexeme() == "==" || latest.getLexeme() == "!=" || latest.getLexeme() == ">" || latest.getLexeme() == "<" || latest.getLexeme() == "<=" || latest.getLexeme() == "=>")
		return;
}

Reader Condition(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Condition> ::= <Expression> <Relop> <Expression>\n";
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
	if (display)
		out << "\t<While> ::= while ( <Condition> ) <Statement>\n";
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
	if (display)
		out << "\t<Scan> ::= get ( <IDs> );\n";

	Lexeme_Check(out, source, "(");
	Reader latest = Lexer_call(out, source);
	if (display)
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	if (latest.getToken() != "identifier")
		Syntax_Error(latest, out, "an identifier");
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
	if (display)
		out << "\t<Print> ::= put ( <Expression> );\n";

	Lexeme_Check(out, source, "(");
	Reader latest = Expression(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	general_instr("STDOUT", "null");
	Lexeme_Check(out, source, ";");

}

Reader IDs_Cont(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ",")
		return IDs(out, source, Lexer_call(out, source));
	else
		return latest;
}

Reader IDs_Cont(std::ofstream& out, std::ifstream& source, bool make, std::string a) {
	if (display)
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ",")
		return IDs(out, source, Lexer_call(out, source), make, a);
	else
		return latest;
}

Reader IDs(std::ofstream& out, std::ifstream& source, Reader latest, bool make, std::string a) {
	if (display)
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	if (latest.getToken() != "identifier")
		Syntax_Error(latest, out, "an identifier");
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
	if (display)
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	if (latest.getToken() != "identifier")
		Syntax_Error(latest, out, "an identifier");
	return IDs_Cont(out, source);
}

void Body(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Body> ::= { < Statement List> }\n";
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
	if (display)
		out << "\t<Qualifier> ::= int | boolean | real\n";
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real")
		return;
	else
		Syntax_Error(latest, out, "int, boolean, or real");
}

void Parameter(std::ofstream& out, std::ifstream& source, Reader a) {
	if (display)
		out << "\t<Parameter> ::= <IDs> <Qualifier>\n";
	Reader latest = IDs(out, source, a);
	Qualifier(out, source, latest);
}

Reader Decla(std::ofstream& out, std::ifstream& source, Reader a) {
	if (display)
		out << "\t<Parameter> ::= <Qualifier> <IDs>\n";
	Qualifier(out, source, a);
	Reader latest = Lexer_call(out, source);

	return IDs(out, source, latest, true, a.getLexeme());
}

Reader Parameter_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Parameter List> ::= <Parameter> <Parameter List>'\n";
	Parameter(out, source, latest);
	return Parameter_List_Cont(out, source);
}

Reader Parameter_List_Cont(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Parameter List>\' ::= ,  <Parameter List>  |  <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ",")
		return Parameter_List(out, source, Lexer_call(out, source));
	else
		return latest;
}

Reader Declare_List_Cont(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Declaration List>\' ::= <Declaration List>  |  <Empty>\n";
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real")
		return Declare_List(out, source, latest);
	else
		return latest;
}

Reader Declare_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Declaration List> ::= <Declaration> ; <Declaration List>\'\n";
	Reader l = Decla(out, source, latest);
	if (l.getLexeme() != ";")
		Syntax_Error(l, out, ";");
	return Declare_List_Cont(out, source, Lexer_call(out, source));
}

Reader State_List_Cont(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Statement List>\' ::= <Statement List>  |  <Empty>\n";
	if (latest.getLexeme() == "{") {
		return State_List(out, source, latest);
	}
	else if (latest.getToken() == "identifier")
		return State_List(out, source, latest);
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
	else
		return latest;
}

Reader State_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Statement List> ::= <Statement> <Statement List>\'\n";
	Statement(out, source, latest);
	return State_List_Cont(out, source, Lexer_call(out, source));
}

Reader OPL(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Opt Parameter List> ::= <Parameter List> | <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getToken() != "identifier") {
		return latest;
	}
	return Parameter_List(out, source, latest);

}

Reader ODL(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Opt Declaration List> ::= <Declaration List> | <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real") {
		return Declare_List(out, source, latest);
	}
	else
		return latest;

}

void Func(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Function> ::= function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>\n";

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
	if (display)
		out << "\t<Function Definitions>' ::= <Function Definitions> | <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else
		return Func_Def(out, source);
}

Reader Func_Def(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Function Definitions> ::= <Function> <Function Definitions>'\n";
	Func(out, source);
	return Func_Def_Cont(out, source);
}
Reader OFD(std::ofstream& out, std::ifstream& source) {

	if (display)
		out << "\t<Opt Function Definitions> ::= <Function Definitions> | <Empty>\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else
		return Func_Def(out, source);
}

void Statement(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Statement ::= <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>\n";

	if (latest.getLexeme() == "{") {
		Compound(out, source);
	}
	else if (latest.getToken() == "identifier")
		Assign(out, source, latest);
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
	else
		Syntax_Error(latest, out, "{ or identifier or if or return or put or get or while");

}

void Compound(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Compound> ::= { <Statement List> }\n";
	Reader latest = State_List(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != "}")
		Syntax_Error(latest, out, "}");
}

void Assign(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<Assign> :: <Identifier> = <Expression>; \n";

	std::string save = latest.getLexeme();

	Lexeme_Check(out, source, "=");

	latest = Expression(out, source, Lexer_call(out, source));
	general_instr("POPM", get_address(save));
	if (latest.getLexeme() != ";")
		Syntax_Error(latest, out, ";");
}

void If(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<If> ::= if ( <Condition> ) <Statement> <If>'\n";
	Lexeme_Check(out, source, "(");
	Reader latest = Condition(out, source);
	if (latest.getLexeme() != ")")
		Syntax_Error(latest, out, ")");

	Statement(out, source, Lexer_call(out, source));
	int addr = instr_address;
	back_patch(addr);
	If_Prime(out, source, Lexer_call(out, source));
	general_instr("LABEL", "");

}

void If_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (display)
		out << "\t<If>' ::= fi | else <Statement> fi\n";

	if (latest.getLexeme() == "fi")
		return;

	else if (latest.getLexeme() == "else") {
		Statement(out, source, Lexer_call(out, source));
		Lexeme_Check(out, source, "fi");
	}
}

void Return(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Return> ::= return <Return>'\n";
	Return_Prime(out, source);
	return;
}

void Return_Prime(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Return>' ::= ; | <Expression>;\n";
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ";")
		return;
	else
		latest = Expression(out, source, latest);

	if (latest.getLexeme() != ";") {
		Syntax_Error(latest, out, ";");
	}

}

void Rat20F(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Rat20F> ::= $$ <Opt Declaration List> <Statement List> $$\n";

	Lexeme_Check(out, source, "$$");

	Reader latest = ODL(out, source); 

	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "$$")
		Syntax_Error(latest, out, "$$");

}