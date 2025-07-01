#include "filesystem.h"

/* Funções auxiliares */

TreeNode* btree_search_recursive(BTreeNode* node, const char* name);

void btree_traverse_recursive(BTreeNode* node);

TreeNode* btree_get_pred(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->leaf) {
        cur = cur->children[cur->num_keys];
    }
    return cur->keys[cur->num_keys - 1];
}

TreeNode* btree_get_succ(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx + 1];
    while (!cur->leaf) {
        cur = cur->children[0];
    }
    return cur->keys[0];
}

BTreeNode* btree_create_node(int leaf) {
    BTreeNode *newNode = malloc(sizeof(BTreeNode));
    newNode->num_keys = 0;
    for (int i = 0; i < 2 * BTREE_ORDER; i++) {
        newNode->children[i] = NULL;
    }
    newNode->leaf = leaf;
    return newNode;
}

void btree_split_child(BTreeNode *root, int i, BTreeNode *leftChild) {
    BTreeNode *rightChild = btree_create_node(leftChild->leaf);
    rightChild->num_keys = BTREE_ORDER - 1;

    for (int j = 0; j < BTREE_ORDER - 1; j++) {
        rightChild->keys[j] = leftChild->keys[j + BTREE_ORDER];
    }

    if (!leftChild->leaf) {
        for (int j = 0; j < BTREE_ORDER; j++) {
            rightChild->children[j] = leftChild->children[j + BTREE_ORDER];
        }
    }

    leftChild->num_keys = BTREE_ORDER - 1;

    for (int j = root->num_keys; j >= i + 1; j--) {
        root->children[j + 1] = root->children[j];
    }

    root->children[i + 1] = rightChild;

    for (int j = root->num_keys - 1; j >= i; j--) {
        root->keys[j + 1] = root->keys[j];
    }

    root->keys[i] = leftChild->keys[BTREE_ORDER - 1];

    root->num_keys = root->num_keys + 1;
}

void btree_insert_non_full(BTreeNode *node, TreeNode* key) {
    int i = node->num_keys - 1;

    if (node->leaf) {
        while (i >= 0 && strcmp(key->name, node->keys[i]->name) < 0) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys = node->num_keys + 1;
    } else {
        while (i >= 0 && strcmp(key->name, node->keys[i]->name) < 0) {
            i--;
        }
        i++;

        if (node->children[i]->num_keys == 2 * BTREE_ORDER - 1) {
            btree_split_child(node, i, node->children[i]);
            if (strcmp(key->name, node->keys[i]->name) > 0) {
                i++;
            }
        }
        btree_insert_non_full(node->children[i], key);
    }
}

int btree_find_key(BTreeNode *node, const char* name) {
    int idx = 0;
    while (idx < node->num_keys && strcmp(node->keys[idx]->name, name) < 0) {
        ++idx;
    }
    return idx;
}

void btree_borrow_from_prev(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx - 1];

    for (int i = child->num_keys - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->leaf) {
        for (int i = child->num_keys; i >= 0; --i) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->keys[0] = node->keys[idx - 1];

    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }

    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

void btree_borrow_from_next(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    child->keys[child->num_keys] = node->keys[idx];

    if (!child->leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }

    node->keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->num_keys; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->num_keys; ++i) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

void btree_merge(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    child->keys[BTREE_ORDER - 1] = node->keys[idx];

    for (int i = 0; i < sibling->num_keys; ++i) {
        child->keys[i + BTREE_ORDER] = sibling->keys[i];
    }

    if (!child->leaf) {
        for (int i = 0; i <= sibling->num_keys; ++i) {
            child->children[i + BTREE_ORDER] = sibling->children[i];
        }
    }

    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
    }
    for (int i = idx + 2; i <= node->num_keys; ++i) {
        node->children[i - 1] = node->children[i];
    }

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
}

void btree_fill(BTreeNode *node, int idx) {
    if (idx != 0 && node->children[idx - 1]->num_keys >= BTREE_ORDER) {
        btree_borrow_from_prev(node, idx);
    }
    else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= BTREE_ORDER) {
        btree_borrow_from_next(node, idx);
    }
    else {
        if (idx != node->num_keys) {
            btree_merge(node, idx);
        } else {
            btree_merge(node, idx - 1);
        }
    }
}

