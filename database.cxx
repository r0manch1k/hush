#include "database.h"

#include <fstream>

using namespace std;

vector<PasswordEntry> global_db_entries;
string                global_db_path = "";

bool db_load_file(const string& filepath, const string& master_pass) {
    // ВАЖНО: Тут вы добавите свою дешифровку
    if (master_pass != "1234") return false;

    ifstream is(filepath, ios::binary);
    if (!is) return false;

    global_db_entries.clear();
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
        global_db_entries.push_back(e);
    }

    global_db_path = filepath;
    return true;
}

bool db_save_file(const std::string& filepath, const std::string& master_pass) {
    if (filepath.empty() || master_pass.empty()) return false;

    // ВАЖНО: Тут вы добавите свое шифрование перед записью
    std::ofstream os(filepath, std::ios::binary);
    if (!os) return false;

    size_t count = global_db_entries.size();
    os.write((char*)&count, sizeof(count));

    for (const auto& e : global_db_entries) {
        auto write_str = [&](const std::string& s) {
            size_t len = s.length();
            os.write((char*)&len, sizeof(len));
            os.write(s.data(), len);
        };
        write_str(e.title);
        write_str(e.login);
        write_str(e.password);
    }

    global_db_path = filepath;
    return true;
}