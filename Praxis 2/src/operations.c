#include "../lib/operations.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fs_mkdir(file_system* fs, char* path) {
    if (fs == NULL || path == NULL || strlen(path) == 0 || path[0] != '/') {
        return -1;
    }

    int new_inode_address = find_free_inode(fs);
    if (new_inode_address == -1) {
        return -1;
    }
    inode* new_inode = &(fs->inodes[new_inode_address]);

    char* pfad = strtok(path, "/");
    char* name[NAME_MAX_LENGTH];
    int count = 0;

    while (pfad != NULL) {
        name[count] = pfad;
        count++;

        pfad = strtok(NULL, "/");
    }

    int parent_inode = fs->root_node;
    int parent_found = 0;

    for (int i = 0; i < count - 1; i++) {
        parent_found = 0;
        for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
            int block_adress = fs->inodes[parent_inode].direct_blocks[j];
            if (block_adress >= 0) {
                if (fs->inodes[block_adress].n_type == directory && strncmp(fs->inodes[block_adress].name, name[i], NAME_MAX_LENGTH) == 0) {
                    parent_inode = block_adress;
                    parent_found = 1;
                    break;
                }
            }
        }
        if (!parent_found) {
            return -1;
        }
    }

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        int block_adress = fs->inodes[parent_inode].direct_blocks[i];
        if (block_adress >= 0) {
            if (fs->inodes[block_adress].n_type == directory && strncmp(fs->inodes[block_adress].name, name[count - 1], NAME_MAX_LENGTH) == 0) {
                return -1;
            }
        }
    }

    strncpy(new_inode->name, name[count - 1], NAME_MAX_LENGTH);

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        new_inode->direct_blocks[i] = -1;
    }

    new_inode->parent = parent_inode;

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (fs->inodes[parent_inode].direct_blocks[i] == -1) {
            fs->inodes[parent_inode].direct_blocks[i] = new_inode_address;
            break;
        }
    }

    new_inode->n_type = directory;
    new_inode->size = 0;

    fs->s_block->free_blocks--;

    return 0;
}



int fs_mkfile(file_system *fs, char *path_and_name){
    if (fs == NULL || path_and_name == NULL || strlen(path_and_name) == 0 || path_and_name[0] != '/') {
        return -1;
    }

    int new_inode_address = find_free_inode(fs);
    if (new_inode_address == -1) {
        return -1;
    }

    inode* new_inode = &(fs->inodes[new_inode_address]);
    char* pfad = strtok(path_and_name, "/");
    char* name[NAME_MAX_LENGTH];
    int count = 0;

    while (pfad != NULL) {
        name[count] = pfad;
        count++;

        pfad = strtok(NULL, "/");
    }

    int parent_inode_index = fs->root_node;
    for (int i = 0; i < count - 1; i++) {
        int parent_found = 0;
        for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
            int block_num = fs->inodes[parent_inode_index].direct_blocks[j];
            if (block_num >= 0) {
                inode* dir_inode = &(fs->inodes[block_num]);
                if (dir_inode->n_type == directory && strncmp(dir_inode->name, name[i], NAME_MAX_LENGTH) == 0) {
                    parent_inode_index = block_num;
                    parent_found = 1;
                    break;
                }
            }
        }
        if (!parent_found) {
            return -1;
        }
    }

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        int block_num = fs->inodes[parent_inode_index].direct_blocks[i];
        if (block_num >= 0) {
            inode* dir_inode = &(fs->inodes[block_num]);
            if (dir_inode->n_type == reg_file && strncmp(dir_inode->name, name[count - 1], NAME_MAX_LENGTH) == 0) {
                return -2;
            }
        }
    }

    strncpy(new_inode->name, name[count - 1], NAME_MAX_LENGTH);

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        new_inode->direct_blocks[i] = -1;
    }

    new_inode->parent = parent_inode_index;

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (fs->inodes[parent_inode_index].direct_blocks[i] == -1) {
            fs->inodes[parent_inode_index].direct_blocks[i] = new_inode_address;
            break;
        }
    }

    new_inode->n_type = reg_file;
    new_inode->size = 0;

    fs->s_block->free_blocks--;

    return 0;

}


