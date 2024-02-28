#include <iostream>

#include "StoryNode.h"
#include "Tree.h"
#include "utils.h"

int main() {
    // Load the story from the file
    Tree<StoryNode> storyTree = loadStoryline("varian_wrynn.txt");
    int currentNodeID = storyTree.getRootID();
    
    // Main Game loop
    while (true) {
        // Displays the current story node to the user
        std::cout << "-------------------------------------------\n";
        std::cout << "Story:\n";
        std::cout << storyTree[currentNodeID].outcome << "\n";
        std::cout << "-------------------------------------------\n";

        // Get children IDs and display options ffor the user to pick from
        std::vector<int> childrenIDs = storyTree.getChildrenIDs(currentNodeID);
        if (childrenIDs.empty()) {
            std::cout << "End of story reached. Thanks for playing!\n";
            break;
        }

        std::cout << "Choose your next action:\n";
        for (size_t i = 0; i < childrenIDs.size(); ++i) {
            std::cout << i + 1 << ". " << storyTree[childrenIDs[i]].action << "\n";
        }

        // Gets the user choice, and then the user picks a choice
        int choice;
        std::cout << "Enter your choice (1-" << childrenIDs.size() << "): ";
        std::cin >> choice;

        // Validates user input
        if (choice < 1 || choice > static_cast<int>(childrenIDs.size())) {
            std::cout << "Invalid choice. Please enter a number between 1 and " << childrenIDs.size() << ".\n";
            continue;
        }

        // updates the node based on what the user input
        currentNodeID = childrenIDs[choice - 1];
    }

    return 0;
}