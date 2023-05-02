#include "../Public/Location.h"

ostream& operator <<(ostream& os, const Location& loc)
{
    os << "(" << loc.x << ", " << loc.y << ")";
    return os;
}
