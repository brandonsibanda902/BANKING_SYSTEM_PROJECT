#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <limits>

using namespace std;
// ADMIN : BRANDON
//TELLER-ID :T001
// PASSWORD :12345

// ===================== 1. ACCOUNT TYPES (INHERITANCE) =====================
class Account {
public:
    virtual string getTypeName() = 0;
    virtual double getInterestRate() = 0;
    virtual ~Account() {}
};

class SavingsAccount : public Account {
public:
    string getTypeName() override { return "Savings"; }
    double getInterestRate() override { return 0.05; }
};

class ChequeAccount : public Account {
public:
    string getTypeName() override { return "Cheque"; }
    double getInterestRate() override { return 0.01; }
};

class FixedDepositAccount : public Account {
public:
    string getTypeName() override { return "Fixed Deposit"; }
    double getInterestRate() override { return 0.12; }
};

class StudentAccount : public Account {
public:
    string getTypeName() override { return "Student"; }
    double getInterestRate() override { return 0.02; }
};

// ===================== 2. DATA STRUCTURES =====================
#pragma pack(push, 1)
struct Customer {
    char accNo[30];
    char name[50];
    char idNumber[15];
    char contact[15];
    char email[50];
    char address[100];
    char dob[15];
    char pinHash[50];
    double balance;
    int type; 
    char branchCode[10];
};

struct Teller {
    char id[15];
    char name[50];
    char passHash[50];
    char branchCode[10];
};

struct Transaction {
    char accNo[30];
    char type[20];
    double amount;
    char date[30];
};

struct Branch {
    char code[10];
    char name[50];
};
#pragma pack(pop)

// ===================== 3. CORE UTILITIES =====================
string encrypt(string data) {
    string output = data;
    for (int i = 0; i < (int)output.length(); i++) output[i] += 7; 
    return output;
}

void setupSystem() {
    ofstream cfg("system_config.txt");
    cfg << "BANK_NAME=STANDARD_BANK\nVERSION=1.5\n";
    cfg.close();

    ifstream bIn("branches.dat", ios::binary);
    if (!bIn) {
        ofstream bOut("branches.dat", ios::binary);
        Branch b1 = {"BR01", "Pretoria Main"};
        Branch b2 = {"BR02", "Cape Town Central"};
        bOut.write(reinterpret_cast<char*>(&b1), sizeof(Branch));
        bOut.write(reinterpret_cast<char*>(&b2), sizeof(Branch));
        bOut.close();
    }
    bIn.close();
}

void logTransaction(string acc, string type, double amt) {
    Transaction t;
    memset(&t, 0, sizeof(Transaction));
    strncpy(t.accNo, acc.c_str(), 29);
    strncpy(t.type, type.c_str(), 19);
    t.amount = amt;
    time_t now = time(0);
    char* dt = ctime(&now);
    if (dt) { dt[strlen(dt)-1] = '\0'; strncpy(t.date, dt, 29); }
    ofstream out("transactions.dat", ios::binary | ios::app);
    out.write(reinterpret_cast<char*>(&t), sizeof(Transaction));
    out.close();
}

void updateCustomer(Customer &c) {
    fstream file("customers.dat", ios::binary | ios::in | ios::out);
    Customer temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(Customer))) {
        if (strcmp(temp.accNo, c.accNo) == 0) {
            file.seekp((int)file.tellg() - sizeof(Customer));
            file.write(reinterpret_cast<char*>(&c), sizeof(Customer));
            break;
        }
    }
    file.close();
}

// ===================== 4. ADMIN & REPORTING FEATURES =====================
void viewAllBranches() {
    ifstream in("branches.dat", ios::binary);
    Branch b;
    cout << "\n--- BRANCH LIST ---\n";
    cout << left << setw(10) << "CODE" << "NAME" << endl;
    while(in.read(reinterpret_cast<char*>(&b), sizeof(Branch)))
        cout << left << setw(10) << b.code << b.name << endl;
    in.close();
}

void compareBranches() {
    ifstream bin("branches.dat", ios::binary);
    Branch b;
    cout << "\n--- INTER-BRANCH COMPARISON ---\n";
    cout << left << setw(10) << "CODE" << setw(20) << "NAME" << "TOTAL DEPOSITS" << endl;
    while(bin.read(reinterpret_cast<char*>(&b), sizeof(Branch))) {
        ifstream cin_f("customers.dat", ios::binary);
        Customer c; double total = 0;
        while(cin_f.read(reinterpret_cast<char*>(&c), sizeof(Customer)))
            if(strcmp(c.branchCode, b.code) == 0) total += c.balance;
        cout << left << setw(10) << b.code << setw(20) << b.name << "R" << fixed << setprecision(2) << total << endl;
        cin_f.close();
    }
    bin.close();
}