char *fs_list(file_system *fs, char *path) {
    // TESTS 
    if (fs == NULL || path == NULL) {
        return NULL;
    }
    if (strchr(path, '/') == NULL) {
        return NULL;
    }

    // Variablen
    char *pfad = strtok(path, "/");
    char *last_element = pfad;
    char *liste = malloc((DIRECT_BLOCKS_COUNT * (NAME_MAX_LENGTH)) + 1);
    liste[0] = '\0';
    int inode_address = 0;
    int address[DIRECT_BLOCKS_COUNT];
    int address_count = 0;

    // Pfad nach letztem Element durchsuchen
    while (pfad != NULL) {
        last_element = pfad;
        pfad = strtok(NULL, "/");
    }
    if (last_element != NULL) {
        for (int i = fs->root_node; fs->inodes[i].n_type != free_block; i++) {
            if (strcmp(fs->inodes[i].name, last_element) == 0) {
                inode_address = i;
                break;
            }
        }
    }
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (fs->inodes[inode_address].direct_blocks[i] != -1) {
            address[address_count] = fs->inodes[inode_address].direct_blocks[i];
            address_count++;
        }
    }

    // Adressen sortieren
    for (int j = 1; j < address_count; j++) {
        int key = address[j];
        int i = j - 1;
        while (i >= 0 && address[i] > key) {
            address[i + 1] = address[i];
            i = i - 1;
        }
        address[i + 1] = key;
    }

    for (int i = 0; i < address_count; i++) {
        if (fs->inodes[address[i]].n_type == directory) {
            strcat(liste, "DIR ");
            strcat(liste, fs->inodes[address[i]].name);
            strcat(liste, "\n");
        } else if (fs->inodes[address[i]].n_type == reg_file) {
            strcat(liste, "FIL ");
            strcat(liste, fs->inodes[address[i]].name);
            strcat(liste, "\n");
        }
    }

    return liste;
}


