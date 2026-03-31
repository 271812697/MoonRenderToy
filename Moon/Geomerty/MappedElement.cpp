

# include <cstdlib>
# include <unordered_set>


#include "MappedElement.h"

using namespace Data;

bool ElementNameComparator::operator()(const MappedName& leftName,
                                       const MappedName& rightName) const
{
    int size = static_cast<int>(std::min(leftName.size(), rightName.size()));
    if (size == 0U) {
        return leftName.size() < rightName.size();
    }
    int currentIndex = 0;
    if (rightName[0] == '#') {
        if (leftName[0] != '#') {
            return true;
        }
        // If both string starts with '#', compare the following hex digits by
        // its integer value.
        int res = 0;
        for (currentIndex = 1; currentIndex < size; ++currentIndex) {
            auto ac = (unsigned char)leftName[currentIndex];
            auto bc = (unsigned char)rightName[currentIndex];
            if (std::isxdigit(bc) != 0) {
                if (std::isxdigit(ac) == 0) {
                    return true;
                }
                if (res == 0) {
                    if (ac < bc) {
                        res = -1;
                    }
                    else if (ac > bc) {
                        res = 1;
                    }
                }
            }
            else if (std::isxdigit(ac) != 0) {
                res = 1;
            }
            else {
                break;
            }
        }
        if (res < 0) {
            return true;
        }
        if (res > 0) {
            return false;
        }

        for (; currentIndex < size; ++currentIndex) {
            char ac = leftName[currentIndex];
            char bc = rightName[currentIndex];
            if (ac < bc) {
                return true;
            }
            if (ac > bc) {
                return false;
            }
        }
        return leftName.size() < rightName.size();
    }
    if (leftName[0] == '#') {
        return false;
    }

    // If the string does not start with '#', compare the non-digits prefix
    // using lexical order.
    for (currentIndex = 0; currentIndex < size; ++currentIndex) {
        auto ac = (unsigned char)leftName[currentIndex];
        auto bc = (unsigned char)rightName[currentIndex];
        if (std::isdigit(bc) == 0) {
            if (std::isdigit(ac) != 0) {
                return true;
            }
            if (ac < bc) {
                return true;
            }
            if (ac > bc) {
                return false;
            }
        }
        else if (std::isdigit(ac) == 0) {
            return false;
        }
        else {
            break;
        }
    }

    // Then compare the following digits part by integer value
    int res = 0;
    for (; currentIndex < size; ++currentIndex) {
        auto ac = (unsigned char)leftName[currentIndex];
        auto bc = (unsigned char)rightName[currentIndex];
        if (std::isdigit(bc) != 0) {
            if (std::isdigit(ac) == 0) {
                return true;
            }
            if (res == 0) {
                if (ac < bc) {
                    res = -1;
                }
                else if (ac > bc) {
                    res = 1;
                }
            }
        }
        else if (std::isdigit(ac) != 0) {
            return false;
        }
        else {
            break;
        }
    }
    if (res < 0) {
        return true;
    }
    if (res > 0) {
        return false;
    }

    // Finally, compare the remaining tail using lexical order
    for (; currentIndex < size; ++currentIndex) {
        char ac = leftName[currentIndex];
        char bc = rightName[currentIndex];
        if (ac < bc) {
            return true;
        }
        if (ac > bc) {
            return false;
        }
    }
    return leftName.size() < rightName.size();
}

HistoryItem::HistoryItem(const Data::MappedName &name)
    :tag(0),element(name)
{
    //if(obj)
        //tag = obj->getID();
}
