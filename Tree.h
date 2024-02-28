/**
 * This N-ary tree implementation offers construction, manipulation,
 * and traversal capabilities. Each node links to its parent and tracks its children.
 * It ensures strict encapsulation to prevent external access or
 * modification directly. Designed generically, it supports various
 * data types and includes serialization/deserialization methods for
 * the tree's storage, transmission, and reconstruction.
 * 
 * 
 * TREE REPRESENTATION
 * ___________________
 * 
 *      1
 *     /|\ 
 *    2 3 4
 *   /| |\
 *  5 6 7 8
 * 
 * 
 * 
 * LINEARIZED REPRESENTATION
 * _________________________
 * 
 * [1, 2, 5, X, 6, X, X, 3, 7, X, 8, X, X, 4, X, X]
 * 
 *
 * 
 * SERIALIZED REPRESENTATION
 * _________________________
 * 
 * [1]: "1"     <--- Node format [ID]: "value"
 * [2]: "2"
 * [5]: "5"
 * [X]          <--- end-of-children token
 * [6]: "6"
 * [X]
 * [X]
 * [3]: "3"
 * [7]: "7"
 * [X]
 * [8]: "8"
 * [X]
 * [X]
 * [4]: "4"
 * [X]
 * [X]
 * 
 * 
 */

#ifndef TREE_H
#define TREE_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <vector>

// Forward declaration of the Tree class to enable the Node class to declare it as a friend
template <typename T>
class Tree;

// Base template for has_out_stream_operator; assumes T does not have a << operator.
template <typename T, typename = void>
struct has_out_stream_operator : std::false_type {};

// Specialization of has_out_stream_operator; true if T has a << operator defined.
template <typename T>
struct has_out_stream_operator<T,
    std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>>
    : std::true_type {};

// Base template for has_in_stream_operator; assumes T does not have a >> operator.
template <typename T, typename = void>
struct has_in_stream_operator : std::false_type {};

// Specialization of has_in_stream_operator; true if T has a >> operator defined.
template <typename T>
struct has_in_stream_operator<T,
    std::void_t<decltype(std::declval<std::istream&>() >> std::declval<T&>())>>
    : std::true_type {};

// Type trait to check if both << and >> operators are defined for a type T.
template <typename T>
struct has_stream_operators {
    static constexpr bool value = has_out_stream_operator<T>::value && has_in_stream_operator<T>::value;
};

// Base template for has_default_constructor; assumes T does not have a default constructor.
template<typename T, typename = void>
struct has_default_constructor : std::false_type {};

// Specialization of has_default_constructor; true if T can be default-constructed.
template<typename T>
struct has_default_constructor<T, std::void_t<decltype(T())>> : std::true_type {};

// Base template for has_equality_operator; assumes T does not have an == operator.
template<typename T, typename = void>
struct has_equality_operator : std::false_type {};

// Specialization of has_equality_operator; true if T has an == operator defined.
template<typename T>
struct has_equality_operator<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

/**
 * @brief Represents a tree node.
 * 
 * Encapsulates a node's data, hierarchical position, and a unique ID to
 * ensure the tree's integrity. Access to node internals is restricted,
 * with Tree class as a friend to allow necessary interactions while
 * maintaining encapsulation.
 * 
 * @tparam T Data type of the node's value, supporting generic usage.
 */
template <typename T>
class Node {
private:
    T value;
    Node* parent;
    std::vector<std::unique_ptr<Node>> children;
    int ID; // Managed and used by tree class to search the nodeMap for pointers to nodes in the tree.

    /**
     * @brief Constructs a Node with a value and optional parent.
     * 
     * Private to enforce encapsulation, this constructor is used exclusively
     * by the Tree class to create nodes. It sets the node's value and parent,
     * critical for its integration into the tree.
     * 
     * @param value The value for the node.
     * @param parent Pointer to the parent node, or nullptr for root nodes.
     * 
     * @note Remove noexcept if T's move constructor may throw.
     */
    Node(T value, Node* parent = nullptr) noexcept : value(std::move(value)), parent(parent) {}

    friend class Tree<T>; // Grants Tree exclusive access to Node's private members.
};

/**
 * @brief Implements an N-ary tree for hierarchical data.
 * 
 * N-ary tree with strict encapsulation to preserve structural integrity. 
 * Provides functionality for node management, traversal, and 
 * serialization/deserialization, making it versatile for data storage, 
 * transfer, and reconstruction. Designed to be generic, supporting any 
 * data type, with specific requirements for type T detailed below.
 * 
 * @attention Type T requirements for compatibility:
 *    - << and >> operators for serialization/deserialization.
 *    - a default constructor for initializing members.
 *    - an == operator for instance comparison.
 *    - a nothrow move constructor for exception safe tree operations.
 * 
 * @attention Not thread-safe; avoid concurrent modifications.
 * @attention Throws exceptions/propagates exceptions for errors that could compromise tree integrity.
 */
