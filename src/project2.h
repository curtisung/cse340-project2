#ifndef PROJECT2_H
#define PROJECT2_H

#include "lexer.h"
#include <unordered_map>
#include <unordered_set>
using namespace std;


typedef enum {EMPTY, TERMINAL, NON_TERMINAL} SymbolType; 

struct Symbol {
    string str;
    SymbolType symbType;

    Symbol() : str(""), symbType(EMPTY) {};
    Symbol(const string& str, SymbolType sType) : str(str), symbType(sType) {};
};

class Project2 {
    public:
        LexicalAnalyzer lexer;
        Token expect(TokenType expected_type);
        void syntax_error();
        TokenType ttype();

        unordered_map<string, vector<vector<string>>> data;
        vector<string> symbList;
        unordered_set<string> nonTerms;
        vector<string> nonTermList;
        vector<string> termList;

        void postProcessSymbols();
        void parseRuleList();
        void parseRule();
        vector<vector<string>> parseRHS();
        vector<string> parseIdList();
        bool firstOfRuleList();
        void addSymbol(string s);

        vector<string> orderedListFromSet(unordered_set<string>);
        unordered_map<string, unordered_set<string>> calcAllFirstSets();


        void ReadGrammar();
        void run(int task);
        void Task1();
        void Task2();
        void Task3();
        void Task4();
        void Task5();
        void Task6();
};

#endif