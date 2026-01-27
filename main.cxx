#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/fl_ask.H>

#include <format>

#include "components/overlay/entry.h"
#include "components/overlay/overlay.h"
#include "database.h"
#include "icons/add.xpm"
#include "icons/delete.xpm"
#include "icons/edit.xpm"

using namespace std;

const int W = 480;
const int H = 320;

const char* const FN_DEFAULT = "keepit.hush";

enum class OverlayType { Main, Editor, Password, Confirm };

enum class EditMode { Add, Edit };

OverlayType global_overlay = OverlayType::Main;
EditMode    global_editor_mode;

string global_pass = "";

int global_edit_mode = -1;

Fl_Double_Window* global_main_win = nullptr;


Fl_Hold_Browser*  global_main_browser = nullptr;
Fl_Double_Window* global_editor_win   = nullptr;
Fl_Input*         global_title_input  = nullptr;
Fl_Input*         global_login_input  = nullptr;
Fl_Input*         global_search_input = nullptr;
Fl_Secret_Input*  global_pass_input   = nullptr;


// Нужна для формирования строки, отображаемой в списке паролей
string format_entry(const string title, const string login) {
    return title + "\t" + login;
}

void update_title() {
    if (!global_main_win) return;

    const char* label;

    if (!global_db_path.empty()) {
        size_t last_slash = global_db_path.find_last_of("/\\");
        string fn =
            (last_slash == string::npos) ? global_db_path : global_db_path.substr(last_slash + 1);
        label = format("Hush - {}", fn).c_str();
    } else {
        label = format("Hush - {}", FN_DEFAULT).c_str();
    }

    global_main_win->label(label);
}

void update_browser(const char* filter = nullptr) {
    if (!global_main_browser) return;

    global_main_browser->clear();

    string f = filter ? filter : "";

    for (const auto& entry : global_db_entries) {
        if (f.empty() || entry.title.find(f) != string::npos) {
            global_main_browser->add(format_entry(entry.title, entry.login).c_str());
        }
    }
}

void search_callback(Fl_Widget* i, void*) {
    update_browser(((Fl_Input*)i)->value());
}

void autosave() {
    if (!global_db_path.empty() && !global_pass.empty()) {
        db_save_file(global_db_path, global_pass);
    }
}

void w_save_as(Fl_Widget*, void*);

bool db_exists() {
    if (!global_db_path.empty()) return true;

    int choice = fl_choice("No database is open. Would you like to create a new one?", "Cancel",
                           "New", nullptr);

    if (choice == 1) {
        w_save_as(nullptr, nullptr);
        return !global_db_path.empty();
    }

    return false;
}


void w_open(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Open database", "*.hush", nullptr);

    if (!file) return;

    const char* p = fl_password("Password:", "");

    if (p && db_load_file(file, p)) {
        global_pass = p;
        update_browser();
        update_title();
    } else if (p) {
        fl_alert("Incorrect password or corrupted file.");
    }
}

void w_save_as(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Save database", "*.hush", FN_DEFAULT);

    if (!file) return;

    const char* p = fl_password("Set password:", "");

    if (!p) return;

    global_pass = p;
    db_save_file(file, p);
    update_title();
}

void add_callback(Fl_Widget*, void*) {
    if (!db_exists()) return;
    global_edit_mode = -1;
    global_title_input->value("");
    global_login_input->value("");
    global_pass_input->value("");
    global_editor_win->label("New Entry");
    global_editor_win->show();
}

void edit_callback(Fl_Widget*, void*) {
    if (!db_exists()) return;
    int v = global_main_browser->value();

    if (v <= 0) {
        fl_alert("Please select an entry to edit.");
        return;
    }

    global_edit_mode = v - 1;
    PasswordEntry& e = global_db_entries[global_edit_mode];
    global_title_input->value(e.title.c_str());
    global_login_input->value(e.login.c_str());
    global_pass_input->value(e.password.c_str());
    global_editor_win->label("Edit Entry");
    global_editor_win->show();
}

void delete_callback(Fl_Widget*, void*) {
    if (!db_exists()) return;

    int v = global_main_browser->value();

    if (v <= 0) {
        fl_alert("Error! Attempt to delete non-existing entry.");
        return;
    }

    if (fl_choice("Are you sure you want to delete this entry?", "Cancel", "Delete", nullptr) ==
        1) {
        global_db_entries.erase(global_db_entries.begin() + (v - 1));
        update_browser(global_search_input->value());
        autosave();
    }
}

void global_save_entry_callback(Fl_Widget*, void*) {
    if (strlen(global_title_input->value()) == 0) return;

    PasswordEntry entry = {global_title_input->value(), global_login_input->value(),
                           global_pass_input->value()};

    if (global_edit_mode >= 0) {
        global_db_entries[global_edit_mode] = entry;
    } else {
        global_db_entries.push_back(entry);
    }

    update_browser(global_search_input->value());
    global_editor_win->hide();
    autosave();
}

void w_about(Fl_Widget*, void*) {
    fl_message_title("About Hush");
    fl_message(
        "Hush Password Manager v1.0\n\n"
        "A simple, secure, and fast password vault.\n"
        "Built with FLTK.\n\n"
        "Made by Roman Sokolovsky, Ruslan Kutorgin.");
}

void w_quit(Fl_Widget*, void*) {
    exit(0);
}

int main(int argc, char** argv) {
    static Fl_Pixmap add_img(add_xpm), edit_img(edit_xpm), del_img(delete_xpm);

    // Инициализация всех накладываемых окон
    overlay::init(0, 0, W, H);

    global_main_win = new Fl_Double_Window(W, H, "Hush - no database");

    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, W, 25);
    menu->add("&Database/&Open      ", FL_META + 'o', w_open);
    menu->add("&Database/&Save As   ", FL_META + 's', w_save_as);
    menu->add("&Database/&Quit      ", FL_META + 'q', w_quit);
    menu->add("&Entry/&Add       ", FL_META + 'n', add_callback);
    menu->add("&Entry/&Edit      ", FL_META + 'e', edit_callback);
    menu->add("&Entry/&Delete    ", FL_META + FL_BackSpace, delete_callback);
    menu->add("&Help/&About", 0, w_about);

    Fl_Group* toolbar = new Fl_Group(0, 25, W, 25);

    Fl_Button* add_btn = new Fl_Button(0, 25, 25, 25);
    add_btn->image(add_img);
    add_btn->callback(add_callback);

    Fl_Button* edit_btn = new Fl_Button(25, 25, 25, 25);
    edit_btn->image(edit_img);
    edit_btn->callback(edit_callback);

    Fl_Button* del_btn = new Fl_Button(2 * 25, 25, 25, 25);
    del_btn->image(del_img);
    del_btn->callback(delete_callback);

    global_search_input = new Fl_Input(3 * 25, 25, W - 3 * 25, 25);
    global_search_input->callback(search_callback);
    global_search_input->when(FL_WHEN_CHANGED);
    toolbar->end();

    global_main_browser = new Fl_Hold_Browser(0, 2 * 25, W, H - 2 * 25);
    static int widths[] = {200, 200, 0};
    global_main_browser->column_widths(widths);
    global_main_browser->column_char('\t');

    global_main_win->resizable(global_main_browser);
    global_main_win->end();
    global_main_win->show(argc, argv);

    return Fl::run();
}