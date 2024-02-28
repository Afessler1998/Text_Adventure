#ifndef STORYNODE_H
#define STORYNODE_H

#include <cctype>
#include <iostream>
#include <string>

struct StoryNode{
    std::string action =" "; // default values for these strings.
    std::string outcome = " ";

    bool operator ==(const StoryNode &sn ){
        return action == sn.action && outcome == sn.outcome;
    }
};

std::ostream& operator <<(std::ostream &os, const StoryNode &sn);

std::istream& operator >>(std::istream &is, StoryNode &sn);

#endif // STORYNODE_H