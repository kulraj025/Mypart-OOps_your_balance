#include <windows.h>
#include <iostream> 
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>
#include "bankaccount.h"
#include "savingsaccount.h"
#include "checkingaccount.h"
#include "transaction.h"
#include "admin.h"
#include "utils.h"
using namespace std;

// Set console color
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Clear input buffer utility
void clearInputBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
}

// Styled header
void printStyledHeader() {
    setColor(3); // Aqua/Cyan
    cout << "\n====================================================\n";
    cout << "              OOPS YOUR BALANCE BANK SYSTEM         \n";
    cout << "====================================================\n";
    setColor(7); // Back to normal
}

// General pause utility
void pressEnterPrompt() {
    setColor(11); // Cyan
    cout << "\nPress Enter to continue...";
    setColor(7);
    clearInputBuffer();
    cin.get();
}

// Load Savings Accounts
vector<SavingsAccount> loadSavingsAccounts() {
    vector<SavingsAccount> accounts;
    ifstream fin("accounts_savings.txt");
    string line;
    while(getline(fin, line)) {
        SavingsAccount acc;
        acc.fromCSV(line);
        accounts.push_back(acc);
    }
    return accounts;
}

void saveSavingsAccounts(const vector<SavingsAccount>& accounts) {
    ofstream fout("accounts_savings.txt", ios::trunc);
    for(const auto& acc : accounts)
        fout << acc.toCSV() << endl;
}

vector<CheckingAccount> loadCheckingAccounts() {
    vector<CheckingAccount> accounts;
    ifstream fin("accounts_checking.txt");
    string line;
    while(getline(fin, line)) {
        CheckingAccount acc;
        acc.fromCSV(line);
        accounts.push_back(acc);
    }
    return accounts;
}

void saveCheckingAccounts(const vector<CheckingAccount>& accounts) {
    ofstream fout("accounts_checking.txt", ios::trunc);
    for(const auto& acc : accounts)
        fout << acc.toCSV() << endl;
}

void logTransaction(const Transaction& tx) {
    ofstream fout("transaction.txt", ios::app);
    fout << tx.toCSV() << endl;
}

// Find Account
BankAccount* findAccount(vector<SavingsAccount>& sa, vector<CheckingAccount>& ca,
                         const string& type, const string& number, const string& password_hash) {
    if(type == "savings") {
        for(auto& acc : sa)
            if(acc.getAccountNumber() == number && acc.verifyPassword(password_hash))
                return &acc;
    }
    if(type == "checking") {
        for(auto& acc : ca)
            if(acc.getAccountNumber() == number && acc.verifyPassword(password_hash))
                return &acc;
    }
    return nullptr;
}

// Show Account Transaction History
void showAccountTransactions(const string& acc_no) {
    setColor(13); // Purple
    cout << "\n--- Transaction History ---\n";
    setColor(7);
    ifstream fin("transaction.txt");
    string line;
    bool found = false;
    while(getline(fin, line)) {
        Transaction tx = Transaction::fromCSV(line);
        if(tx.account_number == acc_no) {
            found = true;
            cout << tx.timestamp << " | " << tx.type
                 << ": $" << tx.amount << " | Bal after: $" << tx.balance_after
                 << " [" << tx.description << "]" << endl;
        }
    }
    if(!found)
        cout << "No transactions found for this account.\n";
}
bool accountNumberExists(const string& acc_no, const vector<SavingsAccount>& sa, const vector<CheckingAccount>& ca) {
    for (const auto& acc : sa)
        if (acc.getAccountNumber() == acc_no)
            return true;
    for (const auto& acc : ca)
        if (acc.getAccountNumber() == acc_no)
            return true;
    return false;
}

