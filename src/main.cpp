#include "pumping_lemma.cpp"

// ==================== Main Interface ====================
int main()
{
    while (true) 
    {
        // Input stage
        cout << "\n=== Pumping Lemma Tester ===\n";
        cout << "Enter pattern (or 'exit' to quit): ";
        string pattern;
        getline(cin, pattern);
        if (pattern == "exit") break;

        cout << "Enter conditions (comma-separated, e.g. n>3,m<=5): ";
        string conditions;
        getline(cin, conditions);

        cout << "Enter pumping length p: ";
        int p;
        cin >> p;

        cout << "Enter number of splits to test: ";
        int max_cases;
        cin >> max_cases;
        cin.ignore();

        try {
            // Parse and generate initial string
            auto tokens = parse_pattern(pattern);
            int n=0, m=0;
            find_valid_n_m(tokens, pattern, conditions, p, n, m);

            string base = generate_string(tokens, n, m);
            cout << "\nGenerated string (n=" << n;
            if (pattern.find('m') != string::npos) cout << ", m=" << m;
            cout << "): " << base << "\n";

            // Test pumping lemma
            test_pumping_cases(base, p, tokens, pattern, conditions, max_cases);
        }
        catch (const exception& e) 
        {
            cerr << colorize("Error: ", "31") << e.what() << "\n";
        }
    }
    return 0;
}