template <typename T>
class Tree {
    // Ensure T meets the requirements for compatibility with Tree
    static_assert(has_stream_operators<T>::value, "Type T must have out-stream (<<) and in-stream (>>) operators defined.");
    static_assert(has_default_constructor<T>::value, "Type T must have a default constructor defined.");
    static_assert(has_equality_operator<T>::value, "Type T must have an equality (==) operator defined.");
    
private:
    std::unique_ptr<Node<T>> root;
    std::unordered_map<int, Node<T>*> nodeMap; // Maps node ID's to pointers for constant time access
    int nextID = 0; // Increments for each new node to ensure unique ID's

    /**
     * @brief Assigns unique IDs to nodes within a subtree recursively.
     * 
     * Ensures each node, from individual to full subtrees, receives a unique identifier,
     * updating `nodeMap` to link new IDs with node pointers. This method is crucial for
     * preserving structural integrity, used when adding nodes and during deserialization
     * to reset ID-to-node mappings.
     * 
     * @param node Pointer to the root of the subtree; if nullptr, return immediately.
     */
    void assignIDs(Node<T>* node) {
        if (!node) {
            return;
        }
        node->ID = nextID++;
        nodeMap[node->ID] = node;
        // If node has children, recursively assign IDs to them
        for (auto& child : node->children) {
            assignIDs(child.get());
        }
    }

    /**
     * @brief Converts the tree to a linearized vector representation.
     * 
     * Generates a linearized form of the tree as a vector of optional values, representing
     * node values and end-of-children tokens (std::nullopt). This process flattens the tree
     * into a sequence that retains hierarchical information, using a pre-order traversal
     * approach (root-first, then children, left-to-right).
     * 
     * The end-of-children tokens indicate to the delinearization algorithm exactly when
     * to pop the current parent from the stack, effectively moving up a level in the tree.
     * 
     * Linearization algorithm:
     * 1. Push the root to a stack if not empty.
     * 2. Pop the top node; if not nullptr, add its value to the vector, push a 
     *    nullptr to stack to mark end-of-chilren token, then push its children
     *    in reverse order for correct traversal.
     * 3. If nullptr, add an end-of-children marker to the vector.
     * 4. Repeat until the stack is empty, returning the linearized vector.
     * 
     * @return std::vector<std::optional<T>> The tree's linearized representation.
     */
    std::vector<std::optional<T>> linearize() const {
        std::vector<std::optional<T>> linearized;
        std::stack<const Node<T>*> stack;
        // if the root exists, push it onto the stack
        if (root) {
            stack.push(root.get());
        }

        while (!stack.empty()) {
            // while the stack is not empty, pop the top node from the stack
            const Node<T>* current = stack.top();
            stack.pop();

            // if the node is not a nullptr
            if (current) {
                // push node into the linearized vector
                linearized.push_back(current->value);
                // push a nullptr to indicate the end of the node's children list
                stack.push(nullptr);
                // push each of the node's children onto the stack in reverse order
                for (auto itr = current->children.rbegin(); itr != current->children.rend(); ++itr) {
                    stack.push(itr->get());
                }
            } else {
                // push the end of child list token into the linearized vector
                linearized.push_back(std::nullopt);
            }
        }
        return linearized;
    }

    /**
     * @brief Reconstructs a tree from its linearized vector representation.
     * 
     * This static method rebuilds a tree from a linear sequence of optional values,
     * representing node values and end-of-children tokens. It uses the end-of-children
     * tokens to determine when to pop the current parent from the stack. This accurately
     * recontructs the tree's original hierarchical structure.
     * 
     * Delinearization algorithm:
     * 1. Create an empty tree.
     * 2. Use a stack to track the current parent nodes.
     * 3. Iterate over the linearized data:
     *    - If encountering a non-empty value with no current tree, set it as the root.
     *    - If encountering a non-empty value with an existing tree, add a new node under
     *      the current parent, updating the stack with the new node's ID.
     *    - If encountering an empty value, pop the current parent from the stack.
     * 4. Return the newly formed tree.
     * 
     * @param linearized Linearized tree data.
     * @return Tree<T> The reconstructed tree.
     */
    static Tree<T> delinearize(const std::vector<std::optional<T>>& linearized) {
        Tree<T> tree;
        std::stack<int> parentIDs;

        for (const auto& optValue : linearized) {
            // Handle node value
            if (optValue.has_value()) {
                // Handle root node
                if (tree.getRootID() == -1) {
                    tree.setRoot(optValue.value());
                    parentIDs.push(tree.getRootID());
                // Handle the rest of the subtree's nodes
                } else {
                    int childID = tree.appendNode(parentIDs.top(), optValue.value());
                    // new node becomes the current parent, so push its ID onto the stack
                    parentIDs.push(childID);
                }
            // Handle end-of-children token
            } else if (!parentIDs.empty()) {
                // pop the parent from the stack since the end of the parent's children list has been reached
                parentIDs.pop();
            }
        }
        return tree;
    }

