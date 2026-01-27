#pragma once

#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

#include "overlay.h"

namespace overlay {

enum class Mode { Add, Edit };

class Entry final : public Base {
   public:
    Entry(int X, int Y, int W, int H);

    Type type() const override { return Type::Entry; }

    void show(Mode mode);

    const char* title() const;
    const char* login() const;
    const char* password() const;

    void set(const char* title, const char* login, const char* pass);

   private:
    Mode             mode_;
    Fl_Input*        title_;
    Fl_Input*        login_;
    Fl_Secret_Input* pass_;
};

}  // namespace overlay