int fs_writef(file_system *fs, char *filename, char *text){
	// TESTS
    if (fs == NULL) {
        return -1;
    }
    if(filename == NULL){
        return -1;
    }
    if(text == NULL){
        return -1;
    }
    if (strchr(filename, '/') == NULL) {
        return -1;
    }
    
    // Variablen
    char *pfad = strtok(filename, "/");
    char *lastWord = NULL;
    int file_adress = -1;
    int sizeof_text = strlen(text);
    int text_left = sizeof_text;
    int block_adress[DIRECT_BLOCKS_COUNT];
    int write_blocks = 0;
    for(int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
        block_adress[i] = -1;
    }

    // Extrahieren des Filename aus dem Pfad
    while(pfad != NULL){
        lastWord = pfad;
        pfad = strtok(NULL, "/");
    }    
    if(lastWord == NULL){
        return -1; // Falls es keinen File gibt -> Fehler (Root Node ist immer directory)
    }
    
    // Adresse des File finden
    for(int i = fs -> root_node; fs -> inodes[i].n_type != free_block; i++){
        if(strcmp(fs -> inodes[i].name, lastWord) == 0){
            file_adress = i;
        }
    }
    if(file_adress == -1){
        return -1;  // Falls keine Fileadresse gefunden = Fehler -> File gibt es nicht
    }
    if(fs -> inodes[file_adress].n_type != reg_file){
        return -1;  // Falls File keine reg_file ist -> Fehler
    }

    // Direct-Blocks der Node abspeichern
    for(int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
        if(fs -> inodes[file_adress].direct_blocks[i] != -1){
            block_adress[i] = fs -> inodes[file_adress].direct_blocks[i];
        }
    }

    // Kleinsten freien Direct Block finden 
    int free_block = -1;
    for(int i = 0; block_adress[i] != -1 && i < DIRECT_BLOCKS_COUNT; i++){
        free_block = block_adress[i];
        break;
    }

    // Überprüfen ob genug Platz ist
    if(BLOCK_SIZE*DIRECT_BLOCKS_COUNT - fs -> inodes[file_adress].size < sizeof_text){
        return -2; // Falls nicht genug Platz -> Fehler
    }

    // Benötigte Anzahl an zusätzlichen Blöcken finden
    int blocks_needed = 0;
    if(free_block == -1){
        blocks_needed = sizeof_text / BLOCK_SIZE;
        if(sizeof_text % BLOCK_SIZE > 1){
            if(sizeof_text % BLOCK_SIZE != 0){
                blocks_needed++;
            }            
        }
    }
    else{
        if(fs -> data_blocks[free_block].size - sizeof_text > 0){
            blocks_needed = 0;
        }
        else{
            int block_adress = sizeof_text;
            block_adress -= fs -> data_blocks[free_block].size;
            if(block_adress > 0){
                blocks_needed = block_adress / BLOCK_SIZE;
                if(block_adress % BLOCK_SIZE != 0){
                    if(block_adress % BLOCK_SIZE != 0){
                        blocks_needed++;
                    }            
                }
            }           
        }
    }


    // Zusätzliche Blöcke erzeugen. Platz reservieren, in free_list auf belegt stellen, in Adress Array anhängen
    if (free_block != -1) {
        write_blocks++;
    }
    while (blocks_needed != 0) {
        int block_found = 0;
        for (int i = 0; block_found == 0; i++) {
            if (fs->free_list[i] == 1) {
                fs->free_list[i] = 0;
                if (free_block == -1) {
                    free_block = i;
                }
                for (int j = 0; j < DIRECT_BLOCKS_COUNT; j++) {
                    if (fs->inodes[file_adress].direct_blocks[j] == -1) {
                        fs->inodes[file_adress].direct_blocks[j] = i;
                        block_adress[j] = i;  // Korrekte Zuweisung des freien Blocks
                        fs -> s_block -> free_blocks--;
                        break;
                    }
                }
                block_found = 1;
            }
        }
        blocks_needed--;
        write_blocks++;
    }

    // Direct-Blocks der Node aktualisieren
    for(int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
        if(fs -> inodes[file_adress].direct_blocks[i] != -1){
            block_adress[i] = fs -> inodes[file_adress].direct_blocks[i];
        }
    }

    // Text in die Blöcke schreiben
    while(write_blocks != 0) {
        if(write_blocks == 1) {
            // Der Text passt vollständig in den Block          
            char existing_text[BLOCK_SIZE];
            strcpy(existing_text, (char*)fs->data_blocks[free_block].block);
            char new_text[BLOCK_SIZE];
            strncpy(new_text, text, text_left);
            new_text[text_left] = '\0';  // Null-Zeichen am Ende setzen
            strcat(existing_text, new_text);

            memcpy(fs->data_blocks[free_block].block, existing_text, strlen(existing_text));

            fs->inodes[file_adress].size += text_left;
            fs->data_blocks[free_block].size += text_left;   
            return sizeof_text;
            
        } else {
           
            int remaining_size = BLOCK_SIZE - fs->data_blocks[free_block].size;
            char text_split[remaining_size];

            strncpy(text_split, text, remaining_size);
            
            char remaining_text[BLOCK_SIZE];

            memcpy(remaining_text, text + remaining_size, strlen(text) +1 - strlen(text_split) + 1);
            strcpy(text, remaining_text);


            char existing_text[BLOCK_SIZE];
            existing_text[BLOCK_SIZE - 1] = '\0';  // Null-Zeichen am Ende setzen, um den Inhalt zu begrenzen
            strcpy(existing_text, (char*)fs->data_blocks[free_block].block);

            strcat(existing_text, text_split);

            memcpy(fs->data_blocks[free_block].block, existing_text, remaining_size);

            fs->inodes[file_adress].size += remaining_size;
            fs->data_blocks[free_block].size += remaining_size;
            text_left -= remaining_size;

            // Den nächsten freien Block finden
            for(int i = 0; i < DIRECT_BLOCKS_COUNT - 1; i++){
                if(block_adress[i] == free_block){
                    if(block_adress[i+1] != -1){
                        free_block = block_adress[i+1];
                        break;                       
                    }
                    else{
                        return -1;
                    }
                }
            }
        }
        write_blocks--;
    }
    
    return sizeof_text;
}

