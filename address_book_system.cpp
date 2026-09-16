#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

using namespace std;

class Contact {
private:
    string name;
    string phone;
    string email;

public:
    Contact() = default;

    Contact(string n, string p, string e)
        : name(std::move(n)), phone(std::move(p)), email(std::move(e)) {}

    const string& getName() const { return name; }
    const string& getPhone() const { return phone; }
    const string& getEmail() const { return email; }

    void setName(const string& n) { name = n; }
    void setPhone(const string& p) { phone = p; }
    void setEmail(const string& e) { email = e; }

    void display(size_t number) const {
        cout << "\nContact " << number << '\n';
        cout << "Name  : " << name << '\n';
        cout << "Phone : " << phone << '\n';
        cout << "Email : " << email << '\n';
    }
};

class AddressBook {
private:
    static constexpr size_t MAX_CONTACTS = 1000;
    static constexpr size_t MAX_NAME_LEN = 60;
    static constexpr size_t MAX_EMAIL_LEN = 100;
    static constexpr size_t MAX_LINE_LEN = 300;

    vector<Contact> contacts;
    const string dataFile = "contacts.db";

    static string trim(const string& input) {
        size_t start = 0;
        while (start < input.size() &&
               isspace(static_cast<unsigned char>(input[start]))) {
            ++start;
        }

        size_t end = input.size();
        while (end > start &&
               isspace(static_cast<unsigned char>(input[end - 1]))) {
            --end;
        }

        return input.substr(start, end - start);
    }

    static string toLowerCopy(string value) {
        transform(value.begin(), value.end(), value.begin(),
                  [](unsigned char ch) {
                      return static_cast<char>(tolower(ch));
                  });
        return value;
    }

    static bool isPrintableAscii(const string& text) {
        for (unsigned char ch : text) {
            if (ch < 32 || ch > 126) {
                return false;
            }
        }
        return true;
    }

    static bool validName(const string& rawName) {
        string name = trim(rawName);

        if (name.size() < 2 || name.size() > MAX_NAME_LEN ||
            !isPrintableAscii(name)) {
            return false;
        }

        bool hasLetter = false;

        for (unsigned char ch : name) {
            if (isalpha(ch)) {
                hasLetter = true;
            } else if (ch != ' ' && ch != '-' && ch != '\'' && ch != '.') {
                return false;
            }
        }

        return hasLetter;
    }

    static bool validPhone(const string& rawPhone) {
        string phone = trim(rawPhone);

        // Simple, predictable format: 7 to 15 digits.
        if (phone.size() < 7 || phone.size() > 15) {
            return false;
        }

        return all_of(phone.begin(), phone.end(),
                      [](unsigned char ch) { return isdigit(ch); });
    }

    static bool validEmail(const string& rawEmail) {
        string email = trim(rawEmail);

        if (email.empty() || email.size() > MAX_EMAIL_LEN ||
            !isPrintableAscii(email)) {
            return false;
        }

        if (email.find(' ') != string::npos ||
            email.find('\t') != string::npos) {
            return false;
        }

        size_t at = email.find('@');

        if (at == string::npos || at == 0 ||
            at != email.rfind('@') || at == email.size() - 1) {
            return false;
        }

        string local = email.substr(0, at);
        string domain = email.substr(at + 1);

        if (local.front() == '.' || local.back() == '.' ||
            local.find("..") != string::npos) {
            return false;
        }

        size_t dot = domain.rfind('.');
        if (dot == string::npos || dot == 0 || dot == domain.size() - 1) {
            return false;
        }

        if (domain.find("..") != string::npos) {
            return false;
        }

        for (unsigned char ch : domain) {
            if (!(isalnum(ch) || ch == '-' || ch == '.')) {
                return false;
            }
        }

        return true;
    }

    static bool parseMenuChoice(const string& text, int& choice) {
        string cleaned = trim(text);

        if (cleaned.empty() || cleaned.size() > 2) {
            return false;
        }

        stringstream ss(cleaned);
        int value;
        char extra;

        if (!(ss >> value)) {
            return false;
        }

        if (ss >> extra) {
            return false;
        }

        choice = value;
        return true;
    }

    bool phoneExists(const string& phone, int ignoreIndex = -1) const {
        for (size_t i = 0; i < contacts.size(); ++i) {
            if (static_cast<int>(i) == ignoreIndex) {
                continue;
            }

            if (contacts[i].getPhone() == phone) {
                return true;
            }
        }

        return false;
    }

