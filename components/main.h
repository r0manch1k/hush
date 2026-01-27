#pragma once

#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>

#include <string>

const char* const FN = "keepit.hush";

class Main : public Fl_Double_Window {
   public:
    Main(int X, int Y, int W, int H, const char* label);

   private:
    string path;
    string password;

    void utitle();
    void ubrowser(const char* filter = nullptr);
    void refresh();

    // Колбеки
    void open();
    void saveas();

    Fl_Menu_Bar*     menu_;
    Fl_Group*        toolbar_;
    Fl_Hold_Browser* browser_;
    Fl_Input*        search_input_;
    Fl_Button*       add_btn_;
    Fl_Button*       edit_btn_;
    Fl_Button*       del_btn_;

    void init();

    Fl_Hold_Browser* main_browser() { return browser_; }
    Fl_Input*        search_input() { return search_input_; }
};