uint8_t *fs_readf(file_system *fs, char *filename, int *file_size) {
    // TESTS
    if (fs == NULL) {
        return NULL;
    }
    if (filename == NULL) {
        return NULL;
    }
    if (file_size == NULL) {
        return NULL;
    }
    if (strchr(filename, '/') == NULL) {
        return NULL;
    }

    // Variablen
    char *pfad = strtok(filename, "/");
    char *lastWord = NULL;
    int file_address = -1;
    int size = 0;
    int current_block = 0;
    int block_address[DIRECT_BLOCKS_COUNT];
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        block_address[i] = -1;
    }

    // Extrahieren des Dateinamens aus dem Pfad
    while (pfad != NULL) {
        lastWord = pfad;
        pfad = strtok(NULL, "/");
    }
    if (lastWord == NULL) {
        return NULL; // Falls es keine Datei gibt -> Fehler (Root Node ist immer ein Verzeichnis)
    }

    // Adresse der Datei finden
    for (int i = fs->root_node; fs->inodes[i].n_type != free_block; i++) {
        if (strcmp(fs->inodes[i].name, lastWord) == 0) {
            file_address = i;
            break;
        }
    }
    if (file_address == -1) {
        return NULL; // Falls keine Dateiadresse gefunden -> Fehler -> Datei existiert nicht
    }
    if (fs->inodes[file_address].n_type != reg_file) {
        return NULL; // Falls Datei keine reg_file ist -> Fehler
    }

    // Direkt-Blöcke des Inodes speichern
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (fs->inodes[file_address].direct_blocks[i] != -1) {
            block_address[i] = fs->inodes[file_address].direct_blocks[i];
        }
    }

    // Dateigröße ermitteln und in der übergebenen Variablen speichern
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (block_address[i] != -1) {
            size += fs->data_blocks[block_address[i]].size;
        }
        else {
            break;
        }
    }
    *file_size = size;

    if (size == 0) {
        return NULL; // Falls Datei leer ist -> Fehler
    }

    // Anzahl der zu lesenden Blöcke ermitteln
    int read_blocks = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE > 0) {
        read_blocks++;
    }

    char file_content[BLOCK_SIZE * DIRECT_BLOCKS_COUNT];
    file_content[0] = '\0';

    // Inhalt der Blöcke in den file_content kopieren
    for (int i = 0; i < read_blocks; i++) {
        if (block_address[current_block] != -1) {
            strncat(file_content, (char*)fs->data_blocks[block_address[current_block]].block, BLOCK_SIZE);
        }
        current_block++;
    }

    uint8_t *ptr = (uint8_t*)malloc((size + 1) * sizeof(uint8_t));
    ptr[size] = '\0';
    strcpy((char*)ptr, file_content);

    return ptr;
}

void fs_rm_directory(file_system* fs, int inode_index) {
    if (fs -> inodes[inode_index].n_type == directory) {
        for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
            if (fs -> inodes[inode_index].direct_blocks[i] != -1) {
                int child_inode = fs -> inodes[inode_index].direct_blocks[i];
                fs_rm_directory(fs, child_inode);
                fs -> inodes[child_inode].n_type = free_block;
                fs -> free_list[child_inode] = 1;
                fs -> inodes[inode_index].direct_blocks[i] = -1;
            }
        }
    }

    fs -> inodes[inode_index].n_type = free_block;
    fs -> free_list[inode_index] = 1;
}