    /**
     * @brief Checks if type T meets serialization, deserialization, and comparison requirements.
     * 
     * Validates that type T supports serialization to a string via the out-stream operator (<<),
     * deserialization from a string via the in-stream operator (>>), and comparison for equality
     * via the equality operator (==). It attempts to serialize and deserialize an instance of T,
     * then compares the original and deserialized instances for equality. Throws an exception if
     * any of these operations are not supported as expected. If this method throws, it indicates
     * that T is incapable of being serialized and then accurately reconstructed via deserialization.
     * 
     * @throws std::invalid_argument if T lacks compatible <<, >> operators or equality operator.
     */
    void T_compatible_check() const {
        // Serialize an instance of T
        T testT = T();
        std::ostringstream testStream;
        testStream << testT;

        // Deserialize the serialized string
        T testT2;
        std::istringstream testStream2(testStream.str());
        testStream2 >> testT2;

        // Check if the original and deserialized instances are equal
        bool t_compatible = testT == testT2;
        // Throw if T is incompatible to prevent initialization of invalid trees
        if (!t_compatible) {
            std::stringstream errMsg;
            errMsg << "Type T has one or more of: an incompatible out-stream operator (<<), " <<
                      "in-stream operator (>>), or equality operator (==).\n" <<
                      "Ensure that out-stream converts T to a string, in-stream converts that " <<
                      "same string to the original T, and the equality operator correctly " <<
                      "returns true when comparing two instances of T where one is the result " <<
                      "of serializing and then deserializing the other.\n";
            throw std::invalid_argument(errMsg.str());
        }
    }

public:
    /**
     * @brief Initializes an empty Tree.
     * 
     * Constructs a Tree with no root, requiring `setRoot`
     * to add the root node. This constructor is ideal 
     * for rebuilding a tree via `delinearize`.
     */
    Tree() {
        T_compatible_check();
        root = nullptr;
    };

    /**
     * @brief Constructs a Tree with a root node from an lvalue.
     * 
     * Creates a tree with a root node, copying the provided value.
     * Suitable for when retaining the original value is necessary.
     * 
     * @param value The value for the root node.
     */
    Tree(const T& value) {
        T_compatible_check();
        setRoot(value);
    }

    /**
     * @brief Constructs a Tree with a root node from an rvalue.
     * 
     * Efficiently sets the root node using the provided movable
     * value. Ideal for transferring ownership without copying.
     * 
     * @param value The value to move as the root node.
     */
    Tree(T&& value) {
        T_compatible_check();
        setRoot(std::move(value));
    }
    
    /**
     * @brief Sets the tree's root node.
     * 
     * Initializes the tree with a specified value for the root.
     * Intended for use with an empty tree to start building its
     * structure. The method assigns a unique ID to the root and 
     * updates the nodeMap.
     * 
     * @param value The value for the root node.
     * @return int The ID of the root node.
     * @throw std::logic_error If attempting to set the root on a non-empty tree.
     */
    int setRoot(T value) {
        if (root) {
            throw std::logic_error("The root node has already been set");
        }
        // instantiate before passing to unique_ptr because make_unique doesn't have access to node constructor
        Node<T>* rootNode = new Node<T>(std::move(value));
        root.reset(rootNode);
        assignIDs(root.get());
        return root->ID;
    }

    /**
     * @brief Adds a child node under a specified parent.
     * 
     * Appends a new node with the provided value as a child to the 
     * given parent node, updating the nodeMap with a unique ID for
     * the new node. Returns the ID of the new node for reference.
     * 
     * @param parentID ID of the parent node.
     * @param value Value for the new node.
     * @return int ID of the new node.
     * @throw std::invalid_argument If the parent node ID is invalid.
     */
    int appendNode(int parentID, T value) {
        auto parentNodeItr = nodeMap.find(parentID);
        if (parentNodeItr == nodeMap.end()) {
            std::stringstream errMsg;
            errMsg << "Parent node with ID " << parentID << " does not exist.";
            throw std::invalid_argument(errMsg.str());
        }

        Node<T>* parentNode = parentNodeItr->second;

        // instantiate before passing to unique_ptr because make_unique doesn't have acccess to node constructor
        parentNode->children.push_back(std::unique_ptr<Node<T>>(new Node<T>(std::move(value), parentNode)));
        assignIDs(parentNode->children.back().get());

        return parentNode->children.back()->ID;
    }

