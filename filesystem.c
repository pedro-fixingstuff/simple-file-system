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
    // 1. Busca o nó do arquivo para garantir que ele existe e é um arquivo
    TreeNode* node = btree_search(tree, name);

    if (node == NULL) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return;
    }

    if (node->type != FILE_TYPE) {
        printf("Erro: '%s' não é um arquivo de texto.\n", name);
        return;
    }

    // 2. Libera a memória alocada para o conteúdo do arquivo
    free(node->data.file->name);
    free(node->data.file->content);
    free(node->data.file);

    // 3. Libera a memória do próprio nó de árvore
    free(node->name);
    // free(node); // O nó em si será tratado pela lógica interna da árvore se necessário, mas o conteúdo é nossa responsabilidade

    // 4. Chama a função btree_delete para remover o nó da estrutura da árvore
    btree_delete(tree, name);

    printf("Arquivo '%s' deletado com sucesso.\n", name);
}

void delete_directory(BTree* tree, const char* name) {
    // 1. Busca o nó do diretório
    TreeNode* node = btree_search(tree, name);

    if (node == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", name);
        return;
    }

    if (node->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", name);
        return;
    }

    Directory* dir_to_delete = node->data.directory;

    // 2. Verifica se o diretório está vazio
    if (dir_to_delete->tree->root != NULL && dir_to_delete->tree->root->num_keys > 0) {
        printf("Erro: Não é possível remover o diretório '%s' porque ele não está vazio.\n", name);
        return;
    }

    // 3. Libera a memória associada ao diretório
    free(dir_to_delete->tree); // A árvore B dentro do diretório
    free(dir_to_delete);       // A estrutura do diretório
    free(node->name);          // O nome no nó da árvore

    // 4. Remove o diretório da árvore pai
    btree_delete(tree, name);

    printf("Diretório '%s' deletado com sucesso.\n", name);
}

Directory* get_root_directory() {
    Directory* root = malloc(sizeof(Directory));
    root->tree = btree_create();
    return root;
}

void change_directory(Directory** current_dir, const char* path, Directory* root) {
    // Caso especial: voltar para o diretório raiz
    if (strcmp(path, "/") == 0) {
        *current_dir = root;
        return;
    }

    // Navegar para um subdiretório
    // Busca o 'path' dentro do diretório atual
    TreeNode* target_node = btree_search((*current_dir)->tree, path);

    if (target_node == NULL) {
        printf("Erro: Diretório '%s' não encontrado.\n", path);
        return;
    }

    if (target_node->type != DIRECTORY_TYPE) {
        printf("Erro: '%s' não é um diretório.\n", path);
        return;
    }

    // Atualiza o ponteiro do diretório atual para o novo diretório
    *current_dir = target_node->data.directory;
}

void list_directory_contents(Directory* dir) {
    if (!dir || !dir->tree || !dir->tree->root || dir->tree->root->num_keys == 0) {
        printf("(O diretório está vazio)\n");
        return;
    }
    // A função btree_traverse agora faz o trabalho pesado de listar
    btree_traverse(dir->tree);
}
