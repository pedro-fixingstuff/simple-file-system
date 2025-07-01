#include <stdio.h>
#include <string.h>
#include "filesystem.h"

// Protótipo para a função que salva o estado em fs.img
void save_fs_recursive(FILE* img_file, BTreeNode* node, int depth);

void print_help() {
    printf("\n--- Simple File System --- Comandos disponíveis ---\n");
    printf("  ls              - Lista o conteúdo do diretório atual.\n");
    printf("  mkdir <nome>    - Cria um novo diretório.\n");
    printf("  touch <nome>    - Cria um novo arquivo.\n");
    printf("  rm <nome>       - Remove um arquivo.\n");
    printf("  rmdir <nome>    - Remove um diretório (deve estar vazio).\n");
    printf("  cd <path>       - Muda de diretório. Use '/' para ir para a raiz.\n");
    printf("  save            - Salva o estado atual do sistema em 'fs.img'.\n");
    printf("  help            - Mostra esta ajuda.\n");
    printf("  exit            - Encerra o programa.\n\n");
}

int main() {
    Directory* root = get_root_directory();
    Directory* current_dir = root;

    char input[1024];
    char* command;
    char* arg1;

    print_help();

    while (1) {
        printf("FS > ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        command = strtok(input, " ");
        if (command == NULL) {
            continue;
        }

        arg1 = strtok(NULL, " ");

        // --- Processamento dos Comandos ---

        if (strcmp(command, "ls") == 0) {
            list_directory_contents(current_dir);
        }
        else if (strcmp(command, "mkdir") == 0) {
            if (arg1 == NULL) {
                printf("Uso: mkdir <nome_do_diretorio>\n");
            } else {
                if (btree_search(current_dir->tree, arg1) != NULL) {
                    printf("Erro: O nome '%s' já existe neste diretório.\n", arg1);
                } else {
                    TreeNode* new_dir_node = create_directory(arg1);
                    btree_insert(current_dir->tree, new_dir_node);
                }
            }
        }
        else if (strcmp(command, "touch") == 0) {
            if (arg1 == NULL) {
                printf("Uso: touch <nome_do_arquivo> [conteúdo do arquivo...]\n");
            } else {
                if (btree_search(current_dir->tree, arg1) != NULL) {
                    printf("Erro: O nome '%s' já existe neste diretório.\n", arg1);
                } else {
                    char* content = strtok(NULL, "");
                    if (content == NULL) {
                        content = "";
                    }

                    TreeNode* new_file_node = create_txt_file(arg1, content);
                    
                    if (new_file_node != NULL) {
                        btree_insert(current_dir->tree, new_file_node);
                    }
                }
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
            break;
        }
        else {
            printf("Comando desconhecido: '%s'. Digite 'help' para ver a lista de comandos.\n", command);
        }
    }
    return 0;
}

void save_fs_recursive(FILE* img_file, BTreeNode* node, int depth) {
    if (node == NULL) return;

    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->leaf) {
            save_fs_recursive(img_file, node->children[i], depth + 1);
        }

        for (int j = 0; j < depth; j++) {
            fprintf(img_file, "│   ");
        }
        
        TreeNode* current_tnode = node->keys[i];
        fprintf(img_file, "└── %s\n", current_tnode->name);

        if (current_tnode->type == DIRECTORY_TYPE) {
            Directory* dir = current_tnode->data.directory;
            if (dir->tree && dir->tree->root) {
                save_fs_recursive(img_file, dir->tree->root, depth + 1);
            }
        }
    }

    if (!node->leaf) {
        save_fs_recursive(img_file, node->children[i], depth + 1);
    }
}