void btree_remove_from_node(BTreeNode *node, const char* name) {
    int idx = btree_find_key(node, name); 

    if (idx < node->num_keys && strcmp(node->keys[idx]->name, name) == 0) {
        if (node->leaf) {
            for (int i = idx + 1; i < node->num_keys; ++i) {
                node->keys[i - 1] = node->keys[i];
            }
            node->num_keys--;
        } else {
            if (node->children[idx]->num_keys >= BTREE_ORDER) {
                TreeNode* pred = btree_get_pred(node, idx); 
                node->keys[idx] = pred;
                btree_remove_from_node(node->children[idx], pred->name);
            }
            else if (node->children[idx + 1]->num_keys >= BTREE_ORDER) {
                TreeNode* succ = btree_get_succ(node, idx);
                node->keys[idx] = succ;
                btree_remove_from_node(node->children[idx + 1], succ->name);
            }
            else {
                btree_merge(node, idx);
                btree_remove_from_node(node->children[idx], name);
            }
        }
    } else {
        if (node->leaf) {
            return;
        }

        int flag = (idx == node->num_keys);

        if (node->children[idx]->num_keys < BTREE_ORDER) {
            btree_fill(node, idx);
        }

        if (flag && idx > node->num_keys) {
            btree_remove_from_node(node->children[idx - 1], name);
        } else {
            btree_remove_from_node(node->children[idx], name);
        }
    }
}

BTree* btree_create() {
    BTree* tree = malloc(sizeof(BTree));
    tree->root = NULL;
    return tree;
}


TreeNode* btree_search(BTree* tree, const char* name) {
    if (!tree || !tree->root) {
        return NULL;
    }
    return btree_search_recursive(tree->root, name);
}

TreeNode* btree_search_recursive(BTreeNode* node, const char* name) {
    if (node == NULL) {
        return NULL;
    }

    int i = 0;

    while (i < node->num_keys && strcmp(name, node->keys[i]->name) > 0) {
        i++;
    }

    if (i < node->num_keys && strcmp(name, node->keys[i]->name) == 0) {
        return node->keys[i];
    }

    if (node->leaf) {
        return NULL;
    }

    return btree_search_recursive(node->children[i], name);
}

void btree_insert(BTree* tree, TreeNode* node) {
    BTreeNode *root = tree->root;

    if (root == NULL) {
        tree->root = btree_create_node(1);
        tree->root->keys[0] = node;
        tree->root->num_keys = 1;
        return;
    }

    if (root->num_keys == 2 * BTREE_ORDER - 1) {
        BTreeNode *newRoot = btree_create_node(0);
        tree->root = newRoot;
        newRoot->children[0] = root;

        btree_split_child(newRoot, 0, root);

        int i = 0;
        if (strcmp(node->name, newRoot->keys[0]->name) > 0) {
            i++;
        }
        btree_insert_non_full(newRoot->children[i], node);

    } else {
        btree_insert_non_full(root, node);
    }
}

void btree_delete(BTree* tree, const char* name) {
    BTreeNode *root = tree->root;
    if (!root) {
        return;
    }

    btree_remove_from_node(root, name);

    if (root->num_keys == 0) {
        BTreeNode *tmp = root;
        if (root->leaf) {
            tree->root = NULL;
        } else {
            tree->root = root->children[0];
        }
        free(tmp);
    }
}

void btree_traverse(BTree* tree) {
    if (tree && tree->root) {
        btree_traverse_recursive(tree->root);
    } else {
        printf("(vazio)\n");
    }
}

void btree_traverse_recursive(BTreeNode* node) {
    if (node == NULL) {
        return;
    }

    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->leaf) {
            btree_traverse_recursive(node->children[i]);
        }

        const char* type_str = (node->keys[i]->type == FILE_TYPE) ? "ARQUIVO" : "DIRETÓRIO";
        printf("  - %-20s [%s]\n", node->keys[i]->name, type_str);
    }

    if (!node->leaf) {
        btree_traverse_recursive(node->children[i]);
    }
}