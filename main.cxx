#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/fl_ask.H>

#include <atomic>
#include <chrono>
#include <format>
#include <fstream>
#include <thread>

#include "database.h"
#include "icons/add.xpm"
#include "icons/delete.xpm"
#include "icons/edit.xpm"
#include "password_utils.h"

using namespace std;

const char* const DEFAULT_DB_NAME       = "keepit.hush";
const int         CLIPBOARD_TIMEOUT_SEC = 10;

// UI Components
Fl_Double_Window* mainWindow          = nullptr;
Fl_Hold_Browser*  entriesBrowser      = nullptr;
Fl_Double_Window* editorWindow        = nullptr;
Fl_Input*         titleInput          = nullptr;
Fl_Input*         loginInput          = nullptr;
Fl_Input*         searchInput         = nullptr;
Fl_Secret_Input*  passwordInput       = nullptr;
Fl_Button*        showPasswordBtn     = nullptr;
Fl_Button*        generatePasswordBtn = nullptr;
Fl_Button*        copyPasswordBtn     = nullptr;
Fl_Box*           strengthIndicator   = nullptr;
Fl_Check_Button*  favoriteCheckbox    = nullptr;
Fl_Box*           clipboardTimerLabel = nullptr;

// Application State
string g_masterPassword    = "";
bool   g_passwordVisible   = false;
int    g_editingEntryIndex = -1;

std::atomic<bool> g_clipboardTimerActive{false};
std::atomic<int>  g_clipboardSecondsLeft{0};

// Forward declarations
void updateTitle();
void updatePasswordStrength();
void updateClipboardTimer(void*);
void startClipboardTimer();
void togglePasswordVisibility(Fl_Widget*, void*);
void generateNewPassword(Fl_Widget*, void*);
void copyPasswordFromEditor(Fl_Widget*, void*);
void copyPasswordFromBrowser(Fl_Widget*, void*);
void saveDatabase(Fl_Widget*, void*);
void createNewDatabase(Fl_Widget*, void*);
void addEntry(Fl_Widget*, void*);
void editEntry(Fl_Widget*, void*);
void deleteEntry(Fl_Widget*, void*);
void saveEntry(Fl_Widget*, void*);
void showAbout(Fl_Widget*, void*);
void exitApplication(Fl_Widget*, void*);
void openDatabase(Fl_Widget*, void*);
void tryOpenLastDatabase();
int  findActualIndex(int displayIndex);

string formatEntry(const string title, const string login) {
    return format("  {}  │  {}", title, login);
}

void updateTitle() {
    if (!mainWindow) return;

    string label;
    if (!g_currentDatabasePath.empty()) {
        size_t lastSlash = g_currentDatabasePath.find_last_of("/\\");
        string filename  = (lastSlash == string::npos) ? g_currentDatabasePath
                                                       : g_currentDatabasePath.substr(lastSlash + 1);
        label            = format("Hush - {}", filename);
    } else {
        label = format("Hush - {}", DEFAULT_DB_NAME);
    }
    mainWindow->copy_label(label.c_str());
}

void updateBrowser(const char* filter = nullptr) {
    if (!entriesBrowser) return;

    entriesBrowser->clear();
    string filterText = filter ? filter : "";

    // Add favorites first
    for (const auto& entry : g_passwordEntries) {
        if (entry.is_favorite &&
            (filterText.empty() || entry.title.find(filterText) != string::npos)) {
            string display = format("* {}", entry.title);
            if (!entry.login.empty()) {
                display += format("  {}", entry.login);
            }
            entriesBrowser->add(display.c_str());
        }
    }

    // Add regular entries
    for (const auto& entry : g_passwordEntries) {
        if (!entry.is_favorite &&
            (filterText.empty() || entry.title.find(filterText) != string::npos)) {
            string display = format("  {}", entry.title);
            if (!entry.login.empty()) {
                display += format("  {}", entry.login);
            }
            entriesBrowser->add(display.c_str());
        }
    }
}

void search(Fl_Widget* widget, void*) {
    updateBrowser(((Fl_Input*)widget)->value());
}

void autosave() {
    if (!g_currentDatabasePath.empty() && !g_masterPassword.empty()) {
        db_save_file(g_currentDatabasePath, g_masterPassword);
    }
}

