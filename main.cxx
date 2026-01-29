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

#include "database.h"
#include "icons/add.xpm"
#include "icons/delete.xpm"
#include "icons/edit.xpm"

using namespace std;

// Название базы данных по умолчанию
const char* const FN_DEFAULT = "keepit.hush";

// Префикс w_ значит, что моя переменная (функция) связана с графическим интерфейсом
Fl_Double_Window* w_main_win     = nullptr;
Fl_Hold_Browser*  w_main_browser = nullptr;
Fl_Double_Window* w_editor_win   = nullptr;
Fl_Input *        w_title_input = nullptr, *w_login_input = nullptr, *w_search_input = nullptr;
Fl_Secret_Input*  w_pass_input = nullptr;

string global_pass = "";

int global_edit_mode = -1;

// Нужна для формирования строки, отображаемой в списке паролей
string format_entry(const string title, const string login) {
    return title + "\t" + login;
}

void update_title() {
    if (!w_main_win) return;

    const char* label;

    if (!global_db_path.empty()) {
        size_t last_slash = global_db_path.find_last_of("/\\");
        string fn =
            (last_slash == string::npos) ? global_db_path : global_db_path.substr(last_slash + 1);
        label = format("Hush - {}", fn).c_str();
    } else {
        label = format("Hush - {}", FN_DEFAULT).c_str();
    }

    w_main_win->label(label);
}

void update_browser(const char* filter = nullptr) {
    if (!w_main_browser) return;

    w_main_browser->clear();

    string f = filter ? filter : "";

    for (const auto& entry : global_db_entries) {
        if (f.empty() || entry.title.find(f) != string::npos) {
            w_main_browser->add(format_entry(entry.title, entry.login).c_str());
        }
    }
}

void search(Fl_Widget* i, void*) {
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

void w_add(Fl_Widget*, void*) {
    if (!db_exists()) return;
    global_edit_mode = -1;
    w_title_input->value("");
    w_login_input->value("");
    w_pass_input->value("");
    w_editor_win->label("New Entry");
    w_editor_win->show();
}

void w_edit(Fl_Widget*, void*) {
    if (!db_exists()) return;
    int v = w_main_browser->value();

    if (v <= 0) {
        fl_alert("Please select an entry to edit.");
        return;
    }

    global_edit_mode = v - 1;
    PasswordEntry& e = global_db_entries[global_edit_mode];
    w_title_input->value(e.title.c_str());
    w_login_input->value(e.login.c_str());
    w_pass_input->value(e.password.c_str());
    w_editor_win->label("Edit Entry");
    w_editor_win->show();
}

void w_delete(Fl_Widget*, void*) {
    if (!db_exists()) return;

    int v = w_main_browser->value();

    if (v <= 0) {
        fl_alert("Error! Attempt to delete non-existing entry.");
        return;
    }

    if (fl_choice("Are you sure you want to delete this entry?", "Cancel", "Delete", nullptr) ==
        1) {
        global_db_entries.erase(global_db_entries.begin() + (v - 1));
        update_browser(w_search_input->value());
        autosave();
    }
}

void w_save_entry(Fl_Widget*, void*) {
    if (strlen(w_title_input->value()) == 0) return;

    PasswordEntry entry = {w_title_input->value(), w_login_input->value(), w_pass_input->value()};

    if (global_edit_mode >= 0) {
        global_db_entries[global_edit_mode] = entry;
    } else {
        global_db_entries.push_back(entry);
    }

    update_browser(w_search_input->value());
    w_editor_win->hide();
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
    static Fl_Pixmap img_add(add_xpm), img_edit(edit_xpm), img_del(delete_xpm);

    w_editor_win    = new Fl_Double_Window(240, 140, "New Entry");
    w_title_input   = new Fl_Input(45, 10, 180, 25, "Title:");
    w_login_input   = new Fl_Input(45, 40, 180, 25, "User:");
    w_pass_input    = new Fl_Secret_Input(45, 70, 180, 25, "Pass:");
    Fl_Button* b_ok = new Fl_Button(145, 105, 80, 25, "Save");
    b_ok->callback(w_save_entry);
    w_editor_win->end();
    w_editor_win->set_modal();

    w_main_win = new Fl_Double_Window(480, 320, "Hush - no database");

    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 480, 25);
    menu->add("&Database/&Open      ", FL_META + 'o', w_open);
    menu->add("&Database/&Save As   ", FL_META + 's', w_save_as);
    menu->add("&Database/&Quit      ", FL_META + 'q', w_quit);
    menu->add("&Entry/&Add       ", FL_META + 'n', w_add);
    menu->add("&Entry/&Edit      ", FL_META + 'e', w_edit);
    menu->add("&Entry/&Delete    ", FL_META + FL_BackSpace, w_delete);
    menu->add("&Help/&About", 0, w_about);

    Fl_Group*  toolbar = new Fl_Group(0, 25, 480, 25);
    Fl_Button* b2      = new Fl_Button(0, 25, 25, 25);
    b2->image(img_add);
    b2->callback(w_add);
    Fl_Button* b3 = new Fl_Button(25, 25, 25, 25);
    b3->image(img_edit);
    b3->callback(w_edit);
    Fl_Button* b4 = new Fl_Button(50, 25, 25, 25);
    b4->image(img_del);
    b4->callback(w_delete);

    w_search_input = new Fl_Input(75, 25, 405, 25);
    w_search_input->callback(search);
    w_search_input->when(FL_WHEN_CHANGED);
    toolbar->end();

    w_main_browser      = new Fl_Hold_Browser(0, 50, 480, 270);
    static int widths[] = {200, 200, 0};
    w_main_browser->column_widths(widths);
    w_main_browser->column_char('\t');

    w_main_win->resizable(w_main_browser);
    w_main_win->end();
    w_main_win->show(argc, argv);
    return Fl::run();
}