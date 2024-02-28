#include <fstream>
#include <iostream>
#include <string>

#include "StoryNode.h"
#include "Tree.h"
#include "utils.h"

void saveStoryline(Tree<StoryNode> tree, std::string filePath) {
    std::ofstream outFile(filePath);
    if (!outFile) {
        std::cerr << "Unable to open file" << std::endl;
        return;
    }

    outFile << tree.serialize();

    outFile.close();
}

Tree<StoryNode> loadStoryline(std::string filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Unable to open file" << std::endl;
        return Tree<StoryNode>();
    }

    std::string story;
    std::string line;

    while (std::getline(inFile, line)) {
        story += line + "\n";
    }

    inFile.close();

    return Tree<StoryNode>::deserialize(story);
}