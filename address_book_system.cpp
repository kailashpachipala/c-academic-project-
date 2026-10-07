#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class Contact {
    string name, phone, email, address, pincode, city, state;

public:
    Contact(string n="", string p="", string e="", string a="",
            string pin="", string c="", string s="")
        : name(n), phone(p), email(e), address(a),
          pincode(pin), city(c), state(s) {}

    const string& getName() const { return name; }
    const string& getPhone() const { return phone; }
    const string& getEmail() const { return email; }
    const string& getAddress() const { return address; }
    const string& getPincode() const { return pincode; }
    const string& getCity() const { return city; }
    const string& getState() const { return state; }

    void setName(const string& x) { name=x; }
    void setPhone(const string& x) { phone=x; }
    void setEmail(const string& x) { email=x; }
    void setAddress(const string& x) { address=x; }
    void setPincode(const string& x) { pincode=x; }
    void setCity(const string& x) { city=x; }
    void setState(const string& x) { state=x; }

    void display(int n) const {
        cout << "\n--- Contact " << n << " ---"
             << "\nName    : " << name
             << "\nPhone   : " << phone
             << "\nEmail   : " << email
             << "\nAddress : " << address
             << "\nPincode : " << pincode
             << "\nCity    : " << city
             << "\nState   : " << state << '\n';
    }
};

class AddressBook {
    vector<Contact> contacts;
    const string file="contacts.db";

    static string lower(string s) {
        transform(s.begin(),s.end(),s.begin(),
            [](unsigned char c){ return tolower(c); });
        return s;
    }

    static string input(const string& msg) {
        cout << msg;
        string s;
        if(!getline(cin,s)) {
            cout << "\nInput closed. Exiting safely.\n";
            exit(0);
        }
        return s;
    }

    static bool digits(const string& s) {
        return !s.empty() && all_of(s.begin(),s.end(),
            [](unsigned char c){ return isdigit(c); });
    }

    static bool textOK(const string& s) {
        if(s.size()<2 || s.size()>60) return false;
        bool letter=false;
        for(unsigned char c:s) {
            if(isalpha(c)) letter=true;
            else if(c!=' ' && c!='-' && c!='.' && c!='\'') return false;
        }
        return letter;
    }

    static bool phoneOK(const string& s) {
        return s.size()==10 && digits(s) && s[0]>='6' && s[0]<='9';
    }

    static bool pinOK(const string& s) {
        return s.size()==6 && digits(s);
    }

    static bool addressOK(const string& s) {
        return !s.empty() && s.size()<=200;
    }

    static bool emailOK(const string& s) {
        if(s.empty() || s.size()>100 || s.find(' ')!=string::npos) return false;

        size_t at=s.find('@');
        if(at==string::npos || at==0 || at!=s.rfind('@') || at==s.size()-1)
            return false;

        string local=s.substr(0,at), domain=s.substr(at+1);
        size_t dot=domain.rfind('.');

        return local.front()!='.' && local.back()!='.' &&
               local.find("..")==string::npos &&
               domain.find("..")==string::npos &&
               dot!=string::npos && dot>0 && dot<domain.size()-1;
    }

    bool phoneExists(const string& x,int skip=-1) const {
        for(int i=0;i<(int)contacts.size();i++)
            if(i!=skip && contacts[i].getPhone()==x) return true;
        return false;
    }

    bool emailExists(const string& x,int skip=-1) const {
        for(int i=0;i<(int)contacts.size();i++)
            if(i!=skip && lower(contacts[i].getEmail())==lower(x)) return true;
        return false;
    }

    string validInput(const string& msg, bool(*check)(const string&),
                      const string& error) const {
        while(true) {
            string x=input(msg);
            if(check(x)) return x;
            cout << "ERROR: " << error << '\n';
        }
    }

    string optionalInput(const string& msg, bool(*check)(const string&),
                         const string& error) const {
        while(true) {
            string x=input(msg);
            if(x.empty() || check(x)) return x;
            cout << "ERROR: " << error << '\n';
        }
    }

    string getPhone(const string& msg,int skip=-1) const {
        while(true) {
            string x=input(msg);

            if(x.empty()) return x;

            if(x.size()!=10)
                cout << "ERROR: Phone must contain exactly 10 digits.\n";
            else if(!digits(x))
                cout << "ERROR: Phone must contain numbers only.\n";
            else if(x[0]<'6' || x[0]>'9')
                cout << "ERROR: Phone must start with 6, 7, 8 or 9.\n";
            else if(phoneExists(x,skip))
                cout << "ERROR: Phone number already exists.\n";
            else return x;
        }
    }

