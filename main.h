#ifndef MAIN_H
#define MAIN_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <vector>
#include <string>

struct PasswordEntry {
    std::string title;
    std::string username;
    std::string password;
};

#endif
