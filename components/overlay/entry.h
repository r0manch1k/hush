#pragma once

#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

#include "overlay.h"

namespace overlay {

class Entry final : public Base {
   public:
    using Okay   = void (*)(const char*, const char*, const char*);
    using Cancel = void (*)();

    Entry(int X, int Y, int W, int H, Okay ok, Cancel cancel);
    const char* title() const;
    const char* login() const;
    const char* password() const;
    void        set(const char* title, const char* login, const char* pass);
    void        clear();

   private:
    Fl_Input*        _title;
    Fl_Input*        _login;
    Fl_Secret_Input* _password;

    Okay   ok_;
    Cancel cancel_;
};

}  // namespace overlay