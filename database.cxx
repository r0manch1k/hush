#include "database.h"
#include <fstream>

std::vector<PasswordEntry> db_entries;
std::string current_db_path = "";

bool db_load_file(const std::string& filepath, const std::string& master_pass) {
    // ВАЖНО: Тут вы добавите свою дешифровку
    if (master_pass != "1234") return false; 

    std::ifstream is(filepath, std::ios::binary);
    if (!is) return false;

    db_entries.clear();
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
        read_str(e.username);
        read_str(e.password);
        db_entries.push_back(e);
    }
    
    current_db_path = filepath;
    return true;
}

bool db_save_file(const std::string& filepath, const std::string& master_pass) {
    if (filepath.empty() || master_pass.empty()) return false;

    // ВАЖНО: Тут вы добавите свое шифрование перед записью
    std::ofstream os(filepath, std::ios::binary);
    if (!os) return false;

    size_t count = db_entries.size();
    os.write((char*)&count, sizeof(count));

    for (const auto& e : db_entries) {
        auto write_str = [&](const std::string& s) {
            size_t len = s.length();
            os.write((char*)&len, sizeof(len));
            os.write(s.data(), len);
        };
        write_str(e.title);
        write_str(e.username);
        write_str(e.password);
    }

    current_db_path = filepath;
    return true;
}