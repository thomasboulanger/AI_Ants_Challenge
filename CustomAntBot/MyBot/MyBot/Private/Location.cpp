#include "../Public/Location.h"

using namespace std;

// ostream& operator <<(ostream& os, const Location& loc)
// {
//     os << "(" << loc.row << ", " << loc.col << ")";
//     return os;
// }
std::ostream& operator<<(std::ostream& out, const Location& loc)
{
    return out << loc.row << ", " << loc.col;
}
