#ifndef REGEXHELPERS_HPP
#define REGEXHELPERS_HPP
#include <regex>
#include <string>
using std::regex;
using std::string;
using std::smatch;
static string getText(const string& patten, const string& target) {
    smatch sm;
    if(regex_search(target.cbegin(), target.cend(), sm, regex(patten, regex_constants::icase)))
        return sm.str();
    else
        return string();
}
static string replaceFirst(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar, regex_constants::format_first_only);
}
static string replaceAll(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar);
}


#endif
