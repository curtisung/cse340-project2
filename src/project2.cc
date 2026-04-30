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


// Task 3: FIRST sets
void Project2::Task3() {
    unordered_map<string, unordered_set<string>> firstSets = calcAllFirstSets();

    unordered_set<string> history{};

    for (string sym : symbList) {
        if (history.count(sym)) {
            continue;
        }

        if (this->nonTerms.count(sym)) {
            // print First
            cout << "FIRST(" << sym << ") = {";
            bool firstElem = true;
            unordered_set<string> eachFirstSet = firstSets.at(sym);
            vector<string> eachFirstList = orderedListFromSet(eachFirstSet);
            
            for (string s: eachFirstList) {
                if (firstElem) {
                    cout << " " << s;
                    firstElem = false;
                } else {
                    cout << ", " << s;
                }
            }
            cout << " }" << endl;
        }
        history.insert(sym);
    }
}


vector<string> Project2::orderedListFromSet(unordered_set<string> set) {
    vector<string> result{};
    unordered_set<string> hist{};
    for (string sym : symbList) {
        if (hist.count(sym)){
            continue;
        }
        if (set.count(sym)) {
            result.push_back(sym);   
        }
        hist.insert(sym);
    }
    return result;
}

bool stringInVector(const string& str, vector<string> v) {
    auto it = find(v.begin(), v.end(), str);
    return it != v.end();
}

/*
        all firsts: map<NT, vector>
        
        changed = false
        thru all rules
            /if NT has first symb as Term, add symb to first list
            /if first symb is a NT, add first of NT to current first
            /if rule has no symbs, add empty str 
            if all symbs have eps in their FIRST, add eps                
            if all symbs up until x have eps in FIRST, add FIRST(x)
            
            if FIRST changed size, changed = True

        repeat until changed == false
        sort all firsts based on symblist order
        print firsts
*/
unordered_map<string, unordered_set<string>> Project2::calcAllFirstSets() {
    unordered_map<string, unordered_set<string>> firstSets{};
    unordered_map<string, bool> ruleHasEpsilon{};
    for (auto pair : this->data) {
        string NT = pair.first;
        firstSets[NT] = unordered_set<string>();
        ruleHasEpsilon[NT] = false;
    }

    bool changed = true;
    while (changed){
        changed = false;
        
        // for each rule, apply 5 rules
        for (auto pair : this->data) {
            string rule = pair.first;
            
            vector<vector<string>> rhsList = pair.second;
            // process each rhs as if it's a separate rule
            for (vector<string> rhs : rhsList) {

                

                // just empty string
                if (rhs.size() == 0) {
                    if (!ruleHasEpsilon[rule]){
                        ruleHasEpsilon[rule] = true;
                        changed = true;
                    }
                    continue;
                }
                
                string firstSym = rhs.at(0);

                // add first symb to FIRST if it's a Term
                if (stringInVector(firstSym, this->termList)) {
                    if (firstSets[rule].insert(firstSym).second) {
                        changed = true;
                    }
                }

                // add First(firstSym) if it is NT
                if (this->nonTerms.count(firstSym) > 0) {
                    // insert first(NT) -> first(rule)
                    unordered_set<string> firstOfSym = firstSets[firstSym]; 
                    int preSz = firstSets[rule].size();
                    firstSets[rule].insert(firstOfSym.begin(), firstOfSym.end());
                    
                    if (firstSets[rule].size() != preSz) {
                        changed = true;
                    }
                }

                // if all symbs have eps, add eps to First
                // if All have eps til certain point, add that certain point First to current first
                bool allHaveEps = true; 
                for (string currSym : rhs) {
                    if (this->nonTerms.count(currSym) == 0) {
                        // terminal
                        allHaveEps = false;
                        break;
                    }

                    if (!ruleHasEpsilon[currSym]) {
                        allHaveEps = false;
                        
                        // insert First(currSym) -> First(rule)
                        int preSize = firstSets[rule].size();
                        unordered_set<string> firstOfCurr = firstSets[currSym];
                        firstSets[rule].insert(firstOfCurr.begin(), firstOfCurr.end());
                        if (firstSets[rule].size() != preSize) {
                            changed = true;
                        }
                        break;
                    }
                }
                
                if (allHaveEps && !ruleHasEpsilon[rule]) {
                    ruleHasEpsilon[rule] = true;
                    changed = true;                    
                }
            }
        }
    }
    return firstSets;
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

