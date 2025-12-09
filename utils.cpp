#include "utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>

string simple_hash(const string& password) {
    // Simple hash for demonstration (NOT for real security!)
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return to_string(hash);
}

bool isValidAccountNumber(const string& account_number) {
    // Valid if all digits and at least 5 characters
    return account_number.length() >= 5 &&
           all_of(account_number.begin(), account_number.end(), ::isdigit);
}

bool isValidDouble(const string& value) {
    // Try to parse as double, checks all input consumed
    std::istringstream iss(value);
    double d;
    iss >> std::noskipws >> d;
    return iss.eof() && !iss.fail();
}

string toLower(const string& input) {
    string output = input;
    transform(output.begin(), output.end(), output.begin(),
              [](unsigned char c){ return tolower(c); });
    return output;
}