#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static char* this_executable_name = "witness_hd_patcher.exe";

// Macro to create String from byte array
#define BYTE_ARRAY_TO_STRING(bytes) {.data = (char*)(bytes), .count = sizeof(bytes)}


void print_usage() {
    printf("Usage: %s <witness_install_path>\n", this_executable_name);
}

typedef struct String {
    char* data;
    size_t count;
} String;

String read_entire_file(char* filepath) {
    String result;

    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to patch. Could not open '%s'\n", filepath);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    result.count = ftell(file);
    fseek(file, 0, SEEK_SET);

    result.data = malloc(result.count + 1);
    fread(result.data, 1, result.count, file);
    result.data[result.count] = '\0';
    fclose(file);

    return result;
}


typedef struct Patch {
    const char* name;
    String search_bytes;

    /*
     * From the end of the search_bytes. So if search_bytes is [0x01, 0x02, 0x03, 0x4], and replacement_bytes is [0x0A, 0x0B],
     * the result of the patch will be [0x01, 0x02, 0x0A, 0x0B].
     */
    String replacement_bytes; 
} Patch;


static const unsigned char draw_connection_in_progress_search_bytes[] = {
    0xF3, 0x0F, 0x59, 0x8C, 0x24, 0xC0, 0x00, 0x00, 0x00,
    0x0F, 0x28, 0xDF,
    0xC7, 0x44, 0x24, 0x20, 0x0E, 0x00, 0x00, 0x00
};
static const unsigned char draw_connection_in_progress_replacement_bytes[] = {
    0xF3, 0x0F, 0x59, 0x8C, 0x24, 0xC0, 0x00, 0x00, 0x00,
    0x0F, 0x28, 0xDF,
    0xC7, 0x44, 0x24, 0x20, 0xFF, 0x00, 0x00, 0x00
};


static const unsigned char draw_dot_01_search_bytes[] = {
    0x45, 0x32, 0xF6,
    0xF6, 0xC1, 0x02,
    0x0F, 0xB6, 0xC8,
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xBD, 0x14, 0x00, 0x00, 0x00
};
static const unsigned char draw_dot_01_replacement_bytes[] = {
    0x45, 0x32, 0xF6,
    0xF6, 0xC1, 0x02,
    0x0F, 0xB6, 0xC8,
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xBD, 0xff, 0x00, 0x00, 0x00
};


static const unsigned char draw_dot_02_search_bytes[] = {
    0x0F, 0xA3, 0xD0,
    0x73, 0x37,
    0x48, 0x8B, 0xCF,
    0xBD, 0x18, 0x00, 0x00, 0x00
};

static const unsigned char draw_dot_02_replacement_bytes[] = {
    0x0F, 0xA3, 0xD0,
    0x73, 0x37,
    0x48, 0x8B, 0xCF,
    0xBD, 0xff, 0x00, 0x00, 0x00
};


Patch patches[] = {
    {
        .name = "draw_connection_in_progress",
        .search_bytes = BYTE_ARRAY_TO_STRING(draw_connection_in_progress_search_bytes),
        .replacement_bytes = BYTE_ARRAY_TO_STRING(draw_connection_in_progress_replacement_bytes)
    },
    {
        .name = "draw_dot_01",
        .search_bytes = BYTE_ARRAY_TO_STRING(draw_dot_01_search_bytes),
        .replacement_bytes = BYTE_ARRAY_TO_STRING(draw_dot_01_replacement_bytes)
    },
    {
        .name = "draw_dot_02",
        .search_bytes = BYTE_ARRAY_TO_STRING(draw_dot_02_search_bytes),
        .replacement_bytes = BYTE_ARRAY_TO_STRING(draw_dot_02_replacement_bytes)
    },
};
#define PATCH_COUNT (sizeof(patches) / sizeof(patches[0]))
size_t patch_offsets[PATCH_COUNT] = {0};

int main(int argc, char* argv[]) {
    this_executable_name = argv[0];

    if (argc <= 1) {
        print_usage();
    }


    char executable_path[8192];
    snprintf(executable_path, sizeof(executable_path), "%s\\%s", argv[1], "witness64_d3d11.exe");
    String executable_data = read_entire_file(executable_path);

    printf("Executable data size: %zu\n", executable_data.count);

    // Process each patch
    for (size_t patch_index = 0; patch_index < sizeof(patches) / sizeof(patches[0]); patch_index++) {
        Patch* patch = &patches[patch_index];
        printf("Searching for patch: %s\n", patch->name);
        
        bool found_patch = false;
        for (size_t read_cursor = 0; read_cursor < executable_data.count - patch->search_bytes.count; read_cursor++) {
            bool match = true;
            for (size_t patch_cursor = 0; patch_cursor < patch->search_bytes.count; patch_cursor++) {
                if (executable_data.data[read_cursor + patch_cursor] != patch->search_bytes.data[patch_cursor]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                printf("Found patch '%s' at offset %zu\n", patch->name, read_cursor);
                found_patch = true;
                patch_offsets[patch_index] = read_cursor;
                break;
            }
        }
        
        if (!found_patch) {
            printf("Warning: Patch '%s' not found\n", patch->name);
        }
    }


    char* patched_data = malloc(executable_data.count);
    memcpy(patched_data, executable_data.data, executable_data.count);

    for (int patch_index = 0; patch_index < PATCH_COUNT; patch_index++) {
        if (patch_offsets[patch_index] != 0) {
            printf("Applying patch '%s' at offset %zu\n", patches[patch_index].name, patch_offsets[patch_index]);
            
            // Calculate where to start the replacement (end of search pattern - length of replacement)
            size_t replacement_start = patch_offsets[patch_index] + patches[patch_index].search_bytes.count - patches[patch_index].replacement_bytes.count;
            memcpy(patched_data + replacement_start, patches[patch_index].replacement_bytes.data, patches[patch_index].replacement_bytes.count);
        } else {
            printf("Warning: Patch '%s' not found\n", patches[patch_index].name);
        }
    }

    char output_path[8192];
    snprintf(output_path, sizeof(output_path), "%s\\%s", argv[1], "witness64_d3d11_patched.exe");
    FILE* output_file = fopen(output_path, "wb");
    if (output_file == NULL) {
        fprintf(stderr, "Failed to patch. Could not open '%s' for writing\n", output_path);
        exit(1);
    }
    
    fwrite(patched_data, 1, executable_data.count, output_file);
    fclose(output_file);
    
    printf("Patched executable saved to '%s'\n", output_path);
    free(patched_data);

    free(executable_data.data);
    return 0;
}
