#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>

#include "database.h"
#include "icons/save.xpm"
#include "icons/add.xpm"
#include "icons/edit.xpm"
#include "icons/delete.xpm"

Fl_Double_Window* main_win = nullptr;
Fl_Hold_Browser* main_browser = nullptr;
Fl_Double_Window* editor_win = nullptr;
Fl_Input *in_title = nullptr, *in_user = nullptr, *search_input = nullptr;
Fl_Secret_Input *in_pass = nullptr;

std::string global_master_pass = "";
int edit_mode = -1; 

void update_title() {
    if (!main_win) return;
    if (current_db_path.empty()) {
        main_win->label("Hush - No Database");
    } else {
        size_t last_slash = current_db_path.find_last_of("/\\");
        std::string filename = (last_slash == std::string::npos) ? current_db_path : current_db_path.substr(last_slash + 1);
        static std::string title_storage; 
        title_storage = "Hush - " + filename;
        main_win->label(title_storage.c_str());
    }
}

void update_browser(const char* filter = nullptr) {
    if (!main_browser) return;
    main_browser->clear();
    std::string f = filter ? filter : "";
    for (const auto& entry : db_entries) {
        if (f.empty() || entry.title.find(f) != std::string::npos) {
            main_browser->add((entry.title + "\t" + entry.username).c_str());
        }
    }
}

void trigger_autosave() {
    if (!current_db_path.empty() && !global_master_pass.empty()) {
        db_save_file(current_db_path, global_master_pass);
    }
}


void save_as_cb(Fl_Widget*, void*);

bool ensure_db_exists() {
    if (current_db_path.empty()) {
        int choice = fl_choice("No database is open. Would you like to create a new database file first?", 
                               "Cancel", "Create New", nullptr);
        if (choice == 1) {
            save_as_cb(nullptr, nullptr);
            return !current_db_path.empty(); 
        }
        return false;
    }
    return true;
}


void search_cb(Fl_Widget* i, void*) {
    update_browser(((Fl_Input*)i)->value());
}

void open_db_cb(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Open Hush DB", "*.hush", nullptr);
    if (file) {
        const char* p = fl_password("Master Password:", "");
        if (p && db_load_file(file, p)) {
            global_master_pass = p;
            update_browser();
            update_title();
        } else if (p) {
            fl_alert("Incorrect password or corrupted file.");
        }
    }
}

void save_as_cb(Fl_Widget*, void*) {
    const char* file = fl_file_chooser("Save Database", "*.hush", "vault.hush");
    if (file) {
        const char* p = fl_password("Set Master Password:", "");
        if (p) {
            global_master_pass = p;
            db_save_file(file, p);
            update_title();
        }
    }
}

void add_cb(Fl_Widget*, void*) {
    if (!ensure_db_exists()) return;
    edit_mode = -1;
    in_title->value(""); in_user->value(""); in_pass->value("");
    editor_win->label("New Entry");
    editor_win->show();
}

void edit_cb(Fl_Widget*, void*) {
    if (!ensure_db_exists()) return;
    int v = main_browser->value();
    if (v > 0) {
        edit_mode = v - 1;
        PasswordEntry& e = db_entries[edit_mode];
        in_title->value(e.title.c_str());
        in_user->value(e.username.c_str());
        in_pass->value(e.password.c_str());
        editor_win->label("Edit Entry");
        editor_win->show();
    } else {
        fl_alert("Please select an entry to edit.");
    }
}

void delete_cb(Fl_Widget*, void*) {
    if (!ensure_db_exists()) return;
    int v = main_browser->value();
    if (v > 0) {
        if (fl_choice("Are you sure you want to delete this entry?", "Cancel", "Delete", nullptr) == 1) {
            db_entries.erase(db_entries.begin() + (v - 1));
            update_browser(search_input->value());
            trigger_autosave();
        }
    }
}

void save_entry_cb(Fl_Widget*, void*) {
    if (strlen(in_title->value()) == 0) return;

    PasswordEntry entry = {in_title->value(), in_user->value(), in_pass->value()};

    if (edit_mode >= 0) {
        db_entries[edit_mode] = entry;
    } else {
        db_entries.push_back(entry);
    }

    update_browser(search_input->value());
    editor_win->hide();
    trigger_autosave();
}

void about_cb(Fl_Widget*, void*) {
    fl_message_title("About Hush");
    fl_message(
        "Hush Password Manager v1.0\n\n"
        "A simple, secure, and fast password vault.\n"
        "Built with C++ and FLTK.\n\n"
        "Format: .hush (Binary Database)"
    );
}

void quit_cb(Fl_Widget*, void*) {
    exit(0);
}


int main(int argc, char **argv) {
    static Fl_Pixmap img_save(save_xpm), img_add(add_xpm), img_edit(edit_xpm), img_del(delete_xpm);

    editor_win = new Fl_Double_Window(280, 140, "New Entry");
    in_title = new Fl_Input(80, 10, 180, 25, "Title:");
    in_user  = new Fl_Input(80, 40, 180, 25, "User:");
    in_pass  = new Fl_Secret_Input(80, 70, 180, 25, "Pass:");
    Fl_Button* b_ok = new Fl_Button(190, 105, 80, 25, "Save");
    b_ok->callback(save_entry_cb);
    editor_win->end();
    editor_win->set_modal();

    main_win = new Fl_Double_Window(480, 320, "Hush - No Database");
    
    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 480, 25);
    menu->add("&File/&Open...",     FL_META + 'o', open_db_cb);
    menu->add("&File/&Save As...",  FL_META + 's', save_as_cb);
    menu->add("&File/&Quit",        FL_META + 'q', quit_cb);
    menu->add("&Edit/&Add Entry",   FL_META + 'n', add_cb);
    menu->add("&Edit/&Edit Entry",  FL_META + 'e', edit_cb);
    menu->add("&Edit/&Delete",      FL_META + FL_BackSpace, delete_cb);
    menu->add("&Help/&About",       0,             about_cb);

    Fl_Group* toolbar = new Fl_Group(0, 25, 480, 25);
    Fl_Button* b1 = new Fl_Button(0, 25, 25, 25);  b1->image(img_save); b1->callback(save_as_cb);
    Fl_Button* b2 = new Fl_Button(25, 25, 25, 25); b2->image(img_add);  b2->callback(add_cb);
    Fl_Button* b3 = new Fl_Button(50, 25, 25, 25); b3->image(img_edit); b3->callback(edit_cb);
    Fl_Button* b4 = new Fl_Button(75, 25, 25, 25); b4->image(img_del);  b4->callback(delete_cb);
    
    search_input = new Fl_Input(105, 25, 370, 25);
    search_input->callback(search_cb);
    search_input->when(FL_WHEN_CHANGED);
    toolbar->end();

    main_browser = new Fl_Hold_Browser(0, 50, 480, 270);
    static int widths[] = {200, 200, 0};
    main_browser->column_widths(widths);
    main_browser->column_char('\t');

    main_win->resizable(main_browser);
    main_win->end();
    main_win->show(argc, argv);
    return Fl::run();
}