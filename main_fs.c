#include <stdio.h>
#include <string.h>
#include "filesystem.h"

// Protótipo para a função que salva o estado em fs.img
void save_fs_recursive(FILE* img_file, BTreeNode* node, int depth);

void print_help() {
    printf("\n--- Simple File System --- Comandos disponíveis ---\n");
    printf("  ls              - Lista o conteúdo do diretório atual.\n");
    printf("  mkdir <nome>    - Cria um novo diretório.\n");
    printf("  touch <nome>    - Cria um novo arquivo vazio.\n");
    printf("  rm <nome>       - Remove um arquivo.\n");
    printf("  rmdir <nome>    - Remove um diretório (deve estar vazio).\n");
    printf("  cd <path>       - Muda de diretório. Use '/' para ir para a raiz.\n");
    printf("  save            - Salva o estado atual do sistema em 'fs.img'.\n");
    printf("  help            - Mostra esta ajuda.\n");
    printf("  exit            - Encerra o programa.\n\n");
}

int main() {
    // Inicialização do sistema de arquivos
    Directory* root = get_root_directory();
    Directory* current_dir = root;

    char input[1024];
    char* command;
    char* arg1;

    print_help();

    while (1) {
        printf("FS > ");
        // Lê a linha de comando inteira
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Encerra em caso de erro ou EOF
        }

        // Remove a quebra de linha do final do input
        input[strcspn(input, "\n")] = 0;

        // Pega o primeiro token (o comando)
        command = strtok(input, " ");
        if (command == NULL) {
            continue; // Linha vazia, pede novo comando
        }

        // Pega o segundo token (o argumento)
        arg1 = strtok(NULL, " ");

        // --- Processamento dos Comandos ---

        if (strcmp(command, "ls") == 0) {
            list_directory_contents(current_dir);
        }
        else if (strcmp(command, "mkdir") == 0) {
            if (arg1 == NULL) {
                printf("Uso: mkdir <nome_do_diretorio>\n");
            } else {
                TreeNode* new_dir_node = create_directory(arg1);
                btree_insert(current_dir->tree, new_dir_node);
            }
        }
        else if (strcmp(command, "touch") == 0) {
            if (arg1 == NULL) {
                printf("Uso: touch <nome_do_arquivo>\n");
            } else {
                // Cria com conteúdo vazio, conforme solicitado
                TreeNode* new_file_node = create_txt_file(arg1, "");
                btree_insert(current_dir->tree, new_file_node);
            }
        }
        else if (strcmp(command, "rm") == 0) {
            if (arg1 == NULL) {
                printf("Uso: rm <nome_do_arquivo>\n");
            } else {
                delete_txt_file(current_dir->tree, arg1);
            }
        }
        else if (strcmp(command, "rmdir") == 0) {
             if (arg1 == NULL) {
                printf("Uso: rmdir <nome_do_diretorio>\n");
            } else {
                delete_directory(current_dir->tree, arg1);
            }
        }
        else if (strcmp(command, "cd") == 0) {
            if (arg1 == NULL) {
                printf("Uso: cd <path>\n");
            } else {
                change_directory(&current_dir, arg1, root);
            }
        }
        else if (strcmp(command, "save") == 0) {
            FILE* img = fopen("fs.img", "w");
            if (img) {
                fprintf(img, "ROOT\n");
                if (root->tree && root->tree->root) {
                    save_fs_recursive(img, root->tree->root, 1);
                }
                fclose(img);
                printf("Sistema de arquivos salvo em fs.img\n");
            } else {
                perror("Erro ao criar fs.img");
            }
        }
        else if (strcmp(command, "help") == 0) {
            print_help();
        }
        else if (strcmp(command, "exit") == 0) {
            printf("Encerrando o sistema de arquivos.\n");
            break; // Sai do loop while(1)
        }
        else {
            printf("Comando desconhecido: '%s'. Digite 'help' para ver a lista de comandos.\n", command);
        }
    }

    // TODO: Idealmente, aqui deveria haver uma função para liberar toda a memória
    // alocada pelo sistema de arquivos antes de encerrar.
    // free_filesystem(root);

    return 0;
}

/**
 * Função recursiva para salvar a estrutura do FS em um arquivo de imagem.
 */
void save_fs_recursive(FILE* img_file, BTreeNode* node, int depth) {
    if (node == NULL) return;

    int i;
    for (i = 0; i < node->num_keys; i++) {
        // Visita o filho esquerdo
        if (!node->leaf) {
            save_fs_recursive(img_file, node->children[i], depth + 1);
        }

        // Imprime a indentação para visualização hierárquica
        for (int j = 0; j < depth; j++) {
            fprintf(img_file, "│   ");
        }
        
        // Imprime o nó atual
        TreeNode* current_tnode = node->keys[i];
        fprintf(img_file, "└── %s\n", current_tnode->name);

        // Se for um diretório, chama a recursão para seus filhos
        if (current_tnode->type == DIRECTORY_TYPE) {
            Directory* dir = current_tnode->data.directory;
            if (dir->tree && dir->tree->root) {
                save_fs_recursive(img_file, dir->tree->root, depth + 1);
            }
        }
    }

    // Visita o último filho
    if (!node->leaf) {
        save_fs_recursive(img_file, node->children[i], depth + 1);
    }
}