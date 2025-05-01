#include "pumping_lemma.hpp"

using namespace std;

// ==================== Configuration ====================
static const int MAX_N = 50;                // Maximum parameter value
static const int MAX_RECURSION_DEPTH = 50;   // Max depth for expression parsing

// ==================== Utility Functions ====================

// 
// colorize: Wraps a string in ANSI escape codes for colored terminal output.
// Params:
//   str   - the text to colorize
//   color - ANSI color code as string (e.g. "31" for red)
// Returns: colored string
string colorize(const string& str, const string& color) 
{
    return "\033[" + color + "m" + str + "\033[0m";
}

// 
// trim: Removes leading and trailing whitespace from a string.
// Params:
//   str - the input string
// Returns: trimmed string
string trim(const string& str) 
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// 
// pow_int: Computes integer exponentiation safely.
// Throws runtime_error if exponent < 0.
int pow_int(int base, int exp) 
{
    if (exp < 0) throw runtime_error("Negative exponent not supported");
    int result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

// ==================== Expression Evaluation ====================

// 
// eval_expr: Recursively evaluates arithmetic expressions involving n and m.
// Supports +, -, *, /, ^, parentheses, and variables 'n' and 'm'.
// Throws runtime_error for invalid syntax or deep recursion.
int eval_expr(const string& input, int n, int m, int depth) 
{
    if (depth > MAX_RECURSION_DEPTH) {
        throw runtime_error("Expression too deeply nested");
    }

    string expr = trim(input);
    if (expr.empty()) {
        throw runtime_error("Empty expression");
    }

    // Strip matching outer parentheses
    bool stripped;
    do {
        stripped = false;
        if (expr.size() >= 2 && expr.front() == '(' && expr.back() == ')') 
        {
            int balance = 0;
            for (size_t i = 0; i < expr.size(); ++i) 
            {
                balance += (expr[i] == '(') ? 1 : (expr[i] == ')') ? -1 : 0;
                if (balance == 0 && i + 1 < expr.size()) 
                    break;
                
                if (i == expr.size() - 1) 
                {
                    // valid outer parentheses
                    expr = expr.substr(1, expr.size() - 2);
                    stripped = true;
                }
            }
        }
    } while (stripped);

    // Helper lambda to split on operator at top level
    auto split_top = [&](char op)->int 
    {
        int balance = 0;
        for (int i = expr.size() - 1; i >= 0; --i) 
        {
            char c = expr[i];
            balance += (c == ')') ? 1 : (c == '(') ? -1 : 0;
            if (balance == 0 && expr[i] == op) return i;
        }
        return -1;
    };

    // Evaluate + and -
    for (char op : {'+', '-'}) 
    {
        int idx = split_top(op);
        if (idx >= 0) {
            int left = eval_expr(expr.substr(0, idx), n, m, depth + 1);
            int right = eval_expr(expr.substr(idx + 1), n, m, depth + 1);
            return (op == '+') ? left + right : left - right;
        }
    }
    // Evaluate * and /
    for (char op : {'*', '/'}) 
    {
        int idx = split_top(op);
        if (idx >= 0) {
            int left = eval_expr(expr.substr(0, idx), n, m, depth + 1);
            int right = eval_expr(expr.substr(idx + 1), n, m, depth + 1);
            if (op == '/' && right == 0) 
            {
                throw runtime_error("Division by zero");
            }
            return (op == '*') ? left * right : left / right;
        }
    }
    // Evaluate ^ (exponent)
    int idx = split_top('^');
    if (idx >= 0) 
    {
        int base = eval_expr(expr.substr(0, idx), n, m, depth + 1);
        int exp  = eval_expr(expr.substr(idx + 1), n, m, depth + 1);
        return pow_int(base, exp);
    }

    // Variables or integer literal
    if (expr == "n") return n;
    if (expr == "m") return m;
    try 
    {
        return stoi(expr);
    } 
    catch (...) 
    {
        throw runtime_error("Invalid token in expression: '" + expr + "'");
    }
}

// ==================== Pattern Parsing ====================

// Token: holds a symbol and its exponent expression


// 
// parse_pattern: Extracts tokens of the form 'a'^(expr) from input.
// Throws runtime_error for malformed patterns.
vector<Token> parse_pattern(const string& pattern) 
{
    vector<Token> tokens;
    size_t i = 0;
    while (i < pattern.size()) 
    {
        if (pattern[i] == '\'') 
        {
            size_t end_sym = pattern.find('\'', i + 1);
            if (end_sym == string::npos) 
                throw runtime_error("Unclosed quote in pattern");

            string sym = pattern.substr(i + 1, end_sym - i - 1);
            i = end_sym + 1;

            if (i >= pattern.size() || pattern[i] != '^')
            
                throw runtime_error("Missing '^' after symbol");
            
            ++i; // skip '^'
            if (i >= pattern.size() || pattern[i] != '(') 
                throw runtime_error("Missing '(' after '^'");
            
            int depth = 0;
            size_t start_expr = i;
            do 
            {
                if      (pattern[i] == '(') ++depth;
                else if (pattern[i] == ')') --depth;
                ++i;
            } 
            while (i < pattern.size() && depth > 0);
            if (depth != 0) 
            
                throw runtime_error("Unbalanced parentheses in exponent");

            string expr = pattern.substr(start_expr + 1, i - start_expr - 2);
            tokens.push_back({sym, expr});
        } 
        else 
        
            ++i;
    }
    return tokens;
}

// ==================== Condition Checking ====================

// 
// check_conditions: Validates comma-separated conditions like "n>3,m<=5".
// Returns true if all conditions hold for given (n,m).
bool check_conditions(const string& cond_str, int n, int m) 
{
    if (cond_str.empty()) return true;
    size_t start = 0;
    while (start < cond_str.size()) 
    {
        size_t end = cond_str.find(',', start);
        string cond = trim(cond_str.substr(start, end - start));

        // parse operator
        size_t pos = cond.find_first_of("<>=!");
        string lhs = trim(cond.substr(0, pos));
        string op;
        if (pos+1 < cond.size() && cond[pos+1] == '=') 
        
            op = cond.substr(pos, 2);
        else 
            op = cond.substr(pos, 1);

        string rhs = trim(cond.substr(pos+op.size()));

        // evaluate sides
        int lv = (lhs == "n" ? n : lhs == "m" ? m : eval_expr(lhs, n, m));
        int rv = (rhs == "n" ? n : rhs == "m" ? m : eval_expr(rhs, n, m));

        // compare
        bool ok = (op == "<"  ? lv <  rv :
                   op == "<=" ? lv <= rv :
                   op == ">"  ? lv >  rv :
                   op == ">=" ? lv >= rv :
                   op == "==" ? lv == rv :
                   op == "!=" ? lv != rv : false);
        if (!ok) return false;

        if (end == string::npos) break;
        start = end + 1;
    }
    return true;
}

// ==================== String Generation ====================

// 
// generate_string: Builds the string according to tokens and parameters (n,m).
string generate_string(const vector<Token>& tokens, int n, int m) 
{
    string result;
    for (auto& t : tokens) 
    {
        int count = eval_expr(t.exponent_expr, n, m);
        if (count < 0) 
            throw runtime_error("Negative repetition count for symbol: " + t.symbol);
        
        result.append(count, t.symbol[0]);
    }
    return result;
}

// ==================== Pumping Lemma Testing ====================

// 
// test_pumping_cases: Tries up to max_cases splits |xy|≤p, |y|>0.
// For each, pumps y for i=2 and i=3. Reports violations.
void test_pumping_cases(
    const string& s,
    int p,
    const vector<Token>& tokens,
    const string& pattern,
    const string& conditions,
    int max_cases
) {
    cout << "\nTesting up to " << max_cases
         << " splits (|xy|≤" << p << ", |y|>0)...\n";
    int tested = 0;
    int violations = 0;

    // iterate possible |y|=len, |x|=xlen
    for (int len=1; len <= p && tested < max_cases; ++len) {
        for (int xlen=0; xlen + len <= p && tested < max_cases; ++xlen) 
        {
            // split into x, y, z
            string x = s.substr(0, xlen);
            string y = s.substr(xlen, len);
            string z = s.substr(xlen + len);

            cout << "\nTest " << (tested+1)
                 << ": x='" << x << "', y='" << y << "', z='" << z << "'\n";

            // pump y with i = 2, 3
            for (int i_val : {2, 3}) 
            {
                // build pumped string
                string pumped = x;
                pumped.append(i_val, y[0]);
                pumped += z;

                // colorized view
                string view = colorize(x, "36")
                             + colorize(string(i_val*y.size(), y[0]), "32")
                             + colorize(z, "31");
                cout << "  i=" << i_val << ": " << view << "\n";

                // check if in language
                bool in_lang = false;
                bool has_m = (pattern.find('m') != string::npos);
                for (int n = p+1; n < MAX_N && !in_lang; ++n) 
                {
                    for (int m = has_m ? p+1 : 0;
                         m < MAX_N;
                         ++m) {
                        if (!has_m && m > 0) break;
                        if (!check_conditions(conditions, n, m)) continue;
                        if (generate_string(tokens, n, m) == pumped) 
                        {
                            in_lang = true;
                            break;
                        }
                    }
                }

                if (!in_lang) 
                {
                    cout << colorize("    Violation! Not in language.\n", "31");
                    ++violations;
                } 
                else 
                    cout << colorize("    In language.\n", "32");
                
            }

            ++tested;
        }
    }

    cout << "\nSummary: " << violations << " violation(s) out of "
         << tested << " tested cases." << endl;
}

// 
// find_valid_n_m: Finds the smallest n (and m if needed) > p satisfying conditions.
void find_valid_n_m(
    const vector<Token>& tokens,
    const string& pattern,
    const string& conditions,
    int p,
    int& out_n,
    int& out_m
) {
    bool has_m = (pattern.find('m') != string::npos);
    for (int n = p+1; n < MAX_N; ++n) 
    {
        for (int m = has_m ? p+1 : 0; m < MAX_N;++m) 
        {
            if (!has_m && m > 0) break;
            if (!check_conditions(conditions, n, m)) continue;
            out_n = n;
            out_m = m;
            return;
        }
    }
    throw runtime_error("No valid (n,m) satisfy the conditions");
}

