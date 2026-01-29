#include "overlay.h"

#include <FL/Fl.H>

#include "entry.h"

namespace overlay {

int Base::handle(int e) {
    if (Fl_Group::handle(e)) return 1;

    if (e == FL_KEYDOWN && Fl::event_key() == FL_Escape) {
        hide();
        return 1;
    }

    switch (e) {
        case FL_PUSH:
        case FL_RELEASE:
        case FL_DRAG:
        case FL_MOVE:
        case FL_FOCUS:
        case FL_UNFOCUS:
        case FL_KEYDOWN:
        case FL_KEYUP: return 1;
    }

    return 0;
}

}  // namespace overlay