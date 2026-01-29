#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

struct PasswordEntry {
    std::string title;
    std::string login;
    std::string password;
    bool        is_favorite = false;
};

extern std::vector<PasswordEntry> g_passwordEntries;
extern std::string                g_currentDatabasePath;

bool db_load_file(const std::string& filepath, const std::string& masterPassword);
bool db_save_file(const std::string& filepath, const std::string& masterPassword);

std::string get_last_db_path();
void        save_last_db_path(const std::string& path);
void        clear_last_db_path();

#endif