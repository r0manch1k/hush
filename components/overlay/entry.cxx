#include "entry.h"

#include <FL/Fl_Button.H>

namespace overlay {

Entry::Entry(int X, int Y, int W, int H, Okay ok, Cancel cancel)
    : Base(X, Y, W, H), ok_(ok), cancel_(cancel) {
    begin();

    box(FL_FLAT_BOX);

    _title    = new Fl_Input(W / 2 - 200 / 2, 90, 200, 25, "Title:");
    _login    = new Fl_Input(W / 2 - 200 / 2, 120, 200, 25, "User:");
    _password = new Fl_Secret_Input(W / 2 - 200 / 2, 150, 200, 25, "Pass:");

    auto* ok_btn     = new Fl_Button(W / 2 - 200 / 2, 185, 100 - 15 / 2, 25, "OK");
    auto* cancel_btn = new Fl_Button(W / 2 + 15 / 2, 185, 100 - 15 / 2, 25, "Cancel");

    ok_btn->callback(
        [](Fl_Widget*, void* d) {
            auto* self = static_cast<Entry*>(d);
            if (self->ok_) {
                self->ok_(self->title(), self->login(), self->password());
            }
            self->hide();
        },
        this);

    cancel_btn->callback(
        [](Fl_Widget*, void* d) {
            auto* self = static_cast<Entry*>(d);
            if (self->cancel_) self->cancel_();
            self->hide();
        },
        this);

    hide();

    end();
}

void Entry::set(const char* t, const char* l, const char* p) {
    _title->value(t);
    _login->value(l);
    _password->value(p);
}

void Entry::clear() {
    set("", "", "");
}

const char* Entry::title() const {
    return _title->value();
}

const char* Entry::login() const {
    return _login->value();
}

const char* Entry::password() const {
    return _password->value();
}

}  // namespace overlay