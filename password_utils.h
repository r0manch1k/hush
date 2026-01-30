#ifndef PASSWORD_UTILS_H
#define PASSWORD_UTILS_H

#include <algorithm>
#include <random>
#include <string>

namespace password_utils {

// Генерация случайного пароля
inline std::string generate_password(int length = 16, bool use_upper = true, bool use_lower = true,
                                     bool use_digits = true, bool use_special = true) {
    std::string chars;
    if (use_lower) chars += "abcdefghijklmnopqrstuvwxyz";
    if (use_upper) chars += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (use_digits) chars += "0123456789";
    if (use_special) chars += "!@#$%^&*()-_=+[]{}|;:,.<>?";

    if (chars.empty()) chars = "abcdefghijklmnopqrstuvwxyz";

    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dis(0, chars.length() - 1);

    std::string password;
    password.reserve(length);
    for (int i = 0; i < length; ++i) {
        password += chars[dis(gen)];
    }

    return password;
}

// Оценка силы пароля (0-100)
inline int password_strength(const std::string& password) {
    if (password.empty()) return 0;

    int score = 0;

    // Длина
    if (password.length() >= 8) score += 20;
    if (password.length() >= 12) score += 10;
    if (password.length() >= 16) score += 10;

    // Разнообразие символов
    bool has_lower = false, has_upper = false, has_digit = false, has_special = false;
    for (char c : password) {
        if (islower(c))
            has_lower = true;
        else if (isupper(c))
            has_upper = true;
        else if (isdigit(c))
            has_digit = true;
        else
            has_special = true;
    }

    if (has_lower) score += 15;
    if (has_upper) score += 15;
    if (has_digit) score += 15;
    if (has_special) score += 15;

    return std::min(score, 100);
}

// Описание силы пароля
inline std::string password_strength_text(int strength) {
    if (strength < 40) return "Weak";
    if (strength < 70) return "Medium";
    return "Strong";
}

// Копирование в буфер обмена (macOS)
inline void copy_to_clipboard(const std::string& text) {
#ifdef __APPLE__
    std::string cmd = "echo '" + text + "' | pbcopy";
    system(cmd.c_str());
#endif
}

// Очистка буфера обмена (macOS)
inline void clear_clipboard() {
#ifdef __APPLE__
    system("echo '' | pbcopy");
#endif
}

}  // namespace password_utils

#endif
