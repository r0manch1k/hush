#pragma once

#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>

#include <string>

#include "./../database.h"
#include "overlay/entry.h"

using namespace std;

const char* const FN = "keepit.hush";

class MainWindow : public Fl_Double_Window {
   public:
    MainWindow(int X, int Y, int W, int H);

   private:
    string                path;
    string                password;
    vector<PasswordEntry> entries;

    int edit = -1;

    void retitle();
    void relist(const char* filter = nullptr);
    void refresh();

    bool exists();

    void push(string title, string login, string password);
    void autosave();

    // Колбеки
    void _open();
    void _saveas();
    void _add();

    // Графические элементы
    Fl_Menu_Bar*     _menu;
    Fl_Group*        _toolbar;
    Fl_Hold_Browser* _browser;
    Fl_Input*        _search;
    overlay::Entry*  _entry;
};