void saveDatabase(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Save database", "*.hush", DEFAULT_DB_NAME);
    if (!file) return;

    string        filepath = file;
    std::ifstream test(filepath);
    if (test) {
        test.close();
        int choice = fl_choice("File already exists. Do you want to overwrite it?", "Cancel",
                               "Overwrite", nullptr);
        if (choice != 1) return;
    }

    const char* password = fl_password("Set password:", "");
    if (!password || strlen(password) == 0) {
        fl_alert("Password cannot be empty.");
        return;
    }

    g_masterPassword = password;
    if (db_save_file(file, password)) {
        updateTitle();
    } else {
        fl_alert("Failed to save database.");
    }
}

bool databaseExists() {
    if (!g_currentDatabasePath.empty()) return true;

    int choice = fl_choice("No database is open. Would you like to create a new one?", "Cancel",
                           "New", nullptr);
    if (choice == 1) {
        saveDatabase(nullptr, nullptr);
        return !g_currentDatabasePath.empty();
    }
    return false;
}

void openDatabase(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Open database", "*.hush", nullptr);
    if (!file) return;

    std::ifstream test(file);
    if (!test) {
        fl_alert("File does not exist.");
        return;
    }
    test.close();

    while (true) {
        const char* passwordPtr = fl_password("Password:", "");
        if (!passwordPtr) return;

        string password = passwordPtr;

        if (db_load_file(file, password)) {
            g_masterPassword = password;
            updateBrowser();
            updateTitle();
            return;
        } else {
            int choice = fl_choice("Incorrect password. Try again?", "Cancel", "Retry", nullptr);
            if (choice != 1) return;
        }
    }
}

void tryOpenLastDatabase() {
    string lastDb = get_last_db_path();
    if (lastDb.empty()) return;

    size_t lastSlash = lastDb.find_last_of("/\\");
    string filename  = (lastSlash == string::npos) ? lastDb : lastDb.substr(lastSlash + 1);
    string prompt    = format("Open last database '{}'?", filename);

    int choice = fl_choice("%s", "Cancel", "Open", "New", prompt.c_str());

    if (choice == 1) {
        while (true) {
            const char* passwordPtr = fl_password("Password:", "");
            if (!passwordPtr) return;

            string password = passwordPtr;

            if (db_load_file(lastDb, password)) {
                g_masterPassword = password;
                updateBrowser();
                updateTitle();
                return;
            } else {
                int retryChoice =
                    fl_choice("Incorrect password. Try again?", "Cancel", "Retry", nullptr);
                if (retryChoice != 1) {
                    clear_last_db_path();
                    return;
                }
            }
        }
    } else if (choice == 2) {
        saveDatabase(nullptr, nullptr);
    }
}

void updatePasswordStrength() {
    if (!passwordInput || !strengthIndicator) return;

    string password     = passwordInput->value();
    int    strength     = password_utils::password_strength(password);
    string strengthText = password_utils::password_strength_text(strength);
    string label        = format("Strength: {} ({}%)", strengthText, strength);
    strengthIndicator->copy_label(label.c_str());

    if (strength < 40) {
        strengthIndicator->labelcolor(FL_RED);
    } else if (strength < 70) {
        strengthIndicator->labelcolor(FL_DARK_YELLOW);
    } else {
        strengthIndicator->labelcolor(FL_DARK_GREEN);
    }
    strengthIndicator->redraw();
}

void updateClipboardTimer(void*) {
    if (!clipboardTimerLabel) return;

    int seconds = g_clipboardSecondsLeft.load();
    if (seconds > 0) {
        string label = format("Clipboard clears in {}s", seconds);
        clipboardTimerLabel->copy_label(label.c_str());
        clipboardTimerLabel->show();
        clipboardTimerLabel->redraw();
    } else {
        clipboardTimerLabel->hide();
        g_clipboardTimerActive = false;
    }

    if (g_clipboardTimerActive) {
        Fl::repeat_timeout(1.0, updateClipboardTimer);
    }
}

void startClipboardTimer() {
    g_clipboardSecondsLeft = CLIPBOARD_TIMEOUT_SEC;
    g_clipboardTimerActive = true;

    if (clipboardTimerLabel) {
        string label = format("Clipboard clears in {}s", CLIPBOARD_TIMEOUT_SEC);
        clipboardTimerLabel->copy_label(label.c_str());
        clipboardTimerLabel->show();
        clipboardTimerLabel->redraw();
    }

    Fl::add_timeout(1.0, updateClipboardTimer);

    thread([]() {
        while (g_clipboardSecondsLeft > 0 && g_clipboardTimerActive) {
            this_thread::sleep_for(chrono::seconds(1));
            g_clipboardSecondsLeft--;
        }

        if (g_clipboardTimerActive) {
            password_utils::clear_clipboard();
            g_clipboardTimerActive = false;
            Fl::awake(
                [](void*) {
                    if (clipboardTimerLabel) clipboardTimerLabel->hide();
                },
                nullptr);
        }
    }).detach();
}