void exportBranchData(string brCode) {
    ifstream in("customers.dat", ios::binary);
    ofstream csv("branch_" + brCode + "_report.csv");
    ofstream txt("branch_" + brCode + "_report.txt");
    csv << "AccNo,Name,ID,Balance\n";
    txt << "REPORT FOR: " << brCode << "\n------------------\n";
    Customer c;
    while (in.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
        if (strcmp(c.branchCode, brCode.c_str()) == 0) {
            csv << c.accNo << "," << c.name << "," << c.idNumber << "," << c.balance << "\n";
            txt << c.accNo << " | " << left << setw(15) << c.name << " | R" << c.balance << "\n";
        }
    }
    cout << "\n[SUCCESS] Reports generated.\n";
    csv.close(); txt.close(); in.close();
}

// ===================== 5. USER INTERFACES =====================
void customerPortal(Customer &c) {
    int ch;
    while (true) {
        cout << "\n--- WELCOME " << c.name << " ---\nBalance: R" << fixed << setprecision(2) << c.balance;
        cout << "\n1. Deposit\n2. Withdraw\n3. Transfer\n4. View Statement\n5. Change PIN\n6. Logout\nChoice: "; cin >> ch;
        if (ch == 6) break;
        else if (ch == 1) { double a; cout << "Amt: "; cin >> a; c.balance += a; updateCustomer(c); logTransaction(c.accNo, "Deposit", a); }
        else if (ch == 2) { double a; cout << "Amt: "; cin >> a; if(a <= c.balance) { c.balance -= a; updateCustomer(c); logTransaction(c.accNo, "Withdraw", a); } }
        else if (ch == 3) {
            char target[30]; double amt; cout << "Recipient Acc: "; cin >> target; cout << "Amount: "; cin >> amt;
            if(amt > 0 && amt <= c.balance) {
                fstream f("customers.dat", ios::binary | ios::in | ios::out); Customer r; bool ok = false;
                while(f.read(reinterpret_cast<char*>(&r), sizeof(Customer))) {
                    if(strcmp(r.accNo, target) == 0) {
                        ok = true; c.balance -= amt; r.balance += amt;
                        f.seekp((int)f.tellg() - sizeof(Customer)); f.write(reinterpret_cast<char*>(&r), sizeof(Customer));
                        logTransaction(c.accNo, "Transfer Sent", amt); logTransaction(r.accNo, "Transfer Recv", amt);
                        break;
                    }
                }
                f.close(); if(ok) updateCustomer(c); else cout << "Recipient not found.\n";
            }
        }
        else if (ch == 4) {
            ifstream in("transactions.dat", ios::binary); Transaction t;
            while(in.read(reinterpret_cast<char*>(&t), sizeof(Transaction)))
                if(strcmp(t.accNo, c.accNo) == 0) cout << t.date << " | " << t.type << " | R" << t.amount << endl;
            in.close();
        }
        else if (ch == 5) {
            char oldP[10], newP[10]; cout << "Current PIN: "; cin >> oldP;
            if(strcmp(c.pinHash, encrypt(string(oldP)).c_str())==0) {
                cout << "New PIN: "; cin >> newP; strncpy(c.pinHash, encrypt(string(newP)).c_str(), 49);
                updateCustomer(c); cout << "Done.\n";
            }
        }
    }
}

// ===================== 6. STAFF INTERFACES =====================
void registerCustomer(string br) {
    Customer c; memset(&c, 0, sizeof(Customer)); cin.ignore();
    cout << "\nName: "; cin.getline(c.name, 50);
    cout << "ID: "; cin.getline(c.idNumber, 15);
    cout << "Contact: "; cin.getline(c.contact, 15);
    cout << "Email: "; cin.getline(c.email, 50);
    cout << "Address: "; cin.getline(c.address, 100);
    cout << "DOB: "; cin.getline(c.dob, 15);
    cout << "Type (1:Savings, 2:Cheque, 3:Fixed, 4:Student): "; cin >> c.type;
    cout << "Deposit: "; cin >> c.balance;
    string acc = "ACC-" + br + "-" + to_string(rand() % 90000 + 10000);
    strncpy(c.accNo, acc.c_str(), 29); strncpy(c.branchCode, br.c_str(), 9);
    int p = rand() % 90000 + 10000; strncpy(c.pinHash, encrypt(to_string(p)).c_str(), 49);
    ofstream out("customers.dat", ios::binary | ios::app);
    out.write(reinterpret_cast<char*>(&c), sizeof(Customer)); out.close();
    cout << "\n[SUCCESS] ACC: " << c.accNo << " | PIN: " << p << endl;
}