int fs_rm(file_system* fs, char* path) {
    // TESTS
    if (fs == NULL) {
        return -1;
    }
    if (path == NULL) {
        return -1;
    }
    if (strchr(path, '/') == NULL) {
        return -1;
    }


    // Variablen
    int file_address = -1;
    char* lastWord = NULL;
    char* pfad = strtok(path, "/");


    // Extrahieren des Dateinamens aus dem Pfad
    while (pfad != NULL) {
        lastWord = pfad;
        pfad = strtok(NULL, "/");
    }
    if (lastWord == NULL) {
        return -1;
    }


    // Adresse der Datei finden
    int file_found = 0;
    for (int i = fs->root_node; fs->inodes[i].n_type != free_block; i++) {
        if (strcmp(fs->inodes[i].name, lastWord) == 0) {
            file_address = i;
            file_found = 1;
            break;
        }
    }
    if (file_found == 0) {
        return -1;
    }

    // Parent Inode festlegen
    int parent_inode = fs -> inodes[file_address].parent;
    fs_rm_directory(fs, parent_inode);

    int free_block_counter = 0;
    for(int i = 0; (int)fs -> free_list[i] != (uint8_t)0; i++) {
        if(fs -> free_list[i] == 1) {
            free_block_counter++;
        }
    }
    
    fs -> s_block -> free_blocks = free_block_counter;

    return 0;
}

int fs_import(file_system *fs, char *int_path, char *ext_path) {
    // TESTS
    if (fs == NULL) {
        return -1;
    }
    if (int_path == NULL) {
        return -1;
    }
    if (ext_path == NULL) {
        return -1;
    }
    if (strchr(int_path, '/') == NULL) {
        return -1;
    }

    FILE *file = fopen(ext_path, "r");
    if (file == NULL) {        
        return -1;
    }


    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = (char*)malloc(file_size +1 * sizeof(char));
    if(file_content == NULL) {
        fclose(file);
        return -1;
    }

    size_t read = fread(file_content, sizeof(char), file_size, file);
    if(read != file_size) {
        fclose(file);
        free(file_content);
        return -1;
    }

    file_content[file_size] = '\0';

    fclose(file);

    fs_writef(fs, int_path, file_content);

    return 0;
}

int fs_export(file_system *fs, char *int_path, char *ext_path) {
    // TESTS
    if (fs == NULL || int_path == NULL || ext_path == NULL || strchr(int_path, '/') == NULL) {
        return -1;
    }

    int file_address = -1;
    int file_size = 0;
    char *lastWord = NULL;
    char *parent = NULL;
    char *pfad = strtok(int_path, "/");

    // Extrahieren des Dateinamens aus dem Pfad
    while (pfad != NULL) {
        parent = lastWord;
        lastWord = pfad;
        pfad = strtok(NULL, "/");
    }
    if (lastWord == NULL) {
        return -1;
    }

    // Adresse der Datei finden
    int file_found = 0;
    for (int i = 0; fs->inodes[i].n_type != free_block; i++) {
        if (strcmp(fs->inodes[i].name, lastWord) == 0) {
            if (fs->inodes[i].n_type != reg_file) {
                return -1;
            } else {
                file_address = i;
                file_found = 1;
                break;
            }
        }
    }
    if (file_found == 0){
        return -1;
    }


    if(parent != NULL) {
        int parent_found = 0;
        for(int i = 0; fs->inodes[i].n_type != free_block; i++) {
            if(strcmp(fs->inodes[i].name, parent) == 0) {
                if(fs->inodes[i].n_type != directory) {
                    return -1;
                }
                parent_found = 1;
            }
        }
        if(parent_found == 0) {
            return -1;
        }
    }

    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (fs->inodes[file_address].direct_blocks[i] != -1) {
            file_size += fs->data_blocks[fs->inodes[file_address].direct_blocks[i]].size;
        }
    }

    uint8_t *file_content = (uint8_t *)malloc((file_size + 1) * sizeof(uint8_t));
    if (file_content == NULL) {
        return -1;
    }

    file_content[file_size] = '\0';
    file_content = fs_readf(fs, int_path, &file_size);

    FILE *file = fopen(ext_path, "w");
    if (file == NULL) {
        free(file_content);
        return -1;
    }

    size_t written = fwrite(file_content, sizeof(uint8_t), file_size, file);
    if (written != (size_t)file_size) {
        fclose(file);
        free(file_content);
        return -1;
    }

    fclose(file);
    free(file_content);

    return 0;
}