    string getEmail(const string& msg,int skip=-1) const {
        while(true) {
            string x=input(msg);

            if(x.empty()) return x;

            if(!emailOK(x))
                cout << "ERROR: Enter a valid email, example: name@gmail.com\n";
            else if(emailExists(x,skip))
                cout << "ERROR: Email already exists.\n";
            else return x;
        }
    }

    vector<int> findName(const string& name) const {
        vector<int> result;
        for(int i=0;i<(int)contacts.size();i++)
            if(lower(contacts[i].getName())==lower(name))
                result.push_back(i);
        return result;
    }

    vector<int> findNameContaining(const string& text) const {
        vector<int> result;
        string searchText=lower(text);

        for(int i=0;i<(int)contacts.size();i++)
            if(lower(contacts[i].getName()).find(searchText)!=string::npos)
                result.push_back(i);

        return result;
    }

    int choose(const vector<int>& v,const string& action) const {
        if(v.empty()) {
            cout << "Contact not found.\n";
            return -1;
        }

        if(v.size()==1) return v[0];

        cout << "\nMultiple contacts found:\n";
        for(int i=0;i<(int)v.size();i++)
            cout << i+1 << ". " << contacts[v[i]].getName()
                 << " | " << contacts[v[i]].getPhone() << '\n';

        while(true) {
            string s=input("Choose contact to "+action+" (0 to cancel): ");
            stringstream ss(s);
            int n; char extra;

            if((ss>>n) && !(ss>>extra) && n>=0 && n<=(int)v.size())
                return n==0 ? -1 : v[n-1];

            cout << "ERROR: Enter a valid contact number.\n";
        }
    }

    bool save() const {
        ofstream out(file);
        if(!out) {
            cout << "ERROR: Unable to save contacts.\n";
            return false;
        }

        for(const auto& c:contacts)
            out << quoted(c.getName()) << ' ' << quoted(c.getPhone()) << ' '
                << quoted(c.getEmail()) << ' ' << quoted(c.getAddress()) << ' '
                << quoted(c.getPincode()) << ' ' << quoted(c.getCity()) << ' '
                << quoted(c.getState()) << '\n';

        return (bool)out;
    }

    void load() {
        ifstream in(file);
        string n,p,e,a,pin,c,s;

        while(in>>quoted(n)>>quoted(p)>>quoted(e)>>quoted(a)
                >>quoted(pin)>>quoted(c)>>quoted(s)) {
                if((n.empty() || textOK(n)) && (p.empty() || phoneOK(p)) &&
                    (e.empty() || emailOK(e)) && (a.empty() || addressOK(a)) &&
                    (pin.empty() || pinOK(pin)) && (c.empty() || textOK(c)) &&
                    (s.empty() || textOK(s)) &&
               !phoneExists(p) && !emailExists(e))
                contacts.emplace_back(n,p,e,a,pin,c,s);
        }
    }

    void updateText(Contact& c,const string& label,const string& old,
                    bool(*check)(const string&),
                    void(Contact::*setter)(const string&)) {
        while(true) {
            string x=input("New "+label+" ["+old+"]: ");
            if(x.empty()) return;
            if(check(x)) {
                (c.*setter)(x);
                return;
            }
            cout << "ERROR: Invalid " << label << ".\n";
        }
    }

public:
    AddressBook() {
        contacts.reserve(1000);
        load();
    }

    void add() {
        if(contacts.size()>=1000) {
            cout << "ERROR: Address Book is full.\n";
            return;
        }

        cout << "\n===== ADD CONTACT =====\n";

        string n=optionalInput("Name: ",textOK,
            "Name must contain 2-60 valid characters.");
        string p=getPhone("Phone: ");
        string e=getEmail("Email: ");
        string a=optionalInput("Address: ",addressOK,
            "Address must not be longer than 200 characters.");
        string pin=optionalInput("Pincode: ",pinOK,
            "Pincode must contain exactly 6 digits.");
        string c=optionalInput("City: ",textOK,
            "Enter a valid city name.");
        string s=optionalInput("State: ",textOK,
            "Enter full state name, example: Andhra Pradesh.");

        contacts.emplace_back(n,p,e,a,pin,c,s);

        cout << (save() ? "SUCCESS: Contact added successfully.\n"
                        : "WARNING: Contact added but saving failed.\n");
    }

    void display() const {
        cout << "\n===== ALL CONTACTS =====\n";

        if(contacts.empty()) {
            cout << "No contacts available.\n";
            return;
        }

        for(int i=0;i<(int)contacts.size();i++)
            contacts[i].display(i+1);

        cout << "\nTotal Contacts: " << contacts.size() << '\n';
    }