// Create New Account
void createAccount(vector<SavingsAccount>& sa, vector<CheckingAccount>& ca) {
    setColor(10); // Green
    cout << "\n--- Create Account: (savings/checking) ---\n";
    setColor(7);
    string type;
    cout << "Account type: ";
    cin >> type; type = toLower(type);

    if(type != "savings" && type != "checking") {
        setColor(12); cout << "Invalid account type!\n"; setColor(7);
        return;
    }

    cout << "New account number (5+ digits): ";
    string acc_no; cin >> acc_no;
    if(!isValidAccountNumber(acc_no)) {
        setColor(12); cout << "Invalid account number!\n"; setColor(7);
        return;
    }
    if(accountNumberExists(acc_no, sa, ca)) {
    setColor(12); cout << "Account number already exists!\n"; setColor(7);
    return;
   }

    cout << "Owner name: ";
    string name; cin.ignore(); getline(cin, name);

    cout << "Password: ";
    string password; cin >> password;

    cout << "Initial deposit: ";
    string deposit_str; cin >> deposit_str;
    if(!isValidDouble(deposit_str)) {
        setColor(12); cout << "Invalid deposit amount!\n"; setColor(7);
        return;
    }
    double deposit = stod(deposit_str);

    if(type == "savings") {
        SavingsAccount acc(acc_no, name, simple_hash(password), deposit);
        sa.push_back(acc);
        saveSavingsAccounts(sa);
        setColor(10); cout << "Savings account created!\n"; setColor(7);
    } else {
        cout << "Overdraft limit (default 100): ";
        string odl_str; cin >> odl_str;
        double odv = isValidDouble(odl_str) ? stod(odl_str) : 100.0;
        CheckingAccount acc(acc_no, name, simple_hash(password), deposit, odv);
        ca.push_back(acc);
        saveCheckingAccounts(ca);
        setColor(10); cout << "Checking account created!\n"; setColor(7);
    }
}

void clientMenu(BankAccount* account) {
    bool running = true;
    while(running) {
        setColor(11); // Cyan
        cout << "\n============== Client Account Menu ==============\n";
        setColor(7);
        cout << " 1. Deposit\n 2. Withdraw\n 3. Check Balance\n 4. Transaction History\n 5. Account Info\n 6. Apply Interest (Savings)\n 7. Exit\n";
        setColor(14); cout << "Select option: "; setColor(7);
        int choice; cin >> choice;
        string amount_str;
        switch(choice) {
            case 1:
                setColor(3); cout << "Deposit amount: "; setColor(7); cin >> amount_str;
                if(!isValidDouble(amount_str)) { setColor(12); cout << "Invalid!\n"; setColor(7); break; }
                account->deposit(stod(amount_str));
                logTransaction(Transaction(account->getAccountNumber(), "Deposit", stod(amount_str), account->getBalance(), Transaction::currentTimestamp(), "User Deposit"));
                setColor(10); cout << "Deposit successful!\n"; setColor(7);
                break;
            case 2:
                setColor(3); cout << "Withdraw amount: "; setColor(7); cin >> amount_str;
                if(!isValidDouble(amount_str)) { setColor(12); cout << "Invalid!\n"; setColor(7); break; }
                if(account->withdraw(stod(amount_str))) {
                    logTransaction(Transaction(account->getAccountNumber(), "Withdraw", stod(amount_str), account->getBalance(), Transaction::currentTimestamp(), "User Withdraw"));
                    setColor(10); cout << "Withdraw successful!\n"; setColor(7);
                } else {
                    setColor(12); cout << "Withdraw failed!\n"; setColor(7);
                }
                break;
            case 3:
                setColor(13); cout << "Current Balance: $" << account->getBalance() << endl; setColor(7);
                break;
            case 4:
                showAccountTransactions(account->getAccountNumber());
                break;
            case 5:
                setColor(9); account->displayInfo(); setColor(7);
                break;
            case 6:
                if(dynamic_cast<SavingsAccount*>(account)) {
                    cout << "Interest rate (e.g. 0.02 for 2%): ";
                    double rate; cin >> rate;
                    SavingsAccount* sa = dynamic_cast<SavingsAccount*>(account);
                    sa->applyInterest(rate);
                    logTransaction(Transaction(account->getAccountNumber(), "Interest", account->getBalance() * rate, account->getBalance(), Transaction::currentTimestamp(), "Interest Applied"));
                    setColor(10); cout << "Interest applied!\n"; setColor(7);
                } else {
                    setColor(12); cout << "Interest feature only for savings accounts!\n"; setColor(7);
                }
                break;
            case 7:
                running = false; break;
            default:
                setColor(12); cout << "Invalid option!\n"; setColor(7);
        }
        pressEnterPrompt();
    }
}

