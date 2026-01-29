#include "database.h"

#include <advobfuscator/string.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef __APPLE__
#include <pwd.h>
#include <unistd.h>
#endif

using namespace std;
using namespace andrivet::advobfuscator;

vector<PasswordEntry> g_passwordEntries;
string                g_currentDatabasePath = "";

static constexpr auto   MAGIC_HEADER = "HUSH"_obf;
static constexpr size_t SALT_SIZE    = 16;
static constexpr size_t KEY_SIZE     = 32;

void derive_key_simple(const string& password, const uint8_t* salt, size_t saltLen, uint8_t* key,
                       size_t keyLen) {
    vector<uint8_t> temp;
    temp.insert(temp.end(), salt, salt + saltLen);
    temp.insert(temp.end(), password.begin(), password.end());

    for (size_t i = 0; i < keyLen; ++i) {
        uint32_t hash = 0x12345678;
        for (size_t j = 0; j < temp.size(); ++j) {
            hash = hash * 31 + temp[j] + i;
        }
        key[i] = static_cast<uint8_t>(hash ^ (hash >> 8) ^ (hash >> 16) ^ (hash >> 24));
    }
}

void xor_cipher(uint8_t* data, size_t len, const uint8_t* key, size_t keyLen) {
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= key[i % keyLen];
    }
}

string encrypt_data(const string& plaintext, const string& masterPassword) {
    if (plaintext.empty() || masterPassword.empty()) return "";

    uint8_t  salt[SALT_SIZE];
    uint32_t seed = static_cast<uint32_t>(plaintext.length() ^ masterPassword.length());
    for (size_t i = 0; i < SALT_SIZE; ++i) {
        seed    = seed * 1103515245 + 12345;
        salt[i] = static_cast<uint8_t>((seed >> 16) & 0xFF);
    }

    uint8_t key[KEY_SIZE];
    derive_key_simple(masterPassword, salt, SALT_SIZE, key, KEY_SIZE);

    vector<uint8_t> encrypted(plaintext.begin(), plaintext.end());

    for (int pass = 0; pass < 3; ++pass) {
        xor_cipher(encrypted.data(), encrypted.size(), key, KEY_SIZE);
        for (size_t i = 0; i < KEY_SIZE; ++i) {
            key[i] = key[i] * 31 + salt[i % SALT_SIZE];
        }
    }

    string result;
    result.append((char*)salt, SALT_SIZE);
    result.append((char*)encrypted.data(), encrypted.size());

    return result;
}

string decrypt_data(const string& encrypted, const string& masterPassword) {
    if (encrypted.empty() || masterPassword.empty()) return "";
    if (encrypted.size() < SALT_SIZE) return "";

    uint8_t salt[SALT_SIZE];
    memcpy(salt, encrypted.data(), SALT_SIZE);

    uint8_t key[KEY_SIZE];
    derive_key_simple(masterPassword, salt, SALT_SIZE, key, KEY_SIZE);

    string          ciphertext = encrypted.substr(SALT_SIZE);
    vector<uint8_t> decrypted(ciphertext.begin(), ciphertext.end());

    uint8_t keys[3][KEY_SIZE];
    memcpy(keys[0], key, KEY_SIZE);
    for (int pass = 1; pass < 3; ++pass) {
        memcpy(keys[pass], keys[pass - 1], KEY_SIZE);
        for (size_t i = 0; i < KEY_SIZE; ++i) {
            keys[pass][i] = keys[pass][i] * 31 + salt[i % SALT_SIZE];
        }
    }

    for (int pass = 2; pass >= 0; --pass) {
        xor_cipher(decrypted.data(), decrypted.size(), keys[pass], KEY_SIZE);
    }

    return string(decrypted.begin(), decrypted.end());
}

static string get_config_path() {
#ifdef __APPLE__
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home              = pw->pw_dir;
    }
    return string(home) + "/.hush_config";
#else
    return ".hush_config";
#endif
}

string get_last_db_path() {
    ifstream config(get_config_path());
    if (!config) return "";

    string path;
    getline(config, path);
    config.close();

    ifstream test(path);
    if (!test) return "";
    test.close();

    return path;
}

void save_last_db_path(const string& path) {
    if (path.empty()) return;

    ofstream config(get_config_path());
    if (config) {
        config << path;
        config.close();
    }
}

void clear_last_db_path() {
    remove(get_config_path().c_str());
}

bool db_load_file(const string& filepath, const string& masterPassword) {
    ifstream is(filepath, ios::binary);
    if (!is) return false;

    char header[5] = {0};
    is.read(header, 4);

    auto expected = MAGIC_HEADER;
    if (string(header) != string(expected)) {
        return false;
    }

    string encryptedData((istreambuf_iterator<char>(is)), istreambuf_iterator<char>());
    is.close();

    if (encryptedData.empty()) return false;

    string decrypted = decrypt_data(encryptedData, masterPassword);
    if (decrypted.empty()) return false;

    g_passwordEntries.clear();
    size_t pos = 0;

    auto read_string = [&](string& s) -> bool {
        if (pos + sizeof(size_t) > decrypted.size()) return false;
        size_t len;
        memcpy(&len, decrypted.data() + pos, sizeof(len));
        pos += sizeof(len);

        if (pos + len > decrypted.size()) return false;
        s = decrypted.substr(pos, len);
        pos += len;
        return true;
    };

    size_t count;
    if (pos + sizeof(count) > decrypted.size()) return false;
    memcpy(&count, decrypted.data() + pos, sizeof(count));
    pos += sizeof(count);

    for (size_t i = 0; i < count; ++i) {
        PasswordEntry entry;
        if (!read_string(entry.title) || !read_string(entry.login) ||
            !read_string(entry.password)) {
            return false;
        }
        if (pos + sizeof(bool) <= decrypted.size()) {
            memcpy(&entry.is_favorite, decrypted.data() + pos, sizeof(bool));
            pos += sizeof(bool);
        }
        g_passwordEntries.push_back(entry);
    }

    g_currentDatabasePath = filepath;
    save_last_db_path(filepath);
    return true;
}

bool db_save_file(const string& filepath, const string& masterPassword) {
    if (filepath.empty() || masterPassword.empty()) return false;

    string plaintext;

    auto write_string = [&](const string& s) {
        size_t len = s.length();
        plaintext.append((char*)&len, sizeof(len));
        plaintext.append(s);
    };

    size_t count = g_passwordEntries.size();
    plaintext.append((char*)&count, sizeof(count));

    for (const auto& entry : g_passwordEntries) {
        write_string(entry.title);
        write_string(entry.login);
        write_string(entry.password);
        plaintext.append((char*)&entry.is_favorite, sizeof(bool));
    }

    string encrypted = encrypt_data(plaintext, masterPassword);
    if (encrypted.empty()) return false;

    ofstream os(filepath, ios::binary);
    if (!os) return false;

    auto   header = MAGIC_HEADER;
    string headerStr(header);
    os.write(headerStr.c_str(), 4);
    os.write(encrypted.data(), encrypted.size());
    os.close();

    g_currentDatabasePath = filepath;
    save_last_db_path(filepath);
    return true;
}