    void search() const {
        if(contacts.empty()) {
            cout << "No contacts available to search.\n";
            return;
        }

        string n=input("Enter name or part of name to search: ");

        if(!textOK(n)) {
            cout << "ERROR: Enter a valid name.\n";
            return;
        }

        auto v=findNameContaining(n);

        if(v.empty()) {
            cout << "Contact not found.\n";
            return;
        }

        cout << "Found " << v.size() << " matching contact(s).\n";
        for(int i:v) contacts[i].display(i+1);
    }

    void update() {
        if(contacts.empty()) {
            cout << "No contacts available to update.\n";
            return;
        }

        string n=input("Enter exact name to update: ");
        int i=choose(findName(n),"update");

        if(i<0) return;

        Contact& c=contacts[i];
        c.display(i+1);

        cout << "\nLeave a field blank to keep the old value.\n";

        updateText(c,"Name",c.getName(),textOK,&Contact::setName);

        while(true) {
            string x=input("New Phone ["+c.getPhone()+"]: ");
            if(x.empty()) break;

            if(!phoneOK(x))
                cout << "ERROR: Phone must be 10 digits and start with 6-9.\n";
            else if(phoneExists(x,i))
                cout << "ERROR: Phone already exists.\n";
            else {
                c.setPhone(x);
                break;
            }
        }

        while(true) {
            string x=input("New Email ["+c.getEmail()+"]: ");
            if(x.empty()) break;

            if(!emailOK(x))
                cout << "ERROR: Enter a valid email address.\n";
            else if(emailExists(x,i))
                cout << "ERROR: Email already exists.\n";
            else {
                c.setEmail(x);
                break;
            }
        }

        updateText(c,"Address",c.getAddress(),addressOK,&Contact::setAddress);
        updateText(c,"Pincode",c.getPincode(),pinOK,&Contact::setPincode);
        updateText(c,"City",c.getCity(),textOK,&Contact::setCity);
        updateText(c,"State",c.getState(),textOK,&Contact::setState);

        cout << (save() ? "SUCCESS: Contact updated successfully.\n"
                        : "WARNING: Updated but saving failed.\n");

        c.display(i+1);
    }

    void removeContact() {
        if(contacts.empty()) {
            cout << "No contacts available to delete.\n";
            return;
        }

        int i=choose(findName(input("Enter exact name to delete: ")),"delete");

        if(i<0) return;

        contacts[i].display(i+1);

        if(lower(input("Type YES to confirm deletion: "))!="yes") {
            cout << "Deletion cancelled.\n";
            return;
        }

        contacts.erase(contacts.begin()+i);

        cout << (save() ? "SUCCESS: Contact deleted successfully.\n"
                        : "WARNING: Deleted but saving failed.\n");
    }

    void sortContacts() {
        if(contacts.empty()) {
            cout << "No contacts available to sort.\n";
            return;
        }

        stable_sort(contacts.begin(),contacts.end(),
            [](const Contact& a,const Contact& b) {
                return lower(a.getName()) < lower(b.getName());
            });

        cout << (save() ? "SUCCESS: Contacts sorted alphabetically.\n"
                        : "WARNING: Sorted but saving failed.\n");

        display();
    }

    void run() {
        while(true) {
            cout << "\n================================"
                 << "\n      ADDRESS BOOK SYSTEM"
                 << "\n================================"
                 << "\n1. Add Contact"
                 << "\n2. Display All Contacts"
                 << "\n3. Search Contact"
                 << "\n4. Update Contact"
                 << "\n5. Delete Contact"
                 << "\n6. Sort Contacts"
                 << "\n7. Exit"
                 << "\n================================\n";

            string s=input("Enter choice (1-7): ");
            stringstream ss(s);
            int choice; char extra;

            if(!(ss>>choice) || (ss>>extra)) {
                cout << "ERROR: Enter one number from 1 to 7 only.\n";
                continue;
            }

            switch(choice) {
                case 1: add(); break;
                case 2: display(); break;
                case 3: search(); break;
                case 4: update(); break;
                case 5: removeContact(); break;
                case 6: sortContacts(); break;
                case 7:
                    cout << "Thank you for using Address Book System.\n";
                    return;
                default:
                    cout << "ERROR: Choice must be between 1 and 7.\n";
            }
        }
    }
};

int main() {
    try {
        AddressBook book;
        book.run();
    }
    catch(const exception& e) {
        cerr << "Unexpected error: " << e.what() << '\n';
        return 1;
    }
    catch(...) {
        cerr << "Unknown error occurred.\n";
        return 1;
    }

    return 0;
}