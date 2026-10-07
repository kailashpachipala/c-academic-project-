Address Book System — Final C++ OOP Project

Project

Project 6: Address Book System

Core requirements covered

Stores contacts

Add contact

Search contact

Delete contact

String handling

Menu-driven system

OOP concepts demonstrated

Classes and objects

Encapsulation

Abstraction

Constructors

Member functions

Array-like collection of objects (std::vector<Contact>)

Access specifiers

Object composition (AddressBook manages Contact objects)

Final features

Add Contact

Display All Contacts

Search Contact

Update Contact

Delete Contact with confirmation

Sort Contacts alphabetically

File persistence using contacts.db

Input validation

Duplicate phone/email prevention

Phone numbers must contain exactly 10 digits and start with 6, 7, 8, or 9

Contacts also store address, a six-digit pincode, alphabetic city, and a two-letter state short form such as AP

Graceful handling of malformed data file entries

Bounded number of contacts

Safer std::string/std::vector usage instead of raw character buffers

Temporary-file save strategy to reduce risk of corrupting the main data file

Compile

g++

g++ -std=c++17 -Wall -Wextra -pedantic AddressBookSystem_Final.cpp -o AddressBookSystem

Run

Windows:
AddressBookSystem.exe

Linux/macOS:
./AddressBookSystem

Important security note

No non-trivial program can be guaranteed to have “zero vulnerabilities.” This project is designed to avoid common beginner C++ problems such as unsafe C-style buffers, unchecked menu input, duplicate records, unbounded storage, malformed file input, and accidental deletions.

The local contacts.db file is not encrypted. Therefore, this academic project should not be used to store confidential or highly sensitive real-world contact data unless secure storage/encryption and operating-system access controls are added.
