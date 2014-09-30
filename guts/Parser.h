#ifndef _PARSER_H_
#define _PARSER_H_

#include <cmath>
#include <vector>
#include <map>
#include <complex>
#include <unordered_map>
#include <random>
#include <iostream>

//Constants to 20 decimal places
#define PARSER_SPI "3.14159265358979323846"
#define PARSER_SE "2.71828182845904523536"
#define PARSER_SPHI "1.61803398874989484820"
#define PARSER_PI 3.14159265358979323846
#define PARSER_E 2.71828182845904523536
#define PARSER_PHI 1.61803398874989484820
#define PARSER_SI "1i"
#define MAX_FCT_LEN 4
#define MIN_FCT_LEN 2
#define NUM_ARITH_OPS 5
#define NUM_FCT_LEN 3
#define FCT_LEN_OFFSET (-3)
#define MAX_CONST_LEN 3
using namespace std;

typedef std::complex<double> cx;

namespace parser {
template <typename T> std::string toString(T t); //To convert numeric types to strings
template <> std::string toString(cx c); //Specialized version for cx
std::string toStringRounded(cx c); //Specialized version for cx, which rounds to 2 decimal places
cx stringToCx(string s);

void init();
//Functions and pointers to call actual operators
cx add(cx a, cx b);
cx sub(cx a, cx b);
cx mul(cx a, cx b);
cx div(cx a, cx b);

cx pow(cx base, cx a); //only raises e^a right now
cx log(cx base, cx a); //Only does nat log of a right now

cx sin(cx a, cx b = 0);
cx cos(cx a, cx b = 0);
cx tan(cx a, cx b = 0);
cx asin(cx a, cx b = 0);
cx acos(cx a, cx b = 0);
cx atan(cx a, cx b = 0);
cx sqrt(cx a, cx b = 0);
cx abs(cx a, cx b = 0);
cx rpm(cx a = 0, cx b = 0);


cx (* const padd)(cx, cx) = parser::add;
cx (* const psub)(cx, cx) = parser::sub;
cx (* const pmul)(cx, cx) = parser::mul;
cx (* const pdiv)(cx, cx) = parser::div;

cx (* const ppow)(cx, cx) = parser::pow;
cx (* const plog)(cx, cx) = parser::log;

cx (* const psin)(cx, cx) = parser::sin;
cx (* const pcos)(cx, cx) = parser::cos;
cx (* const ptan)(cx, cx) = parser::tan;
cx (* const pasin)(cx, cx) = parser::asin;
cx (* const pacos)(cx, cx) = parser::acos;
cx (* const patan)(cx, cx) = parser::atan;
cx (* const psqrt)(cx, cx) = parser::sqrt;
cx (* const pabs)(cx, cx) = parser::abs;
cx (* const prpm)(cx, cx) = parser::rpm;



class Node {
	friend class Tree;
private:

	Node* m_parent;
	Node* m_left;
	Node* m_right;
	string m_val; //should always be char (for std. fcts, operators) or Variable
	Node(Node *parent = NULL, string val = "");
public:
	~Node();
	string toString();
};

enum DelimInd { ADD, SUB, MUL, DIV, POW, FT2, FT3, FT4, VAR, CON};
class Tree { //Hold decomposed expr
private:
	static string delim[];
	static bool initd;
	static std::unordered_map<std::string, Tree*> variables;

	string toString(Node *n, string path);
	string m_fct;
	int parse(Node *root);
	cx value(Node *root);
	static std::unordered_map<std::string, cx (* const)(cx,cx)> parseops;
	Node *m_root;
	void init();
	int isToken(string s);
	int checkParenthesis(string s);
public:
	Tree(string expr="");
	~Tree();

	string toString();
	bool isInitd();
	int parse();
	cx eval();
	string getFct();
	string getVarFct(string var);

	string getVar(string var);
	void setVar(string var, string a);
	void setVar(string var, cx a);
	cx evalVar(string var);
};

}
#endif
