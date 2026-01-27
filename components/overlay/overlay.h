#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

namespace overlay {

enum class Type { None, Entry };

class Base : public Fl_Group {
   public:
    using Fl_Group::Fl_Group;

    virtual Type type() const = 0;

    virtual void on_show() {}

    int handle(int e) override;

    virtual ~Base() = default;
};

void init(int X, int Y, int W, int H);
void show(Type type);
void hide();
Type current();


class Editor;
Editor& editor();

}  // namespace overlay