void adminMenu(vector<SavingsAccount>& sa, vector<CheckingAccount>& ca) {
    bool running = true;
    while(running) {
        setColor(13); cout << "\n============== Admin Menu ==============\n"; setColor(7);
        cout << " 1. View All Accounts\n 2. View Transactions by Account\n 3. Delete an Account\n 4. Reset User Password\n 5. Exit\n";
        setColor(14); cout << "Select option: "; setColor(7);
        int choice; cin >> choice;
        clearInputBuffer();
        switch (choice) {
            case 1:
                setColor(10); cout << "\n--- Savings Accounts ---\n"; setColor(7);
                for(const auto& acc : sa) acc.displayInfo();
                setColor(3); cout << "\n--- Checking Accounts ---\n"; setColor(7);
                for(const auto& acc : ca) acc.displayInfo();
                break;
            case 2:
                cout << "Enter account number: ";
                { string acc_no; cin >> acc_no; showAccountTransactions(acc_no); }
                break;
            case 3:
                cout << "Enter account number to delete: ";
                {
                    string acc_no; cin >> acc_no; bool found = false;
                    for(auto it = sa.begin(); it != sa.end(); ++it)
                        if(it->getAccountNumber() == acc_no) { sa.erase(it); found = true; break; }
                    for(auto it = ca.begin(); it != ca.end(); ++it)
                        if(it->getAccountNumber() == acc_no) { ca.erase(it); found = true; break; }
                    if(found) {
                        saveSavingsAccounts(sa); saveCheckingAccounts(ca);
                        setColor(10); cout << "Account deleted.\n"; setColor(7);
                    } else {
                        setColor(12); cout << "Account not found!\n"; setColor(7);
                    }
                }
                break;
            case 4:
                cout << "Account number: ";
                {
                    string acc_no; cin >> acc_no;
                    cout << "New password: "; string pwd; cin >> pwd;
                    bool found = false;
                    for(auto& acc : sa)
                        if(acc.getAccountNumber() == acc_no) { acc.setPasswordHash(simple_hash(pwd)); found = true; }
                    for(auto& acc : ca)
                        if(acc.getAccountNumber() == acc_no) { acc.setPasswordHash(simple_hash(pwd)); found = true; }
                    if(found) {
                        saveSavingsAccounts(sa); saveCheckingAccounts(ca);
                        setColor(10); cout << "Password reset.\n"; setColor(7);
                    } else {
                        setColor(12); cout << "Account not found!\n"; setColor(7);
                    }
                }
                break;
            case 5:
                running = false; break;
            default:
                setColor(12); cout << "Invalid option!\n"; setColor(7);
        }
        pressEnterPrompt();
    }
}

int main() {
    system("chcp 65001 > nul"); // Enable UTF-8 (for box lines)
    vector<SavingsAccount> savingsAccounts = loadSavingsAccounts();
    vector<CheckingAccount> checkingAccounts = loadCheckingAccounts();
    Admin admin;

    while(true) {
        system("cls");
        printStyledHeader();
        setColor(11); // Cyan
        cout << "\n 1. Client Login\n 2. Admin Login\n 3. Create Account\n 4. Exit\n";
        setColor(14); cout << "\nSelect option: "; setColor(7);
        int main_choice;
        cin >> main_choice;
        clearInputBuffer();
        if(main_choice == 1) {
            setColor(10); cout << "\n--- Client Login ---\n"; setColor(7);
            cout << "Account type (savings/checking): ";
            string type; cin >> type; type = toLower(type);

            cout << "Account number: ";
            string acc_no; cin >> acc_no;

            cout << "Password: ";
            string pwd; cin >> pwd;
            BankAccount* account = findAccount(savingsAccounts, checkingAccounts, type, acc_no, simple_hash(pwd));
            if(account) {
                setColor(10); cout << "Login successful!\n"; setColor(7);
                clientMenu(account);
                saveSavingsAccounts(savingsAccounts);
                saveCheckingAccounts(checkingAccounts);
            } else {
                setColor(12); cout << "Invalid login!\n"; setColor(7);
            }
            pressEnterPrompt();
        } else if(main_choice == 2) {
            setColor(13); cout << "\n--- Admin Login ---\n"; setColor(7);
            cout << "Admin username: "; string username; cin >> username;
            cout << "Admin password: "; string pwd; cin >> pwd;
            if(admin.login(username, pwd)) {
                setColor(10); cout << "Admin login successful!\n"; setColor(7);
                adminMenu(savingsAccounts, checkingAccounts);
            } else {
                setColor(12); cout << "Admin authentication failed.\n"; setColor(7);
            }
            pressEnterPrompt();
        } else if(main_choice == 3) {
            createAccount(savingsAccounts, checkingAccounts);
            pressEnterPrompt();
        } else if(main_choice == 4) {
            setColor(10); cout << "Thank you for using OOPS_YOUR_BALANCE! Goodbye!\n"; setColor(7);
            break;
        } else {
            setColor(12); cout << "Invalid option!\n"; setColor(7);
            pressEnterPrompt();
        }
    }
    saveSavingsAccounts(savingsAccounts);
    saveCheckingAccounts(checkingAccounts);
    return 0;
}

