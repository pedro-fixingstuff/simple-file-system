#include "filesystem.h"

TreeNode* create_txt_file(const char* name, const char* content) {
    File* file = malloc(sizeof(File));
    file->name = strdup(name);
    file->content = strdup(content);
    file->size = strlen(content);

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = FILE_TYPE;
    node->data.file = file;
    return node;
}

TreeNode* create_directory(const char* name) {
    Directory* dir = malloc(sizeof(Directory));
    dir->tree = btree_create();

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;
    node->data.directory = dir;
    return node;
}

void delete_txt_file(BTree* tree, const char* name) {
    TreeNode* node_to_free = btree_search(tree, name);

    if (node_to_free == NULL) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return;
    }

    if (node_to_free->type != FILE_TYPE) {
        printf("Erro: '%s' não é um arquivo de texto.\n", name);
        return;
    }

    btree_delete(tree, name);

    free(node_to_free->data.file->name);
    free(node_to_free->data.file->content);
    free(node_to_free->data.file);
    free(node_to_free->name);
    free(node_to_free);

    printf("Arquivo '%s' deletado com sucesso.\n", name);
}

void delete_directory(BTree* tree, const char* name) {
    TreeNode* node_to_free = btree_search(tree, name);

    if (node_to_free == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", name);
        return;
    }

    if (node_to_free->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", name);
        return;
    }

    Directory* dir_to_delete = node_to_free->data.directory;

    if (dir_to_delete->tree->root != NULL && dir_to_delete->tree->root->num_keys > 0) {
        printf("Erro: Não é possível remover o diretório '%s' porque ele não está vazio.\n", name);
        return;
    }

    btree_delete(tree, name);

    free(dir_to_delete->tree);
    free(dir_to_delete);
    free(node_to_free->name);
    free(node_to_free);

    printf("Diretório '%s' deletado com sucesso.\n", name);
}

Directory* get_root_directory() {
    Directory* root = malloc(sizeof(Directory));
    root->tree = btree_create();
    return root;
}

void change_directory(Directory** current_dir, const char* path, Directory* root) {
    if (strcmp(path, "/") == 0) {
        *current_dir = root;
        return;
    }

    TreeNode* target_node = btree_search((*current_dir)->tree, path);

    if (target_node == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", path);
        return;
    }

    if (target_node->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", path);
        return;
    }

    *current_dir = target_node->data.directory;
}

void list_directory_contents(Directory* dir) {
    if (!dir || !dir->tree || !dir->tree->root || dir->tree->root->num_keys == 0) {
        printf("(O diretório está vazio)\n");
        return;
    }
    btree_traverse(dir->tree);
}
#include "filesystem.h"

TreeNode* create_txt_file(const char* name, const char* content) {
    File* file = malloc(sizeof(File));
    file->name = strdup(name);
    file->content = strdup(content);
    file->size = strlen(content);

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = FILE_TYPE;
    node->data.file = file;
    return node;
}

TreeNode* create_directory(const char* name) {
    Directory* dir = malloc(sizeof(Directory));
    dir->tree = btree_create();

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;
    node->data.directory = dir;
    return node;
}

void delete_txt_file(BTree* tree, const char* name) {
    TreeNode* node_to_free = btree_search(tree, name);

    if (node_to_free == NULL) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return;
    }

    if (node_to_free->type != FILE_TYPE) {
        printf("Erro: '%s' não é um arquivo de texto.\n", name);
        return;
    }

    btree_delete(tree, name);

    free(node_to_free->data.file->name);
    free(node_to_free->data.file->content);
    free(node_to_free->data.file);
    free(node_to_free->name);
    free(node_to_free);

    printf("Arquivo '%s' deletado com sucesso.\n", name);
}

void delete_directory(BTree* tree, const char* name) {
    TreeNode* node_to_free = btree_search(tree, name);

    if (node_to_free == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", name);
        return;
    }

    if (node_to_free->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", name);
        return;
    }

    Directory* dir_to_delete = node_to_free->data.directory;

    if (dir_to_delete->tree->root != NULL && dir_to_delete->tree->root->num_keys > 0) {
        printf("Erro: Não é possível remover o diretório '%s' porque ele não está vazio.\n", name);
        return;
    }

    btree_delete(tree, name);

    free(dir_to_delete->tree);
    free(dir_to_delete);
    free(node_to_free->name);
    free(node_to_free);

    printf("Diretório '%s' deletado com sucesso.\n", name);
}

Directory* get_root_directory() {
    Directory* root = malloc(sizeof(Directory));
    root->tree = btree_create();
    return root;
}

void change_directory(Directory** current_dir, const char* path, Directory* root) {
    if (strcmp(path, "/") == 0) {
        *current_dir = root;
        return;
    }

    TreeNode* target_node = btree_search((*current_dir)->tree, path);

    if (target_node == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", path);
        return;
    }

    if (target_node->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", path);
        return;
    }

    *current_dir = target_node->data.directory;
}

void list_directory_contents(Directory* dir) {
    if (!dir || !dir->tree || !dir->tree->root || dir->tree->root->num_keys == 0) {
        printf("(O diretório está vazio)\n");
        return;
    }
    btree_traverse(dir->tree);
}