    bool emailExists(const string& email, int ignoreIndex = -1) const {
        string target = toLowerCopy(email);

        for (size_t i = 0; i < contacts.size(); ++i) {
            if (static_cast<int>(i) == ignoreIndex) {
                continue;
            }

            if (toLowerCopy(contacts[i].getEmail()) == target) {
                return true;
            }
        }

        return false;
    }

    vector<size_t> findByName(const string& name) const {
        vector<size_t> matches;
        string target = toLowerCopy(trim(name));

        for (size_t i = 0; i < contacts.size(); ++i) {
            if (toLowerCopy(contacts[i].getName()) == target) {
                matches.push_back(i);
            }
        }

        return matches;
    }

    static string readLine(const string& prompt) {
        cout << prompt;
        string value;

        if (!getline(cin, value)) {
            if (cin.eof()) {
                cout << "\nInput stream closed. Exiting safely.\n";
                exit(0);
            }
            // Non-EOF error on stdin: return empty to let caller handle
            cout << "\nInput error. Exiting.\n";
            exit(0);
        }

        return trim(value);
    }

    string readValidName(const string& prompt) const {
        while (true) {
            string value = readLine(prompt);

            if (!value.empty()) {
                if (validName(value)) {
                    return value;
                }
            }

            cout << "Invalid name. Use 2-60 letters and only spaces, "
                    "hyphens, apostrophes or periods.\n";
        }
    }

    string readValidPhone(const string& prompt, int ignoreIndex = -1) const {
        while (true) {
            string value = readLine(prompt);

            if (value.empty()) {
                // EOF: exit gracefully rather than infinite loop
                cout << "\nInput stream closed. Exiting.\n";
                exit(0);
            }

            if (!validPhone(value)) {
                cout << "Invalid phone. Enter 7-15 digits only.\n";
                continue;
            }

            if (phoneExists(value, ignoreIndex)) {
                cout << "That phone number already exists.\n";
                continue;
            }

            return value;
        }
    }

    string readValidEmail(const string& prompt, int ignoreIndex = -1) const {
        while (true) {
            string value = readLine(prompt);

            if (value.empty()) {
                // EOF: exit gracefully rather than infinite loop
                cout << "\nInput stream closed. Exiting.\n";
                exit(0);
            }

            if (!validEmail(value)) {
                cout << "Invalid email address.\n";
                continue;
            }

            if (emailExists(value, ignoreIndex)) {
                cout << "That email address already exists.\n";
                continue;
            }

            return value;
        }
    }

    bool saveToFile() const {
        const string tempFile = dataFile + ".tmp";

        ofstream out(tempFile, ios::trunc);
        if (!out) {
            cerr << "Warning: unable to create temp file.\n";
            return false;
        }

        for (const auto& contact : contacts) {
            out << quoted(contact.getName()) << ' '
                << quoted(contact.getPhone()) << ' '
                << quoted(contact.getEmail()) << '\n';

            if (!out) {
                cerr << "Warning: write failed. Leaving original data intact.\n";
                out.close();
                std::error_code ec;
                filesystem::remove(tempFile, ec);
                return false;
            }
        }

        // Flush data to disk, then close. Check for flush/close errors.
        if (!out.good()) {
            cerr << "Warning: flush error. Leaving original data intact.\n";
            out.close();
            std::error_code ec;
            filesystem::remove(tempFile, ec);
            return false;
        }
        out.flush();
        out.close();

        std::error_code ec;

        // Replace only after the temporary file was written successfully.
        ec.clear();
        filesystem::rename(tempFile, dataFile, ec);

        if (ec) {
            cerr << "Warning: could not replace the data file: "
                 << ec.message() << '\n';
            filesystem::remove(tempFile, ec);
            return false;
        }

        return true;
    }

