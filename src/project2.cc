/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "lexer.h"
#include "project2.h"
#include <algorithm>
#include <unordered_set>

using namespace std;

Token Project2::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type) {
        syntax_error();
    }
    return t;
}

void Project2::syntax_error() {
    cout<< "SYNTAX ERROR !!!!!&%!!\n";
    exit(1);
}

TokenType Project2::ttype() {
    return lexer.peek(1).token_type;
}

// read grammar
void Project2::ReadGrammar() {
    parseRuleList();
    expect(HASH);
}

void Project2::parseRuleList(){
    parseRule();
    if (firstOfRuleList()) {
        parseRuleList();
    }
}

bool Project2::firstOfRuleList() {
    return ttype() == ID;
}

void Project2::addSymbol(string s) {
    this->symbList.push_back(s);
}

void Project2::parseRule() {
    string ruleName = expect(ID).lexeme;
    
    this->nonTerms.insert(ruleName);
    addSymbol(ruleName);
    
    expect(ARROW);
    vector<vector<string>> rhsList = parseRHS();
    
    for (vector<string> rhs : rhsList) {
        for (string sym : rhs) {
            addSymbol(sym);
        }
    }
    
    expect(STAR);
    if (data.find(ruleName) != data.end()) {
        data[ruleName].insert(data[ruleName].end(), rhsList.begin(), rhsList.end());
    } else {
        this->data[ruleName] = rhsList;
    }
}

vector<vector<string>> Project2::parseRHS() {
    vector<vector<string>> rhsList = vector<vector<string>>{};
    vector<string> idList = parseIdList();
    rhsList.push_back(idList);
    if (ttype() == OR) {
        expect(OR);
        vector<vector<string>> moreRhs = parseRHS();
        rhsList.insert(rhsList.end(), moreRhs.begin(), moreRhs.end());
    }
    return rhsList;
}

vector<string> Project2::parseIdList() {
    vector<string> idList = vector<string>{};

    if (ttype() == ID) {
            string id = expect(ID).lexeme;
            idList.push_back(id);
            vector<string> moreIds = parseIdList();
            idList.insert(idList.end(), moreIds.begin(), moreIds.end());
    } else if (ttype() == OR || ttype() == STAR) {
        return idList;
    } else {
        syntax_error();
    }
    return idList;
}

void Project2::postProcessSymbols() {
    for (string s: symbList) {
        if (nonTerms.count(s) == 1) {
            auto it = find(this->nonTermList.begin(), this->nonTermList.end(), s);
            if (it == this->nonTermList.end()) {
                this->nonTermList.push_back(s);
            }
        } else {
            auto it = find(this->termList.begin(), this->termList.end(), s);
            if (it == this->termList.end()) {
                this->termList.push_back(s);
            }
        }
    }
}


/* 
 * Task 1: 
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
*/
void Project2::Task1() {
    for (string term : this->termList) {
        cout << term << " ";
    }

    for (string nonTerm : this->nonTermList) {
        cout << nonTerm << " ";
    }
    cout << endl;
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
    
    ps:
        iterate thru all rules
        changed = false
            if rule has a rhs.size() == 0, add to nullable. changed = true
            if rule has a rhs where all symbs are nullable, add to nullable. changed=true
        repeat until changed == false
        print nullable
 */
void Project2::Task2() {
    unordered_set<string> nullable{};
    bool changed = true;
    while (changed){
        changed = false;

        for (auto pair : this->data) {
            string ntName = pair.first;
            // if current NT is already nullable, skip it
            if (nullable.count(ntName) > 0) {
                continue;
            }

            // for each of this NT's RHSs, check if either:
            //   it is the empty string (size == 0)
            //   all of its symbols are nullable 
            for (int i=0; i<pair.second.size(); i++) {
                vector<string> rhs = pair.second.at(i);
                
                // check for empty string
                if (rhs.size() == 0) {
                    nullable.insert(ntName);
                    changed = true;
                    break;
                // check if all symbs are nullable
                } else {
                    bool allNullSymbs = true;
                    for (string sym : rhs) {
                        if (nullable.count(sym) == 0) {
                            allNullSymbs = false;
                            break;
                        }
                    }
                    if (allNullSymbs) {
                        nullable.insert(ntName);
                        changed = true;
                        break;
                    }
                }        
            }
        }
    }

    cout << "Nullable = {";

    unordered_set<string> history{};
    bool firstElem = true;
    for (string sym : symbList) {
        if (nullable.count(sym) && history.count(sym) == 0) {
            if (firstElem) {
                cout << " " << sym << " ";
                firstElem = false;
            } else {
                cout << ", " << sym << " ";
            }
            history.insert(sym);
        }
    }
    cout << "}" << endl;
}

unordered_set<string> Project2::calcFirstOfRhs(vector<string> rhs){
    unordered_set<string> first{};
    for (string eachSym : rhs) {
        auto it = find(this->termList.begin(), this->termList.end(), eachSym);
        if (it != this->termList.end()) {
            first.insert(eachSym);
        } else {
        }
    }
    return first;
}

unordered_set<string> Project2::calcFirstSet(string symb){
    vector<vector<string>> rhsList = this->data[symb];
    unordered_set<string> first{};
    for (vector<string> eachRhs : rhsList) {
        unordered_set<string> eachFirstSet = calcFirstOfRhs(eachRhs);
        first.insert(eachFirstSet.begin(), eachFirstSet.end());
    }   
    return first;
}

// Task 3: FIRST sets
void Project2::Task3() {
    unordered_set<string> history{};

    for (string symb : symbList) {
        if (nonTerms.count(symb) && history.count(symb) == 0) {
            history.insert(symb);
            unordered_set<string> firstSet = calcFirstSet(symb);
        
            cout << "FIRST(" << symb << ") = {";
            bool firstElem = true;
            for (string s: firstSet) {
                if (firstElem) {
                    cout << " " << s;
                    firstElem = false;
                } else {
                    cout << ", " << s;
                }
            }
            cout << " }" << endl;
        }        
    }
}

// Task 4: FOLLOW sets
void Project2::Task4()
{
}

// Task 5: left factoring
void Project2::Task5()
{
}

// Task 6: eliminate left recursion
void Project2::Task6()
{
}
    
void Project2::run(int task){
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file
    postProcessSymbols();
    switch (task) {
        case 1: Task1();
            break;

        case 2: Task2();
            break;

        case 3: Task3();
            break;

        case 4: Task4();
            break;

        case 5: Task5();
            break;
        
        case 6: Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }

    return;
}

int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    Project2 p2 = Project2();
    p2.run(task);
    return 0;

}

