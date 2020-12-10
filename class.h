#ifndef CLASS_H
#define CLASS_H
#include <iostream>
#include <vector>

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

std::vector<Character> table;

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
#endif