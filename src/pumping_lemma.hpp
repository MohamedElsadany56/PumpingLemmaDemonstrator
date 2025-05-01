#ifndef PUMPING_LEMMA_HPP
#define PUMPING_LEMMA_HPP
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
using namespace std;
struct Token ;
struct Token 
{
    string symbol;
    string exponent_expr;
};
string colorize(const string& str, const string& color);
string trim(const string& str);
int pow_int(int base, int exp);
int eval_expr(const string& input, int n, int m = 0, int depth = 0);
bool check_conditions(const string& cond_str, int n, int m);
string generate_string(const vector<Token>& tokens, int n, int m = 0);
void test_pumping_cases(
    const string& s,
    int p,
    const vector<Token>& tokens,
    const string& pattern,
    const string& conditions,
    int max_cases
);
void find_valid_n_m(
    const vector<Token>& tokens,
    const string& pattern,
    const string& conditions,
    int p,
    int& out_n,
    int& out_m
);


#endif //PUMPING_LEMMA_HPP