    void loadFromFile() {
        ifstream in(dataFile);

        if (!in) {
            // First run: no file yet.
            return;
        }

        vector<Contact> loaded;
        string line;

        while (getline(in, line) && loaded.size() < MAX_CONTACTS) {
            if (line.empty() || line.size() > MAX_LINE_LEN) {
                continue;
            }

            string name, phone, email;
            stringstream ss(line);

            bool mismatch = false;

            if (!(ss >> quoted(name) >> quoted(phone) >> quoted(email) >> ws)) {
                continue;
            }

            string trailing;
            while (ss >> trailing) {
                // extra tokens mean the line had more than 3 fields; skip the line
                mismatch = true;
                break;
            }

            if (mismatch) {
                continue;
            }

            name = trim(name);
            phone = trim(phone);
            email = trim(email);

            if (!validName(name) || !validPhone(phone) || !validEmail(email)) {
                continue;
            }

            bool duplicate = false;

            for (const auto& c : loaded) {
                if (c.getPhone() == phone ||
                    toLowerCopy(c.getEmail()) == toLowerCopy(email)) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                loaded.emplace_back(name, phone, email);
            }
        }

        contacts = std::move(loaded);
    }

    int chooseMatch(const vector<size_t>& matches,
                    const string& actionName) const {
        if (matches.empty()) {
            cout << "Contact not found.\n";
            return -1;
        }

        if (matches.size() == 1) {
            return static_cast<int>(matches[0]);
        }

        cout << "\nMultiple contacts have that name:\n";

        for (size_t i = 0; i < matches.size(); ++i) {
            const Contact& c = contacts[matches[i]];
            cout << i + 1 << ". "
                 << c.getName() << " | "
                 << c.getPhone() << " | "
                 << c.getEmail() << '\n';
        }

        while (true) {
            string raw = readLine("Choose the contact to " + actionName +
                                  " (1-" + to_string(matches.size()) +
                                  ", or 0 to cancel): ");

            int choice;
            if (!parseMenuChoice(raw, choice) ||
                choice < 0 ||
                choice > static_cast<int>(matches.size())) {
                cout << "Invalid choice.\n";
                continue;
            }

            if (choice == 0) {
                return -1;
            }

            return static_cast<int>(matches[choice - 1]);
        }
    }

public:
    AddressBook() {
        contacts.reserve(MAX_CONTACTS);
        loadFromFile();
    }

    void addContact() {
        if (contacts.size() >= MAX_CONTACTS) {
            cout << "\nAddress Book is full.\n";
            return;
        }

        cout << "\n===== ADD CONTACT =====\n";

        string name = readValidName("Enter Name: ");
        string phone = readValidPhone("Enter Phone Number: ");
        string email = readValidEmail("Enter Email: ");

        contacts.emplace_back(name, phone, email);

        if (saveToFile()) {
            cout << "Contact added successfully.\n";
        } else {
            cout << "Contact was added in memory, but saving failed.\n";
        }
    }

    void displayContacts() const {
        cout << "\n===== ALL CONTACTS =====\n";

        if (contacts.empty()) {
            cout << "No contacts available.\n";
            return;
        }

        for (size_t i = 0; i < contacts.size(); ++i) {
            contacts[i].display(i + 1);
        }

        cout << "\nTotal contacts: " << contacts.size() << '\n';
    }

    void searchContact() const {
        if (contacts.empty()) {
            cout << "\nNo contacts available.\n";
            return;
        }

        cout << "\n===== SEARCH CONTACT =====\n";
        string name = readLine("Enter exact name to search: ");

        if (name.empty()) {
            cout << "Input stream closed. Exiting.\n";
            exit(0);
        }

        if (!validName(name)) {
            cout << "Invalid search name.\n";
            return;
        }

        vector<size_t> matches = findByName(name);

        if (matches.empty()) {
            cout << "Contact not found.\n";
            return;
        }

        cout << "\nFound " << matches.size() << " matching contact(s):\n";

        for (size_t index : matches) {
            contacts[index].display(index + 1);
        }
    }

