#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// thread implementation
#include <pthread.h>

// read dirs
#include <dirent.h>

// MUpdf
#include <mupdf/fitz.h>

#define TARGET_SIZE 256
#define PATH_SIZE 1024

// get target, write to target
char* get_Target(void) {
    char *target = malloc(TARGET_SIZE);
    if (!target) {
        perror("Failed to allocate memory for target");
        return NULL;
    }

    printf("Enter target string to search for in PDFs: ");
    if (fgets(target, TARGET_SIZE, stdin) == NULL) {
        free(target);
        return NULL;
    }

    // Remove trailing newline
    target[strcspn(target, "\n")] = '\0';

    return target;
}

// get folder from cwd
char* get_Folder(void) {
    char *folder = malloc(TARGET_SIZE);
    if (!folder) {
        perror("Failed to allocate memory for folder");
        return NULL;
    }

    printf("Enter folder name for searching: ");
    if (fgets(folder, TARGET_SIZE, stdin) == NULL) {
        perror("Failed to read folder name");
        free(folder);
        return NULL;
    }

    // Remove trailing newline
    folder[strcspn(folder, "\n")] = '\0';

    char *cwd = malloc(PATH_SIZE);
    if (!cwd) {
        perror("Failed to allocate memory for cwd");
        free(folder);
        return NULL;
    }

    if (getcwd(cwd, PATH_SIZE) == NULL) {
        perror("Failed to get current working directory");
        free(folder);
        free(cwd);
        return NULL;
    }

    // Build full path
    size_t needed = strlen(cwd) + 1 + strlen(folder) + 1;
    if (needed > PATH_SIZE) {
        fprintf(stderr, "Path too long\n");
        free(folder);
        free(cwd);
        return NULL;
    }

    strcat(cwd, "/");
    strcat(cwd, folder);

    free(folder);
    return cwd;
}

static void quiet_warnings(void *user, const char *message) {
   (void)user;
   (void)message;
}

int read_PDF(const char *file_path, const char *target) {
    int match_count = 0;
    fz_context *ctx = NULL;
    fz_document *doc = NULL;

    ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) {
        fprintf(stderr, "Failed to create MuPDF context.\n");
        return -1;
    }

    fz_try(ctx) {

   // stop broken xref subsection warnings
   // this is just the pdf being scanned improperly
   fz_set_warning_callback(ctx, quiet_warnings, NULL);



        fz_register_document_handlers(ctx);
        doc = fz_open_document(ctx, file_path);

        int page_count = fz_count_pages(ctx, doc);

        for (int i = 0; i < page_count; i++) {
            fz_page *page = NULL;
            fz_stext_page *text_page = NULL;
            fz_buffer *buf = NULL;
            fz_output *out = NULL;

            fz_try(ctx) {
                page = fz_load_page(ctx, doc, i);
                text_page = fz_new_stext_page_from_page(ctx, page, NULL);

               // open buffer
                buf = fz_new_buffer(ctx, 4096);
                out = fz_new_output_with_buffer(ctx, buf);

               // put text in buffer
                fz_print_stext_page_as_text(ctx, out, text_page);
                fz_close_output(ctx, out);

                // append stop to eof
                fz_append_byte(ctx, buf, 0);

                unsigned char *data = NULL;
                fz_buffer_storage(ctx, buf, &data);

                 // search for match, case insensitive
                if (data) {
                    char *text = (char *)data;
                    char *ptr = text;
                    size_t target_len = strlen(target);

                    while ((ptr = strcasestr(ptr, target)) != NULL) {
                        match_count++;
                        ptr += target_len;
                    }
                }
            }

            fz_always(ctx) {
                fz_drop_output(ctx, out);
                fz_drop_buffer(ctx, buf);
                fz_drop_stext_page(ctx, text_page);
                fz_drop_page(ctx, page);
            }

            fz_catch(ctx) {
                fprintf(stderr, "Error processing page %d\n", i);
            }
        }
    }

    fz_catch(ctx) {
        fprintf(stderr, "Error: Cannot open document %s\n", file_path);
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        return -1;
    }

    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);

    return match_count;
}


int main(void) {
    char *target = get_Target();
    if (!target) return 1;
    printf("Target: %s\n", target);

    char *folder = get_Folder();
    if (!folder) {
        free(target);
        return 1;
    }
    printf("Folder: %s\n", folder);

    DIR *dr = opendir(folder);
    if (!dr) {
        perror("Failed to open directory");
        free(target);
        free(folder);
        return 1;
    }

    printf("Reading PDFs in folder: %s\n", folder);

   pthread_t thread_id;
   int ret = 0;

   ret = pthread_create(&thread_id, NULL, (void *)read_PDF, NULL);
   if (ret) {
       fprintf(stderr, "Error creating thread\n");
      closedir(dr);
      free(target);
      free(folder);
       return 1;
   }

    struct dirent *en;
    char file_path[PATH_SIZE];
    int total_matches = 0;

    while ((en = readdir(dr)) != NULL) {
        // Skip non-PDF files
        size_t len = strlen(en->d_name);
        if (len < 4 || strcasecmp(en->d_name + len - 4, ".pdf") != 0) {
            continue;
        }

        // Build full file path
        snprintf(file_path, sizeof(file_path), "%s/%s", folder, en->d_name);

        int matches = read_PDF(file_path, target);
        if (matches > 0) {
            printf("  Found %d matches in %s\n", matches, en->d_name);
            total_matches += matches;
        }
    }

    pthread_join(thread_id, NULL);
    printf("\nTotal matches found: %d\n", total_matches);

    closedir(dr);
    free(target);
    free(folder);

    return 0;
}


