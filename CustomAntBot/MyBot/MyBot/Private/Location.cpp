#include "../Public/Location.h"

using namespace std;

ostream& operator <<(ostream& os, const Location& loc)
{
    os << "(" << loc.x << ", " << loc.y << ")";
    return os;
}
