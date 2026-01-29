#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

using namespace std;

struct PasswordEntry {
    std::string title;
    std::string login;
    std::string password;
};

// extern говорит: "эта штука есть где-то в другом месте"
// extern std::vector<PasswordEntry> global_db_entries;
// extern std::string                global_db_path;

bool db_load_file(vector<PasswordEntry>* entries, string* path, const std::string& filepath,
                  const std::string& master_pass);
bool db_save_file(vector<PasswordEntry>* entries, string* path, const std::string& filepath,
                  const std::string& master_pass);

#endif