    void updateContact() {
        if (contacts.empty()) {
            cout << "\nNo contacts available.\n";
            return;
        }

        cout << "\n===== UPDATE CONTACT =====\n";
        string name = readLine("Enter exact name to update: ");

        if (name.empty()) {
            cout << "Input stream closed. Exiting.\n";
            exit(0);
        }

        if (!validName(name)) {
            cout << "Invalid search name.\n";
            return;
        }

        vector<size_t> matches = findByName(name);
        int index = chooseMatch(matches, "update");

        if (index < 0) {
            return;
        }

        cout << "\nCurrent details:\n";
        contacts[index].display(static_cast<size_t>(index) + 1);

        cout << "\nEnter new details. Leave a field blank to keep the old value.\n";

        while (true) {
            string newName = readLine("New Name [" + contacts[index].getName() + "]: ");

            if (newName.empty()) {
                break;
            }

            if (validName(newName)) {
                contacts[index].setName(newName);
                break;
            }

            cout << "Invalid name.\n";
        }

        while (true) {
            string newPhone = readLine("New Phone [" + contacts[index].getPhone() + "]: ");

            if (newPhone.empty()) {
                break;
            }

            if (!validPhone(newPhone)) {
                cout << "Invalid phone. Enter 7-15 digits only.\n";
                continue;
            }

            if (phoneExists(newPhone, index)) {
                cout << "That phone number already exists.\n";
                continue;
            }

            contacts[index].setPhone(newPhone);
            break;
        }

        while (true) {
            string newEmail = readLine("New Email [" + contacts[index].getEmail() + "]: ");

            if (newEmail.empty()) {
                break;
            }

            if (!validEmail(newEmail)) {
                cout << "Invalid email address.\n";
                continue;
            }

            if (emailExists(newEmail, index)) {
                cout << "That email address already exists.\n";
                continue;
            }

            contacts[index].setEmail(newEmail);
            break;
        }

        if (saveToFile()) {
            cout << "Contact updated successfully.\n";
        } else {
            cout << "Contact was updated in memory, but saving failed.\n";
        }
    }

    void deleteContact() {
        if (contacts.empty()) {
            cout << "\nNo contacts available.\n";
            return;
        }

        cout << "\n===== DELETE CONTACT =====\n";
        string name = readLine("Enter exact name to delete: ");

        if (name.empty()) {
            cout << "Input stream closed. Exiting.\n";
            exit(0);
        }

        if (!validName(name)) {
            cout << "Invalid search name.\n";
            return;
        }

        vector<size_t> matches = findByName(name);
        int index = chooseMatch(matches, "delete");

        if (index < 0) {
            return;
        }

        cout << "\nSelected contact:\n";
        contacts[index].display(static_cast<size_t>(index) + 1);

        string confirmation = toLowerCopy(
            readLine("Type YES to confirm deletion: ")
        );

        if (confirmation != "yes") {
            cout << "Deletion cancelled.\n";
            return;
        }

        contacts.erase(contacts.begin() + index);

        if (saveToFile()) {
            cout << "Contact deleted successfully.\n";
        } else {
            cout << "Contact was deleted in memory, but saving failed.\n";
        }
    }

    void sortContacts() {
        if (contacts.size() < 2) {
            cout << "\nNot enough contacts to sort.\n";
            return;
        }

        stable_sort(contacts.begin(), contacts.end(),
                    [](const Contact& a, const Contact& b) {
                        return toLowerCopy(a.getName()) <
                               toLowerCopy(b.getName());
                    });

        if (saveToFile()) {
            cout << "\nContacts sorted alphabetically.\n";
        } else {
            cout << "\nContacts sorted in memory, but saving failed.\n";
        }
    }

    void showMenu() const {
        cout << "\n====================================\n";
        cout << "         ADDRESS BOOK SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Add Contact\n";
        cout << "2. Display All Contacts\n";
        cout << "3. Search Contact\n";
        cout << "4. Update Contact\n";
        cout << "5. Delete Contact\n";
        cout << "6. Sort Contacts by Name\n";
        cout << "7. Exit\n";
        cout << "====================================\n";
    }

    void run() {
        while (true) {
            showMenu();

            string rawChoice = readLine("Enter your choice: ");

            if (rawChoice.empty()) {
                cout << "\nInput stream closed. Exiting.\n";
                return;
            }

            int choice;
            if (!parseMenuChoice(rawChoice, choice)) {
                cout << "Invalid input. Enter a number from 1 to 7.\n";
                continue;
            }

            switch (choice) {
                case 1:
                    addContact();
                    break;
                case 2:
                    displayContacts();
                    break;
                case 3:
                    searchContact();
                    break;
                case 4:
                    updateContact();
                    break;
                case 5:
                    deleteContact();
                    break;
                case 6:
                    sortContacts();
                    break;
                case 7:
                    cout << "\nThank you for using Address Book System.\n";
                    return;
                default:
                    cout << "Invalid choice. Enter a number from 1 to 7.\n";
            }
        }
    }
};

int main() {
    try {
        AddressBook book;
        book.run();
    } catch (const exception& ex) {
        cerr << "\nUnexpected error: " << ex.what() << '\n';
        return 1;
    } catch (...) {
        cerr << "\nUnexpected unknown error.\n";
        return 1;
    }

    return 0;
}