void togglePasswordVisibility(Fl_Widget*, void*) {
    if (!passwordInput) return;
    g_passwordVisible = !g_passwordVisible;
    showPasswordBtn->label(g_passwordVisible ? "Hide" : "Show");
    showPasswordBtn->redraw();
}

void generateNewPassword(Fl_Widget*, void*) {
    string newPassword = password_utils::generate_password(16, true, true, true, true);
    passwordInput->value(newPassword.c_str());
    updatePasswordStrength();
}

void copyPasswordFromEditor(Fl_Widget*, void*) {
    if (!passwordInput) return;
    string password = passwordInput->value();
    if (password.empty()) {
        fl_alert("No password to copy.");
        return;
    }
    password_utils::copy_to_clipboard(password);
    startClipboardTimer();
}

void copyPasswordFromBrowser(Fl_Widget*, void*) {
    int displayIndex = entriesBrowser->value();
    if (displayIndex <= 0) {
        fl_alert("Please select an entry.");
        return;
    }

    int actualIndex = findActualIndex(displayIndex);
    if (actualIndex >= 0) {
        password_utils::copy_to_clipboard(g_passwordEntries[actualIndex].password);
        startClipboardTimer();
    }
}

int findActualIndex(int displayIndex) {
    int actualIndex    = 0;
    int currentDisplay = 1;

    // Search in favorites
    for (size_t i = 0; i < g_passwordEntries.size(); ++i) {
        if (g_passwordEntries[i].is_favorite) {
            if (currentDisplay == displayIndex) return i;
            currentDisplay++;
        }
    }

    // Search in regular entries
    for (size_t i = 0; i < g_passwordEntries.size(); ++i) {
        if (!g_passwordEntries[i].is_favorite) {
            if (currentDisplay == displayIndex) return i;
            currentDisplay++;
        }
    }

    return -1;
}

void createNewDatabase(Fl_Widget*, void*) {
    if (!g_currentDatabasePath.empty()) {
        int choice =
            fl_choice("Current database will be closed. Continue?", "Cancel", "Continue", nullptr);
        if (choice != 1) return;
    }

    g_passwordEntries.clear();
    g_currentDatabasePath = "";
    g_masterPassword      = "";
    updateBrowser();
    saveDatabase(nullptr, nullptr);
}

void addEntry(Fl_Widget*, void*) {
    if (!databaseExists()) return;
    g_editingEntryIndex = -1;
    titleInput->value("");
    loginInput->value("");
    passwordInput->value("");
    editorWindow->label("New Entry");
    editorWindow->show();
}

void editEntry(Fl_Widget*, void*) {
    if (!databaseExists()) return;
    int displayIndex = entriesBrowser->value();
    if (displayIndex <= 0) {
        fl_alert("Please select an entry to edit.");
        return;
    }

    int actualIndex = findActualIndex(displayIndex);
    if (actualIndex < 0) return;

    g_editingEntryIndex  = actualIndex;
    PasswordEntry& entry = g_passwordEntries[actualIndex];
    titleInput->value(entry.title.c_str());
    loginInput->value(entry.login.c_str());
    passwordInput->value(entry.password.c_str());
    favoriteCheckbox->value(entry.is_favorite ? 1 : 0);
    updatePasswordStrength();
    editorWindow->label("Edit Entry");
    editorWindow->show();
}

void deleteEntry(Fl_Widget*, void*) {
    if (!databaseExists()) return;
    int displayIndex = entriesBrowser->value();
    if (displayIndex <= 0) {
        fl_alert("Error! Attempt to delete non-existing entry.");
        return;
    }

    if (fl_choice("Are you sure you want to delete this entry?", "Cancel", "Delete", nullptr) ==
        1) {
        int actualIndex = findActualIndex(displayIndex);
        if (actualIndex >= 0) {
            g_passwordEntries.erase(g_passwordEntries.begin() + actualIndex);
            updateBrowser(searchInput->value());
            autosave();
        }
    }
}

void saveEntry(Fl_Widget*, void*) {
    if (strlen(titleInput->value()) == 0) return;

    PasswordEntry entry = {titleInput->value(), loginInput->value(), passwordInput->value(),
                           favoriteCheckbox->value() == 1};

    if (g_editingEntryIndex >= 0) {
        g_passwordEntries[g_editingEntryIndex] = entry;
    } else {
        g_passwordEntries.push_back(entry);
    }

    updateBrowser(searchInput->value());
    editorWindow->hide();
    autosave();
}

