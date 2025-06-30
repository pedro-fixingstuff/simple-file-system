#include "filesystem.h"

/* Funções auxiliares */

TreeNode* btree_search_recursive(BTreeNode* node, const char* name);

void btree_traverse_recursive(BTreeNode* node);

TreeNode* getPred(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->leaf) {
        cur = cur->children[cur->num_keys];
    }
    return cur->keys[cur->num_keys - 1];
}

TreeNode* getSucc(BTreeNode *node, int idx) {
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

    // Copia as últimas (T-1) chaves de leftChild para rightChild
    for (int j = 0; j < BTREE_ORDER - 1; j++) {
        rightChild->keys[j] = leftChild->keys[j + BTREE_ORDER];
    }

    // Se leftChild não for uma folha, copia os últimos T filhos de leftChild para rightChild
    if (!leftChild->leaf) {
        for (int j = 0; j < BTREE_ORDER; j++) {
            rightChild->children[j] = leftChild->children[j + BTREE_ORDER];
        }
    }

    // Reduz o número de chaves em leftChild
    leftChild->num_keys = BTREE_ORDER - 1;

    // Cria espaço para o novo filho em root
    for (int j = root->num_keys; j >= i + 1; j--) {
        root->children[j + 1] = root->children[j];
    }

    // Conecta rightChild como filho de root
    root->children[i + 1] = rightChild;

    // Move as chaves em root para abrir espaço para a nova chave
    for (int j = root->num_keys - 1; j >= i; j--) {
        root->keys[j + 1] = root->keys[j];
    }

    // Copia a chave do meio de leftChild para root
    root->keys[i] = leftChild->keys[BTREE_ORDER - 1];

    // Incrementa o contador de chaves em root
    root->num_keys = root->num_keys + 1;
}

void btree_insert_non_full(BTreeNode *node, TreeNode* key) {
    int i = node->num_keys - 1;

    // Se o nó é uma folha
    if (node->leaf) {
        // Encontra a posição correta para a nova chave e move as chaves maiores
        while (i >= 0 && strcmp(key->name, node->keys[i]->name) < 0) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        // Insere a nova chave
        node->keys[i + 1] = key;
        node->num_keys = node->num_keys + 1;
    } else { // Se o nó não é uma folha
        // Encontra o filho que receberá a nova chave
        while (i >= 0 && strcmp(key->name, node->keys[i]->name) < 0) {
            i--;
        }
        i++;

        // Verifica se o filho encontrado está cheio
        if (node->children[i]->num_keys == 2 * BTREE_ORDER - 1) {
            // Se estiver cheio, divide o filho
            btree_split_child(node, i, node->children[i]);
            // Decide para qual dos dois filhos (após a divisão) a chave deve ir
            if (strcmp(key->name, node->keys[i]->name) > 0) {
                i++;
            }
        }
        // Chama a função recursivamente para o filho apropriado
        btree_insert_non_full(node->children[i], key);
    }
}

int btree_find_key(BTreeNode *node, const char* name) {
    int idx = 0;
    // A correção está em usar 'node->keys[idx]->name' na comparação
    while (idx < node->num_keys && strcmp(node->keys[idx]->name, name) < 0) {
        ++idx;
    }
    return idx;
}

void btree_borrow_from_prev(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx - 1];

    // Move todas as chaves no filho uma posição para frente
    for (int i = child->num_keys - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
    }

    // Se o filho não for folha, move os ponteiros dos filhos também
    if (!child->leaf) {
        for (int i = child->num_keys; i >= 0; --i) {
            child->children[i + 1] = child->children[i];
        }
    }

    // A primeira chave do filho se torna a chave[idx-1] do pai ('node')
    child->keys[0] = node->keys[idx - 1];

    // Se o filho não for folha, o primeiro 'neto se torna o último filho do irmão
    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }

    // A última chave do irmão sobe para o pai
    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

void btree_borrow_from_next(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    // A chave[idx] do pai desce para ser a última chave de 'child'
    child->keys[child->num_keys] = node->keys[idx];

    // O primeiro filho do irmão se torna o último filho de 'child'
    if (!child->leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }

    // A primeira chave do irmão sobe para o pai
    node->keys[idx] = sibling->keys[0];

    // Move todas as chaves do irmão uma posição para trás
    for (int i = 1; i < sibling->num_keys; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    // Move os ponteiros dos filhos do irmão uma posição para trás
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

    // Puxa uma chave do nó pai para o filho
    child->keys[BTREE_ORDER - 1] = node->keys[idx];

    // Copia para o final do filho as chaves do seu irmão
    for (int i = 0; i < sibling->num_keys; ++i) {
        child->keys[i + BTREE_ORDER] = sibling->keys[i];
    }

    // Se não forem folhas, copia os ponteiros dos filhos também
    if (!child->leaf) {
        for (int i = 0; i <= sibling->num_keys; ++i) {
            child->children[i + BTREE_ORDER] = sibling->children[i];
        }
    }

    // Move as chaves e filhos no nó pai para preencher o buraco
    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
    }
    for (int i = idx + 2; i <= node->num_keys; ++i) {
        node->children[i - 1] = node->children[i];
    }

    // Atualiza contadores de chaves e libera o nó irmão
    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
}