void registerTeller() {
    Teller t; memset(&t, 0, sizeof(Teller)); string pass;
    cout << "\n--- REGISTER NEW TELLER ---";
    cout << "\nStaff ID: "; cin >> t.id; cin.ignore();
    cout << "Full Name: "; cin.getline(t.name, 50);
    cout << "Branch Code: "; cin >> t.branchCode;
    cout << "Password: "; cin >> pass;
    strncpy(t.passHash, encrypt(pass).c_str(), 49);
    ofstream out("tellers.dat", ios::binary | ios::app);
    out.write(reinterpret_cast<char*>(&t), sizeof(Teller));
    out.close();
    cout << "[SUCCESS] Teller " << t.name << " registered.\n";
}

void assistedTransaction(string br) {
    char acc[30], pin[10]; cout << "\nCust Acc: "; cin >> acc; cout << "Cust PIN: "; cin >> pin;
    ifstream in("customers.dat", ios::binary); Customer c; string hp = encrypt(string(pin));
    bool found = false;
    while(in.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
        if(strcmp(c.accNo, acc) == 0 && strcmp(c.pinHash, hp.c_str()) == 0) {
            found = true; in.close(); customerPortal(c); break;
        }
    }
    if(!found) { cout << "Auth Failed.\n"; in.close(); }
}

bool staffLogin(string &br, string &nm) {
    char id[15], pass[20]; cout << "\nID: "; cin >> id; cout << "Password: "; cin >> pass;
    if (strcmp(id, "T001") == 0 && strcmp(pass, "12345") == 0) { br="BR01"; nm="BRANDON (ADMIN)"; return true; }
    ifstream in("tellers.dat", ios::binary); Teller t; string hp = encrypt(string(pass));
    while (in.read(reinterpret_cast<char*>(&t), sizeof(Teller))) {
        if (strcmp(t.id, id) == 0 && strcmp(t.passHash, hp.c_str()) == 0) { br = t.branchCode; nm = t.name; return true; }
    }
    return false;
}

int main() {
    setupSystem(); srand((unsigned int)time(0)); int choice;
    while (true) {
        cout << "\n--- STANDARD BANK ---\n1. Staff Login\n2. Customer Login\n3. Exit\nChoice: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
        if (choice == 1) {
            string br, nm;
            if (staffLogin(br, nm)) {
                while (true) {
                    cout << "\nLOGIN: " << nm << " [" << br << "]";
                    cout << "\n1. Register Customer\n2. Register Staff\n3. Export Branch Data\n4. Assisted Transaction";
                    if(nm == "BRANDON (ADMIN)") cout << "\n5. View All Branches\n6. Inter-Branch Comparison";
                    cout << "\n0. Logout\nChoice: "; int sc; cin >> sc;
                    if (sc == 1) registerCustomer(br);
                    else if (sc == 2) registerTeller();
                    else if (sc == 3) exportBranchData(br);
                    else if (sc == 4) assistedTransaction(br);
                    else if (sc == 5 && nm == "BRANDON (ADMIN)") viewAllBranches();
                    else if (sc == 6 && nm == "BRANDON (ADMIN)") compareBranches();
                    else if (sc == 0) break;
                }
            } else cout << "Denied.\n";
        } 
        else if (choice == 2) {
            char acc[30], pin[10]; cout << "Acc: "; cin >> acc; cout << "PIN: "; cin >> pin;
            ifstream in("customers.dat", ios::binary); Customer c; string hp = encrypt(string(pin));
            bool ok = false;
            while (in.read(reinterpret_cast<char*>(&c), sizeof(Customer)))
                if (strcmp(c.accNo, acc) == 0 && strcmp(c.pinHash, hp.c_str()) == 0) { ok = true; in.close(); customerPortal(c); break; }
            if(!ok) cout << "Invalid.\n";
        }
        else if (choice == 3) break;
    }
    return 0;
}