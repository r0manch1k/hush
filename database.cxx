#include "database.h"

#include <advobfuscator/aes.h>
#include <advobfuscator/aes_string.h>
#include <advobfuscator/bytes.h>
#include <advobfuscator/string.h>

#include <fstream>
#include <iostream>

using namespace std;
using namespace andrivet::advobfuscator;

// vector<PasswordEntry> global_db_entries;
// string                global_db_path = "";

bool db_load_file(vector<PasswordEntry>* entries, string* path, const string& filepath,
                  const string& master_pass) {
    // Использовать обфускацию при необходимости (см. external/advobfuscator/Examples/demo)
    cout << "abc"_obf << '\n';

    // ВАЖНО: Тут вы добавите свою дешифровку
    if (master_pass != "1234") return false;

    ifstream is(filepath, ios::binary);
    if (!is) return false;

    entries->clear();
    size_t count;
    is.read((char*)&count, sizeof(count));

    for (size_t i = 0; i < count; ++i) {
        auto read_str = [&](std::string& s) {
            size_t len;
            is.read((char*)&len, sizeof(len));
            s.resize(len);
            is.read(&s[0], len);
        };
        PasswordEntry e;
        read_str(e.title);
        read_str(e.login);
        read_str(e.password);
        entries->push_back(e);
    }

    *path = filepath;
    return true;
}

bool db_save_file(vector<PasswordEntry>* entries, string* path, const std::string& filepath,
                  const std::string& master_pass) {
    if (filepath.empty() || master_pass.empty()) return false;

    // ВАЖНО: Тут вы добавите свое шифрование перед записью
    std::ofstream os(filepath, std::ios::binary);
    if (!os) return false;

    size_t count = entries->size();
    os.write((char*)&count, sizeof(count));

    for (const auto& e : *entries) {
        auto write_str = [&](const std::string& s) {
            size_t len = s.length();
            os.write((char*)&len, sizeof(len));
            os.write(s.data(), len);
        };
        write_str(e.title);
        write_str(e.login);
        write_str(e.password);
    }

    *path = filepath;
    return true;
}