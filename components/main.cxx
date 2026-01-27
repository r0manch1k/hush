#include "main.h"

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
#include "overlay/overlay.h"

using namespace std;

Main::Main(int X, int Y, int W, int H, const char* label) : Fl_Double_Window(X, Y, W, H, label) {
    overlay::init(X, Y, W, H);

    init();

    resizable(browser_);
    end();
}

void Main::init() {
    menu_ = new Fl_Menu_Bar(0, 0, w(), 25);
    menu_->add(
        "&Database/&Open      ", FL_META + 'o',
        [](Fl_Widget*, void* d) {
            Main* self = static_cast<Main*>(d);
            self->open();
        },
        this);
    menu_->add("&Database/&Save As   ", FL_META + 's', nullptr);
    menu_->add("&Database/&Quit      ", FL_META + 'q', [](Fl_Widget*, void*) { exit(0); });
    menu_->add("&Entry/&Add", FL_META + 'n', nullptr);
    menu_->add("&Entry/&Edit", FL_META + 'e', nullptr);
    menu_->add("&Entry/&Delete", FL_BackSpace, nullptr);
    menu_->add("&Help/&About", 0, nullptr);

    toolbar_ = new Fl_Group(0, 25, w(), 25);

    static Fl_Pixmap add_img(add_xpm), edit_img(edit_xpm), del_img(delete_xpm);

    add_btn_ = new Fl_Button(0, 25, 25, 25);
    add_btn_->image(add_img);

    edit_btn_ = new Fl_Button(25, 25, 25, 25);
    edit_btn_->image(edit_img);

    del_btn_ = new Fl_Button(50, 25, 25, 25);
    del_btn_->image(del_img);

    search_input_ = new Fl_Input(3 * 25, 25, w() - 3 * 25, 25);

    toolbar_->end();

    browser_            = new Fl_Hold_Browser(0, 2 * 25, w(), h() - 2 * 25);
    static int widths[] = {200, 200, 0};
    browser_->column_widths(widths);
    browser_->column_char('\t');
}


void Main::utitle() {
    const char* label;
    if (!global_db_path.empty()) {
        size_t last_slash = global_db_path.find_last_of("/\\");
        string fn =
            (last_slash == string::npos) ? global_db_path : global_db_path.substr(last_slash + 1);
        label = format("Hush - {}", fn).c_str();
    } else {
        label = format("Hush - {}", "keepit.hush").c_str();
    }
    this->label(label);
}

void Main::ubrowser(const char* filter) {
    if (!browser_) return;
    browser_->clear();
    string f = filter ? filter : "";
    for (const auto& entry : global_db_entries) {
        if (f.empty() || entry.title.find(f) != string::npos) {
            browser_->add((entry.title + "\t" + entry.login).c_str());
        }
    }
}

void Main::refresh() {
    update_title();
    update_browser(search_input_ ? search_input_->value() : nullptr);
}

void Main::open() {
    const char* file = fl_file_chooser("Open database", "*.hush", nullptr);

    if (!file) return;

    const char* p = fl_password("Password:", "");

    if (p && db_load_file(file, p)) {
        password = p;
        ubrowser();
        utitle();
    } else if (p) {
        fl_alert("Incorrect password or corrupted file.");
    }
}

void Main::saveas() {
    const char* file = fl_file_chooser("Save database", "*.hush", FN);

    if (!file) return;

    const char* p = fl_password("Set password:", "");

    if (!p) return;

    password = p;
    db_save_file(file, p);
    update_title();
}