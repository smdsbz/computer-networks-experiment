#include "pch.h"
#include "common_utils.h"

using std::string;
using std::ifstream;
using std::getline;

/* Function Implementations */

bool string_starts_with(
    const string &lhs,
    const string &rhs
)
{
    for (
            unsigned idx = 0,
                lhs_range = lhs.size(),
                rhs_range = rhs.size();
            idx != rhs_range;
            ++idx
        ) {
        if (idx == lhs_range) {     // lhs ends earlier than rhs
            return false;
        }
        else if (lhs[idx] != rhs[idx]) {    // not equal
            return false;
        }
    }
    return true;
}


bool string_ends_with(
    const string &lhs,
    const string &rhs
)
{
    int lhsidx = lhs.size() - 1,
        rhsidx = rhs.size() - 1;
    for (;
            (lhsidx != -1) && (rhsidx != -1);
            --lhsidx, --rhsidx
        ) {
        if (lhs[lhsidx] != rhs[rhsidx]) {
            return false;
        }
    }
    // if compare process ended, equal
    if (rhsidx == -1) {
        return true;
    }
    // ended for insufficient lhs chars, not equal
    return false;
}


string read_file(const string &filename)
{
    auto file = ifstream(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error(
            string("File not found: ")
            + filename
        );
    }
    // get file size
    std::streampos filesize = file.tellg();
    file.seekg(0, std::ios::end);
    filesize = file.tellg() - filesize;
    file.seekg(0, std::ios::beg);
    // make buffer space
    string ret;
    ret.resize(filesize, '\0');
    // read data into string buffer
    file.read(&ret[0], filesize);
    file.close();
    return ret;
}
