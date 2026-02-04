#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// read dirs
#include <dirent.h>

// MUpdf
#include <mupdf/fitz.h>

#define TARGET_SIZE 256

// get target, write to target
char* get_Target(char *target) {

   target = malloc(sizeof(*target * TARGET_SIZE));
   printf("Enter target string to search for in PDFs: ");
   fgets(target, sizeof(target) * TARGET_SIZE, stdin);

   return target;
}

// get folder from cwd
char* get_Folder(char *folder) {
    // Allocate memory for 'folder'
    folder = malloc(TARGET_SIZE);
    if (folder == NULL) {
        perror("Failed to allocate memory for folder");
        return NULL;
    }

    // Prompt the user for input
    printf("Enter folder name for searching: ");
    if (fgets(folder, TARGET_SIZE, stdin) == NULL) {
        perror("Failed to read folder name");
        free(folder); // Free allocated memory on failure
        return NULL;
    }

    // Remove the trailing newline from user input (if any)
    size_t len = strlen(folder);
    if (len > 0 && folder[len - 1] == '\n') {
        folder[len - 1] = '\0';
    }

    // Get current working directory
    char *cwd = malloc(TARGET_SIZE);
    if (cwd == NULL) {
        perror("Failed to allocate memory for cwd");
        free(folder); // Free 'folder' on failure
        return NULL;
    }
    if (getcwd(cwd, TARGET_SIZE) == NULL) {
        perror("Failed to get current working directory");
        free(folder);
        free(cwd);
        return NULL;
    }

    // Concatenate the path
    size_t cwd_space = TARGET_SIZE - strlen(cwd) - 1;
    if (strlen(folder) + 1 > cwd_space) {
        fprintf(stderr, "Folder name is too long to concatenate with cwd\n");
        free(folder);
        free(cwd);
        return NULL;
    }

    // Add slash and folder name to cwd
    strncat(cwd, "/", cwd_space);
    strncat(cwd, folder, TARGET_SIZE - strlen(cwd) - 1);

    // Clean up
    free(folder); // Free folder only if `cwd` takes over its role
    return cwd;   // Return concatenated path
}

// find matches in file based on target
char* find_Match(char* file_path, char* target) {
   char* match;

   match = malloc(sizeof(target));

   FILE *file;
   file = fopen(file_path, "r");

   if (file == NULL) perror("Error");

   return match;
}


int read_PDF(char* file_path, char* target) {
   int match_count = 0;
   fz_context *ctx = NULL;
   fz_document *doc = NULL;
   int page_count = 0;

   // 1. Create a context to hold the exception stack and various caches.
   // The last argument FZ_STORE_DEFAULT manages resource storage.
   ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
   if (!ctx) {
     fprintf(stderr, "Failed to create MuPDF context.\n");
     return 1;
   }

   // 2. Register standard document handlers (PDF, XPS, EPUB, etc.)
   fz_register_document_handlers(ctx);

   // Use fz_try/fz_catch for error handling in the MuPDF library
   fz_try(ctx) {
     // 3. Open the document
     doc = fz_open_document(ctx, file_path);

     // 4. Retrieve the number of pages
     page_count = fz_count_pages(ctx, doc);
     printf("Successfully opened document: %s\n", file_path);
     printf("Total pages: %d\n", page_count);

     // You can now proceed to load pages, render them, extract text, etc.
     // For example: fz_load_page(ctx, doc, page_number);

      fz_read_file(ctx, file_path);


   } fz_catch(ctx) {
     fprintf(stderr, "Error: Cannot open document %s\n", file_path);
     // An error occurred; the context's error stack is caught here.
   }

   // 5. Clean up resources
   if (doc) {
     fz_drop_document(ctx, doc);
   }
   if (ctx) {
     fz_drop_context(ctx);
   }

   return 0;
}


int main(void) {

   char *target = NULL;
   target = get_Target(target);
   printf("Target: %s", target);

   char *folder = NULL;
   folder = get_Folder(folder);
   printf("Folder: %s", folder);

   DIR *dr;           // Directory
   struct dirent *en; // entry of directory

   dr = opendir(folder);

   if (dr == NULL) {
      printf("Failed to open directory\n");
      return 1;
   }

   read_PDF(en->d_name, target);

   // // print dir
   // while ((en = readdir(dr)) != NULL) {
   //    printf("%s\n", en->d_name);
   // }

   closedir(dr);

   return 0;
}


