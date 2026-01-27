#include "overlay.h"

#include <FL/Fl.H>

#include "entry.h"

namespace overlay {

static Type   current_ = Type::None;
static Entry* entry_   = nullptr;

int Base::handle(int e) {
    if (e == FL_KEYDOWN && Fl::event_key() == FL_Escape) {
        overlay::hide();
        return 1;
    }
    return Fl_Group::handle(e);
}

void init(int X, int Y, int W, int H) {
    entry_ = new Entry(X, Y, W, H);
}

void show(Type type) {
    hide();
    current_ = type;

    if (type == Type::Entry) entry_->show(Mode::Add);
}

void hide() {
    if (entry_) entry_->hide();
    current_ = Type::None;
}

Type current() {
    return current_;
}

Entry& entry() {
    return *entry_;
}

}  // namespace overlay