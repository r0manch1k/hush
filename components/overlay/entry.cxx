#include "entry.h"

#include <FL/Fl_Button.H>

namespace overlay {

Entry::Entry(int X, int Y, int W, int H) : Base(X, Y, W, H) {
    box(FL_FLAT_BOX);
    color(fl_rgb_color(30, 30, 30));
    hide();

    title_ = new Fl_Input(140, 90, 200, 25, "Title:");
    login_ = new Fl_Input(140, 120, 200, 25, "User:");
    pass_  = new Fl_Secret_Input(140, 150, 200, 25, "Pass:");

    auto* ok     = new Fl_Button(180, 190, 60, 25, "OK");
    auto* cancel = new Fl_Button(260, 190, 80, 25, "Cancel");

    ok->callback([](Fl_Widget*, void*) { overlay::hide(); });

    cancel->callback([](Fl_Widget*, void*) { overlay::hide(); });

    end();
}

void Entry::show(Mode mode) {
    mode_ = mode;

    if (mode == Mode::Add) {
        title_->value("");
        login_->value("");
        pass_->value("");
    }

    overlay::show(Type::Entry);
}

void Entry::set(const char* t, const char* l, const char* p) {
    title_->value(t);
    login_->value(l);
    pass_->value(p);
}

const char* Entry::title() const {
    return title_->value();
}
const char* Entry::login() const {
    return login_->value();
}
const char* Entry::password() const {
    return pass_->value();
}

}  // namespace overlay