    /**
     * @brief Removes a node and its subtree.
     * 
     * Deletes a node by ID, including its entire subtree, ensuring
     * removal from the nodeMap and detaching from its parent.
     * 
     * @param nodeID ID of the node to remove.
     * @throw std::invalid_argument If the node is the root or the ID is invalid.
     */
    void removeNode(int nodeID) {
        if (nodeID == getRootID()) {
            throw std::invalid_argument("The root node cannot be removed");
        }

        // find the node in the nodeMap
        auto nodeItr = nodeMap.find(nodeID);
        if (nodeItr == nodeMap.end()) {
            std::stringstream errMsg;
            errMsg << "Node with ID " << nodeID << " does not exist";
            throw std::invalid_argument(errMsg.str());
        }

        // lambda to recursively erase node, and its subtree's, nodeMap entries
        std::function<void(Node<T>*)> cleanupSubtree = [&](Node<T>* node) {
            for (auto& child : node->children) {
                cleanupSubtree(child.get());
            }
            nodeMap.erase(node->ID);
        };

        // Remove the node and its subtree from the nodeMap
        Node<T>* nodeToRemove = nodeItr->second;
        cleanupSubtree(nodeToRemove);

        // remove the node from its parent's list of children
        if (nodeToRemove->parent) {
            auto& siblings = nodeToRemove->parent->children;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                        [nodeToRemove](const std::unique_ptr<Node<T>>& child) {
                            return child.get() == nodeToRemove;
                        }), siblings.end());
        }
        // the entire subtree is now implicitly released because of unique_ptrs and vectors
    }

    /**
     * @brief Gets the root node's ID.
     * 
     * Returns the ID of the root node for referencing in further
     * operations, or -1 if the tree lacks a root.
     * 
     * @return int The root node's ID, or -1 if absent.
     */
    int getRootID() const noexcept {
        return root ? root->ID : -1;
    }

    /**
     * @brief Lists a node's children IDs.
     * 
     * Returns the IDs of a specified node's children for external reference. 
     * Returns an empty vector if the node has no children.
     * 
     * @param nodeID ID of the node to query.
     * @return std::vector<int> IDs of the node's children.
     * @throw std::invalid_argument If node ID is invalid.
     */
    std::vector<int> getChildrenIDs(int nodeID) const {
        auto nodeItr = nodeMap.find(nodeID);
        if (nodeItr == nodeMap.end()) {
            std::stringstream errMsg;
            errMsg << "Node with ID " << nodeID << " does not exist";
            throw std::invalid_argument(errMsg.str());
        }

        std::vector<int> childrenIDs;
        for (const auto& child : nodeItr->second->children) {
            childrenIDs.push_back(child->ID);
        }

        return childrenIDs;
    }

    /**
     * @brief Retrieves a node's value.
     * 
     * Provides read-only access to a node's value.
     * 
     * @param nodeID ID for value retrieval.
     * @return T const& The node's value.
     * @throw std::invalid_argument If node ID is invalid.
     */
    const T& getValue(int nodeID) const {
        auto nodeItr = nodeMap.find(nodeID);
        if (nodeItr == nodeMap.end()) { 
            std::stringstream errMsg; 
            errMsg << "Node with ID " << nodeID << " does not exist";
            throw std::invalid_argument(errMsg.str());
        }

        return nodeItr->second->value;
    }

    /**
     * @brief Accesses a node's value by ID.
     * 
     * Offers a concise method to retrieve a node's value using its ID, similar to arra
     * indexing. Functionally equivalent to `getValue`. It's just syntactic sugar.
     * 
     * @param nodeID ID of the node.
     * @return T const& Value of the node.
     * @throw std::invalid_argument If node is missing.
     */
    const T& operator[](int nodeID) const {
        auto nodeItr = nodeMap.find(nodeID);
        if (nodeItr == nodeMap.end()) {
            std::stringstream errMsg;
            errMsg << "Node with ID " << nodeID << " does not exist";
            throw std::invalid_argument(errMsg.str());
        }

        return nodeItr->second->value;
    }

    /**
     * @brief Serializes the tree to a string format.
     * 
     * Transforms the tree into a string for easy storage, transmission,
     * and later reconstruction, using a linearized form with nodes
     * numbered and children list ends marked by "[X]", referred to as
     * end-of-children tokens. The end-of-children tokens maintain
     * complete hierarchical information for accurate reconstruction.
     * 
     * @return std::string The tree's serialized form.
     */
    std::string serialize() const {
        auto linearized = linearize();
        std::string serialized;
        int nodeCount = 0;

        for (const auto& optValue : linearized) {
            // Handle node values
            if (optValue.has_value()) {
                 // use new stream each loop
                std::ostringstream textStream;
                 // serialize the node
                textStream << "[" << nodeCount++ << "]: " << optValue.value() << "\n";
                // append the serialized node to the string
                serialized += textStream.str();
            // Handle end-of-children tokens
            } else {
                serialized += "[X]\n";
            }
        }

        return serialized;
    }

    /**
     * @brief Rebuilds a tree from its serialized string form.
     * 
     * Deserializes a tree from a string, restoring its structure.
     * Validates line formatting and node to end-of-children token
     * ratios to ensure accurate reconstruction.
     * 
     * @param serialized Serialized tree string.
     * @return Tree<T> The deserialized tree.
     * @throw std::invalid_argument for parsing errors or invalid serialization.
     */
    static Tree<T> deserialize(const std::string& serialized) {
        std::vector<std::optional<T>> linearized;
        std::istringstream ss(serialized);
        std::string line;
        int nodeCount = 0, eocTokenCount = 0;
        std::stringstream errMsg;

        try {

        while (std::getline(ss, line)) {
            // Handle end-of-children tokens
            if (line == "[X]") {
                linearized.push_back(std::nullopt);
                // Invalid hierarchical structure - too many end-of-children tokens
                if (++eocTokenCount > nodeCount) {
                    errMsg << "Invalid tree serialization: too many end-of-children tokens\n"
                        << "A valid serialization should have one end-of-children token for every node.\n"
                        << "Occurred during line " << nodeCount + eocTokenCount << " of the serialization.\n";
                    throw std::invalid_argument(errMsg.str());
                }
            // Handle node values
            } else if (line.find("[") == 0 && line.find("]: ") != std::string::npos) {
                std::string valuePart = line.substr(line.find("]: ") + 3);
                // Compile time check to handle strings differently for serialization
                // Since istream stops at the first whitespace; We want to capture the entire line
                if constexpr (std::is_same<T, std::string>::value) {
                    linearized.push_back(valuePart);
                    nodeCount++;
                // Handle fundamental types
                // And custom types that implement the << and >> operators    
                } else {
                    std::istringstream valueStream(valuePart);
                    T value;
                    if (valueStream >> value) {
                        linearized.push_back(value);
                        nodeCount++;
                    } else {
                        errMsg << "Invalid tree serialization: unable to parse value\n"
                            << "Unable to parse value from line while deserializing tree: " << line << "\n"
                            << "Occurred during line " << nodeCount + eocTokenCount << " of the serialization.\n";
                        throw std::invalid_argument(errMsg.str());
                    }
                }
            // Invalid line format
            } else {
                errMsg << "Invalid tree serialization: invalid line format\n"
                    << "Malformed line detected while deserializing tree: " << line << "\n"
                    << "Expected format: '[n]: value' for nodes or '[X]' for end-of-children tokens.\n"
                    << "Occurred during line " << nodeCount + eocTokenCount << " of the serialization.\n";
                throw std::invalid_argument(errMsg.str());
            }
        }

        // Invalid hierarchical structure - too few end-of-children tokens
        if (nodeCount != eocTokenCount) {
            errMsg << "Invalid tree serialization: Too few end-of-children tokens\n"
                << "A valid serialization should have one end-of-children token for every node.\n";
            throw std::invalid_argument(errMsg.str());
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

        return delinearize(linearized);
    }

    /**
     * @brief Displays the tree's linearized form in the console.
     * 
     * Useful for debugging, this method prints the tree's structure in
     * a linear format, showing node values and end-of-children tokens.
     */
    void printLinearized() const {
        auto linearized = linearize();
        std::cout << "[ ";
        for (size_t i = 0; i < linearized.size(); ++i) {
            // Handle node values
            if (linearized[i].has_value()) {
                std::cout << linearized[i].value();
            // Handle end-of-children tokens
            } else {
                std::cout << "X";
            }
            // Comma separation for all but the last element
            if (i != linearized.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << " ]";
        std::cout << std::endl;
    }
};

#endif // TREE_H