#ifndef UTILS_H
#define UTILS_H

#include "StoryNode.h"
#include "Tree.h"

/**
 * @brief Saves the storyline to a file
 * 
 * Calls tree.serialize() to get the serialized string and writes it to a file as is.
 * The tree handles the rest.
 * 
 * @tparam T
 * @param tree tree to save
 * @param filePath file to save to
*/
void saveStoryline(Tree<StoryNode> tree, std::string filePath);

/**
 * @brief Loads the storyline from a file
 * 
 * Reads the file and returns a Tree<T> object using the serialized string.
 * 
 * @tparam T
 * @param filePath file to load from
 * @return Tree<StoryNode>
*/
Tree<StoryNode> loadStoryline(std::string filePath);

#endif // UTILS_H