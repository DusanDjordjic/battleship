#include <include/game_results.h>

error_code game_results_load(Vector* results, const char* filepath) {
    FILE* file = fopen(filepath, "ab+");;
    if (file == NULL) {
        return ERR_UNKNOWN;
    }

    fseek(file, 0, SEEK_END);
    int64_t isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }
    
    uint64_t size = (uint64_t)isize; 

    fseek(file, 0, SEEK_SET);

    // Check if fseek failed
    isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    if (isize != 0) {
        fprintf(stderr, RED "ERROR: After calling fseek to set cursor at the beginning ftell returned %ld\n" RESET, isize);
        fclose(file);
        return ERR_UNKNOWN;
    }

    uint64_t results_count = size / sizeof(game_results_t);
    if (results_count == 0) {
        fprintf(stdout, "No game results to load, skipping...\n");
        vector_create(results, sizeof(game_results_t));
        fclose(file);
        return ERR_NONE;
    }

    // allocate memory for all game results
    results->elements = malloc(sizeof(game_results_t) * results_count);
    if (results->elements == NULL) {
        fclose(file);
        return ERR_ALLOC;
    }

    results->element_size = sizeof(game_results_t); 
    results->logical_length = results_count;
    results->actual_length = results_count;

    uint64_t count = fread(results->elements, sizeof(game_results_t), results_count, file);
    if (ferror(file)) {
        fprintf(stderr, RED "ERROR: Failed to read all game results: expected %lu, read %lu\n" RESET, results_count, count);
        fclose(file);
        return ERR_UNKNOWN;
    }

    if (count == results_count) {
        fprintf(stdout, "Loaded all %lu game results, total file size %lu\n" RESET, results_count, size);
        fclose(file);
        return ERR_NONE;
    }

    // We didn't reach the end of file 
    fprintf(stderr, RED "ERROR: Didn't reach the end of file, game results to read: expected %lu, read %lu\n" RESET, results_count, count);
    fclose(file);
    return ERR_UNKNOWN;
}

error_code game_results_save(Vector* results, const char* filepath) {
    FILE* file = fopen(filepath, "w");;
    if (file == NULL) {
        return ERR_UNKNOWN;
    }

    fseek(file, 0, SEEK_SET);
    int64_t isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    fwrite(results->elements, sizeof(game_results_t), results->logical_length, file);
    if (ferror(file)) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    fprintf(stdout, "Saved %u game results\n" RESET, results->logical_length);

    fclose(file);
    return ERR_NONE;
}