void showAbout(Fl_Widget*, void*) {
    fl_message_title("About Hush");
    fl_message(
        "Hush Password Manager v1.0\n\n"
        "A simple, secure, and fast password vault.\n"
        "Built with FLTK.\n\n"
        "Made by Roman Sokolovsky, Ruslan Kutorgin.");
}

void exitApplication(Fl_Widget*, void*) {
    g_clipboardTimerActive = false;
    password_utils::clear_clipboard();
    exit(0);
}

int main(int argc, char** argv) {
    static Fl_Pixmap imgAdd(add_xpm), imgEdit(edit_xpm), imgDelete(delete_xpm);

    editorWindow  = new Fl_Double_Window(340, 210, "New Entry");
    titleInput    = new Fl_Input(70, 10, 260, 25, "Title:");
    loginInput    = new Fl_Input(70, 40, 260, 25, "User:");
    passwordInput = new Fl_Secret_Input(70, 70, 180, 25, "Pass:");
    passwordInput->callback([](Fl_Widget*, void*) { updatePasswordStrength(); });

    showPasswordBtn = new Fl_Button(255, 70, 75, 25, "Show");
    showPasswordBtn->callback(togglePasswordVisibility);

    generatePasswordBtn = new Fl_Button(70, 100, 85, 25, "Generate");
    generatePasswordBtn->callback(generateNewPassword);

    copyPasswordBtn = new Fl_Button(160, 100, 85, 25, "Copy");
    copyPasswordBtn->callback(copyPasswordFromEditor);

    strengthIndicator = new Fl_Box(250, 100, 80, 25, "");
    strengthIndicator->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    strengthIndicator->labelfont(FL_BOLD);

    favoriteCheckbox = new Fl_Check_Button(70, 130, 100, 25, "Favorite");

    Fl_Button* okBtn = new Fl_Button(245, 175, 85, 25, "Save");
    okBtn->callback(saveEntry);

    editorWindow->end();
    editorWindow->set_modal();

    mainWindow = new Fl_Double_Window(480, 320, "Hush - no database");

    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 480, 25);
    menu->add("&Database/&New       ", FL_META + 'n', createNewDatabase);
    menu->add("&Database/&Open      ", FL_META + 'o', openDatabase);
    menu->add("&Database/&Save As   ", FL_META + 's', saveDatabase);
    menu->add("&Database/&Quit      ", FL_META + 'q', exitApplication);
    menu->add("&Entry/&Add       ", FL_META + FL_SHIFT + 'n', addEntry);
    menu->add("&Entry/&Edit      ", FL_META + 'e', editEntry);
    menu->add("&Entry/Copy Password", FL_META + 'c', copyPasswordFromBrowser);
    menu->add("&Entry/&Delete    ", FL_META + FL_BackSpace, deleteEntry);
    menu->add("&Help/&About", 0, showAbout);

    Fl_Group*  toolbar = new Fl_Group(0, 25, 480, 25);
    Fl_Button* btnAdd  = new Fl_Button(0, 25, 25, 25);
    btnAdd->image(imgAdd);
    btnAdd->callback(addEntry);
    Fl_Button* btnEdit = new Fl_Button(25, 25, 25, 25);
    btnEdit->image(imgEdit);
    btnEdit->callback(editEntry);
    Fl_Button* btnDelete = new Fl_Button(50, 25, 25, 25);
    btnDelete->image(imgDelete);
    btnDelete->callback(deleteEntry);

    searchInput = new Fl_Input(75, 25, 405, 25);
    searchInput->callback(search);
    searchInput->when(FL_WHEN_CHANGED);
    toolbar->end();

    entriesBrowser      = new Fl_Hold_Browser(0, 50, 480, 245);
    static int widths[] = {250, 0};
    entriesBrowser->column_widths(widths);

    clipboardTimerLabel = new Fl_Box(0, 295, 480, 25, "");
    clipboardTimerLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
    clipboardTimerLabel->labelsize(11);
    clipboardTimerLabel->labelcolor(FL_BLACK);
    clipboardTimerLabel->box(FL_FLAT_BOX);
    clipboardTimerLabel->color(FL_BACKGROUND_COLOR);
    clipboardTimerLabel->hide();

    mainWindow->resizable(entriesBrowser);
    mainWindow->end();
    mainWindow->show(argc, argv);

    tryOpenLastDatabase();

    return Fl::run();
}