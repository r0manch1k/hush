#include "mainwindow.h"

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
#include "overlay/entry.h"
#include "overlay/overlay.h"

using namespace std;

MainWindow::MainWindow(int X, int Y, int W, int H) : Fl_Double_Window(X, Y, W, H) {
    _menu = new Fl_Menu_Bar(0, 0, w(), 25);
    _menu->add(
        "&Database/&Open      ", FL_META + 'o',
        [](Fl_Widget*, void* d) {
            MainWindow* self = static_cast<MainWindow*>(d);
            self->_open();
        },
        this);
    _menu->add(
        "&Database/&Save As   ", FL_META + 's',
        [](Fl_Widget*, void* d) {
            MainWindow* self = static_cast<MainWindow*>(d);
            self->_saveas();
        },
        this);
    _menu->add("&Database/&Quit", FL_META + 'q', [](Fl_Widget*, void*) { exit(0); });
    _menu->add(
        "&Entry/&Add", FL_META + 'n',
        [](Fl_Widget*, void* d) {
            MainWindow* self = static_cast<MainWindow*>(d);
            self->_add();
        },
        this);
    _menu->add("&Entry/&Edit", FL_META + 'e', nullptr);
    _menu->add("&Entry/&Delete", FL_BackSpace, nullptr);
    _menu->add("&Help/&About", 0, nullptr);

    _toolbar = new Fl_Group(0, 25, w(), 25);

    static Fl_Pixmap add_img(add_xpm), edit_img(edit_xpm), del_img(delete_xpm);

    Fl_Button* add_btn = new Fl_Button(0, 25, 25, 25);
    add_btn->image(add_img);
    add_btn->callback(
        [](Fl_Widget*, void* d) {
            MainWindow* self = static_cast<MainWindow*>(d);
            self->_add();
        },
        this);

    Fl_Button* edit_btn = new Fl_Button(25, 25, 25, 25);
    edit_btn->image(edit_img);

    Fl_Button* del_btn = new Fl_Button(50, 25, 25, 25);
    del_btn->image(del_img);

    _search = new Fl_Input(3 * 25, 25, w() - 3 * 25, 25);

    _toolbar->end();

    _browser = new Fl_Hold_Browser(0, 2 * 25, w(), h() - 2 * 25);

    static int widths[] = {200, 200, 0};

    _browser->column_widths(widths);
    _browser->column_char('\t');

    _entry = new overlay::Entry(
        0, 0, W, H,
        [](const char* t, const char* l, const char* p, void* d) {
            auto* self = static_cast<MainWindow*>(d);
            self->push(t, l, p);
        },
        [](void*) { printf("cancel\n"); }, this);

    resizable(_browser);

    end();
}


bool MainWindow::exists() {
    if (!path.empty()) return true;

    int choice = fl_choice("No database is open. Would you like to create a new one?", "Cancel",
                           "New", nullptr);

    if (choice == 1) {
        _saveas();
        return !path.empty();
    }

    return false;
}


void MainWindow::retitle() {
    const char* label;
    if (!path.empty()) {
        size_t slash = path.find_last_of("/\\");
        string fn    = (slash == string::npos) ? path : path.substr(slash + 1);
        label        = format("Hush - {}", fn).c_str();
    } else {
        label = format("Hush - {}", "keepit.hush").c_str();
    }
    this->label(label);
}

void MainWindow::relist(const char* filter) {
    if (!_browser) return;
    _browser->clear();
    string f = filter ? filter : "";
    for (const auto& entry : entries) {
        if (f.empty() || entry.title.find(f) != string::npos) {
            _browser->add((entry.title + "\t" + entry.login).c_str());
        }
    }
}

void MainWindow::refresh() {
    retitle();
    relist(_search ? _search->value() : nullptr);
}

void MainWindow::push(string title, string login, string password) {
    if (title.empty()) return;

    PasswordEntry entry = {title, login, password};

    if (edit > -1) {
        entries[edit] = entry;
    } else {
        entries.push_back(entry);
    }

    relist(_search->value());
    autosave();
}

void MainWindow::autosave() {
    if (!path.empty() && !password.empty()) {
        // db_save_file(path, password);
    }
}

void MainWindow::_open() {
    const char* file = fl_file_chooser("Open database", "*.hush", nullptr);

    if (!file) return;

    const char* p = fl_password("Password:", "");

    // if (p && db_load_file(&entries, &path, file, p)) {
    if (p) {
        password = p;
        refresh();
    } else if (p) {
        fl_alert("Incorrect password or corrupted file.");
    }
}

void MainWindow::_saveas() {
    const char* file = fl_file_chooser("Save database", "*.hush", FN);

    if (!file) return;

    const char* p = fl_password("Set password:", "");

    if (!p) return;

    password = p;
    // db_save_file(&entries, &path, file, p);
    retitle();
}

void MainWindow::_add() {
    // if (!exists()) return;
    edit = -1;
    _entry->clear();
    _entry->show();
}