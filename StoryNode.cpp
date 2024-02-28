#include <cctype>
#include <iostream>
#include <string>

#include "StoryNode.h"

std::ostream& operator <<(std::ostream &os, const StoryNode &sn) { // conversion from "StoryNode" type to "string".
    os << "action: \"" << sn.action << "\" outcome: \"" << sn.outcome << "\"";
    return os;
}

std::istream& operator >>(std::istream &is, StoryNode &sn) { // conversion from "string" to "StoryNode"
    std::string buffer;
    std::getline(is, buffer);
    if(!buffer.empty() && buffer.back() == ' '){ // supposed to minimize whitespace.
        buffer.pop_back();
    }

    //Initializing "action" and "outcome" struct members with substrings from buffer (special thanks to Alec for helping me with this).
    size_t actionPos = buffer.find("action: \"");
    size_t actionEnd = buffer.find("\"", actionPos + 9);
    sn.action = buffer.substr(actionPos + 9, actionEnd - (actionPos + 9));

    size_t outcomePos = buffer.find("outcome: \"", actionEnd);
    size_t outcomeEnd = buffer.find("\"", outcomePos + 10);
    sn.outcome = buffer.substr(outcomePos + 10, outcomeEnd - (outcomePos + 10));
    return is;
}