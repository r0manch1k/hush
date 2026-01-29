#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

namespace overlay {

class Base : public Fl_Group {
   public:
    using Fl_Group::Fl_Group;
    int handle(int e) override;
    virtual ~Base() = default;
};

}  // namespace overlay