void btree_fill(BTreeNode *node, int idx) {
    // Se o irmão anterior tem mais de "ordem-1" chaves, pega emprestado dele
    if (idx != 0 && node->children[idx - 1]->num_keys >= BTREE_ORDER) {
        btree_borrow_from_prev(node, idx);
    }
    // Se o irmão seguinte tem mais de "ordem-1" chaves, pega emprestado dele
    else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= BTREE_ORDER) {
        btree_borrow_from_next(node, idx);
    }
    // Senão, faz a fusão do filho com um de seus irmãos
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

    // Se a chave está presente neste nó
    if (idx < node->num_keys && strcmp(node->keys[idx]->name, name) == 0) {
        if (node->leaf) { // Chave está em um nó folha
            for (int i = idx + 1; i < node->num_keys; ++i) {
                node->keys[i - 1] = node->keys[i];
            }
            node->num_keys--;
        } else { // Chave está em um nó interno
            // Se o filho predecessor tem no mínimo "ordem" chaves
            if (node->children[idx]->num_keys >= BTREE_ORDER) {
                // CORREÇÃO: 'pred' é um TreeNode*, não um int
                TreeNode* pred = getPred(node, idx); 
                node->keys[idx] = pred;
                // CORREÇÃO: Passa o nome do predecessor para a remoção recursiva
                btree_remove_from_node(node->children[idx], pred->name);
            }
            // Se o filho sucessor tem no mínimo "ordem" chaves
            else if (node->children[idx + 1]->num_keys >= BTREE_ORDER) {
                // CORREÇÃO: 'succ' é um TreeNode*, não um int
                TreeNode* succ = getSucc(node, idx);
                node->keys[idx] = succ;
                // CORREÇÃO: Passa o nome do sucessor para a remoção recursiva
                btree_remove_from_node(node->children[idx + 1], succ->name);
            }
            // Se ambos os filhos têm "ordem-1" chaves, faz-se a fusão
            else {
                btree_merge(node, idx);
                btree_remove_from_node(node->children[idx], name);
            }
        }
    } else { // A chave não está neste nó
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
        return node->keys[i]; // Sucesso!
    }

    if (node->leaf) {
        return NULL;
    }

    return btree_search_recursive(node->children[i], name);
}

void btree_insert(BTree* tree, TreeNode* node) {
    BTreeNode *root = tree->root;

    // CASO ESPECIAL: A árvore está vazia. Esta é a correção.
    if (root == NULL) {
        // Cria um novo nó raiz, que também é uma folha.
        tree->root = btree_create_node(1); // 1 significa que é uma folha
        // Insere a primeira chave.
        tree->root->keys[0] = node;
        tree->root->num_keys = 1;
        return; // Inserção concluída.
    }

    // Se a raiz está cheia, a árvore cresce em altura.
    if (root->num_keys == 2 * BTREE_ORDER - 1) { // Esta linha agora é segura
        BTreeNode *newRoot = btree_create_node(0); // Nova raiz não é folha
        tree->root = newRoot;
        newRoot->children[0] = root;

        btree_split_child(newRoot, 0, root);

        // Decide para qual dos dois filhos a nova chave deve ir
        int i = 0;
        if (strcmp(node->name, newRoot->keys[0]->name) > 0) {
            i++;
        }
        btree_insert_non_full(newRoot->children[i], node);

    } else { // Se a raiz não está cheia, chama a inserção normal.
        btree_insert_non_full(root, node);
    }
}

void btree_delete(BTree* tree, const char* name) {
    BTreeNode *root = tree->root;
    if (!root) {
        return;
    }

    btree_remove_from_node(root, name);

    // Se a raiz ficar sem chaves, o seu primeiro filho se torna a nova raiz
    if (root->num_keys == 0) {
        BTreeNode *tmp = root;
        if (root->leaf) {
            // A árvore fica vazia
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
        // O diretório está vazio, não imprime nada ou uma mensagem.
        printf("(vazio)\n");
    }
}

void btree_traverse_recursive(BTreeNode* node) {
    if (node == NULL) {
        return;
    }

    int i;
    // Percorre as chaves e os filhos em ordem intercalada
    for (i = 0; i < node->num_keys; i++) {
        // 1. Visita a sub-árvore filha à esquerda da chave[i]
        if (!node->leaf) {
            btree_traverse_recursive(node->children[i]);
        }

        // 2. Processa (imprime) a chave[i]
        const char* type_str = (node->keys[i]->type == FILE_TYPE) ? "ARQUIVO" : "DIRETÓRIO";
        printf("  - %-20s [%s]\n", node->keys[i]->name, type_str);
    }

    // 3. Visita a última sub-árvore filha (à direita da última chave)
    if (!node->leaf) {
        btree_traverse_recursive(node->children[i]);
    }
}