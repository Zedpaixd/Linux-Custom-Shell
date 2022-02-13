#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#define red "\033[31m"
#define green "\033[38;5;10m"
#define yellow "\033[33m"
#define blue "\033[34m"
#define magenta "\033[38;5;13m"
#define cyan "\033[36m"
#define reset "\033[0m"

static unsigned get_file_size(const char *file_name)
{
    struct stat sb;

    if (stat(file_name, &sb) != 0)
    {
        fprintf(stderr, "'stat' failed for '%s': %s.\n",
                file_name, strerror(errno));

        exit(EXIT_FAILURE);
    }

    return sb.st_size;
}

/* This routine reads the entire file into memory. */

char * read_whole_file(const char *file_name)
{
    FILE *f = fopen(file_name, "rb");

    fseek(f, 0, SEEK_END);

    long fsize = ftell(f);

    fseek(f, 0, SEEK_SET); /* same as rewind(f); */

    char *string = malloc(fsize + 1);

    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;

    return string;
}


char *get_name(char *path)
{
    if (path[strlen(path) - 1] == '/')
        
        return path;

    int j = strlen(path);

    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == 47)

            break;
       
        else

            j--;
    }

    return strdup(path + j);
}


bool is_dir(char *path)
{
    while (*(path++) != '\0')

        if (*(path) == 47)
            return true;

    return false;
}


int main(int argc, char **argv)
{

    
    if (argc < 3)
    {
        fprintf(stderr, red "Usage: mv {source} {dest} [options]" reset "\n");
        exit(EXIT_FAILURE);
    }

    int opt, nr_paths = 0;

    bool i_flag, t, S, b;

    char *target_dir = strdup(""),
         *suffix = strdup("~"),
         *cwd = (char *)malloc(sizeof(char) * (PATH_MAX)),
         **paths;

    getcwd(cwd, PATH_MAX);

    while ((opt = getopt(argc, argv, "it:S:b")) != -1)
    {
        switch (opt)
        {
            case 'i':

                i_flag = true;

                break;

            case 't':

                t = true;

                if (optarg == NULL)
                {
                    fprintf(stderr, red "If you use the -t argument you need to specify a directory!" reset "\n");
                    exit(EXIT_FAILURE);
                }

                target_dir = strdup(optarg);

                break;

            case 'S':

                S = true;
                b = true;

                if (optarg == NULL)
                {
                    fprintf(stderr, red "If you use the -S argument you need to specify a suffix!" reset "\n");
                    exit(EXIT_FAILURE);
                }

                suffix = strdup(optarg);

                break;

            case 'b':

                b = true;

                break;
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-' && strcmp(argv[i], suffix) != 0 && strcmp(argv[i], target_dir))
            nr_paths++;
    }

    paths = (char **)malloc(sizeof(char *) * (nr_paths) + 1);
    paths[nr_paths] = NULL;

    nr_paths = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-' && strcmp(argv[i], suffix) != 0 && strcmp(argv[i], target_dir))
        {
            paths[nr_paths] = strdup(argv[i]);
            nr_paths++;
        }
    }

    if (!t)
    {
        bool nr_modified = false;

        target_dir = strdup(paths[nr_paths - 1]);

        if (nr_paths > 2)
        {
            t = true;
            nr_modified = true;
            nr_paths--;
            paths[nr_paths] = NULL;
        }

        if (nr_paths == 2 && !nr_modified)
        {
            bool is_dest_dir = (strcmp(target_dir, get_name(target_dir)) == 0) && is_dir(target_dir);

            char *name;

            unsigned char *file;

            if (is_dest_dir)

                if (access(target_dir, F_OK) != 0)
                {
                    fprintf(stderr, red "Target directory: %s isn't accessible" reset "\n", target_dir);
                    exit(EXIT_FAILURE);
                }

            if (access(paths[0], F_OK) != 0)
            {
                fprintf(stderr, red "Target file at %s isn't accessible" reset "\n", paths[0]);
                exit(EXIT_FAILURE);
            }

            struct stat source_sb;

            stat(paths[0], &source_sb);

            if (!S_ISREG(source_sb.st_mode))
            {
                fprintf(stderr, red "%s is a directory, not a regular file" reset "\n", target_dir);
                exit(EXIT_FAILURE);
            }

            if (is_dest_dir)
            {
                file = read_whole_file(paths[0]);
                name = get_name(paths[0]);

                chdir(target_dir);

                bool file_exists = (access(name, F_OK) == 0);

                if (i_flag && file_exists)
                {
                    printf("overwrite '%s%s'? (y/n) ", target_dir, name);
                    char res;

                    while ((res = getchar()) != EOF && res != '\n')

                        if(!(tolower(res) == 'y'))
                        {
                            exit(EXIT_SUCCESS);
                        }
                }

                if (b && file_exists)
                {
                    char *backup_name = (char *)malloc(sizeof(char) * (strlen(suffix) + strlen(name) + 1));

                    strcpy(backup_name, name);
                    strcat(backup_name, suffix);

                    unsigned char *backup = read_whole_file(name);

                    FILE *fptr = fopen(backup_name, "wb");

                    fprintf(fptr, "%s", backup);
                    fclose(fptr);
                    
                    chmod(backup_name, source_sb.st_mode);
                }

                FILE *fptr = fopen(name, "w");

                fprintf(fptr, "%s", file);
                fclose(fptr);
                
                chmod(name, source_sb.st_mode);
                chdir(cwd);
                
                remove(paths[0]);
                
            }

            if (!is_dest_dir)
            {
                file = read_whole_file(paths[0]);
                name = strdup(target_dir);

                bool file_exists = (access(name, F_OK) == 0);

                if (i_flag && file_exists)
                {
                    printf("overwrite '%s'? (y/n) ", target_dir);

                    char res;

                    while ((res = getchar()) != EOF && res != '\n')

                        if(!(tolower(res) == 'y'))
                        {
                            exit(EXIT_SUCCESS);
                        }
                }

                if (b && file_exists)
                {
                    char *backup_name = (char *)malloc(sizeof(char) * (strlen(suffix) + strlen(name) + 1));

                    strcpy(backup_name, name);
                    strcat(backup_name, suffix);

                    unsigned char *backup = read_whole_file(name);

                    FILE *fptr = fopen(backup_name, "wb");
                    
                    fprintf(fptr, "%s", backup);
                    fclose(fptr);
                    
                    chmod(backup_name, source_sb.st_mode);
                }

                FILE *fptr = fopen(name, "w");

                fprintf(fptr, "%s", file);
                fclose(fptr);

                chmod(name, source_sb.st_mode);
                remove(paths[0]);
            }
        }
    }

    if (t)
    {
        
        if (access(target_dir, F_OK) != 0)
        {
            fprintf(stderr, red "Target directory: %s isn't accessible" reset "\n", target_dir);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nr_paths; i++)
        {
            if (access(paths[i], F_OK) != 0)
            {
                fprintf(stderr, red "Target file at %s isn't accessible" reset "\n", paths[i]);
                exit(EXIT_FAILURE);
            }
        }

        struct stat destsb;
        stat(target_dir, &destsb);

        if (!S_ISDIR(destsb.st_mode))
        {
            fprintf(stderr, red "Target directory: %s isn't a directory" reset "\n", target_dir);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nr_paths; i++)
        {
            struct stat tempsb;

            stat(paths[i], &tempsb);

            if (!S_ISREG(tempsb.st_mode))
            {
                fprintf(stderr, red "%s is a directory, not a regular file" reset "\n", target_dir);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < nr_paths; i++)
        {
            bool jump = false;

            struct stat file_sb;

            stat(paths[i], &file_sb);

            unsigned char *file = read_whole_file(paths[i]);

            char *name = get_name(paths[i]);

            chdir(target_dir);

            bool file_exists = (access(name, F_OK) == 0);

            if (i_flag && file_exists)
            {
                printf("overwrite '%s%s' (y/n)? ", target_dir, name);
                
                char res; 

                while ((res = getchar()) != EOF && res != '\n')

                    if(!(tolower(res) == 'y'))
                    {
                        chdir(cwd);
                        jump = true;
                    }
                
            }

            if (jump) 
                continue;

            if (b && file_exists)
            {   
                char *backup_name = (char *)malloc(sizeof(char) * (strlen(suffix) + strlen(name) + 1));

                strcpy(backup_name, name);
                strcat(backup_name, suffix);

                char *backup = read_whole_file(name);

                FILE *fptr = fopen(backup_name, "wb");

                fprintf(fptr, "%s", backup);
                fclose(fptr);

                chmod(backup_name, file_sb.st_mode);
            }

            FILE *fptr = fopen(name, "w");

            fprintf(fptr, "%s", file);
            fclose(fptr);

            chmod(name, file_sb.st_mode);
            chdir(cwd);

            remove(paths[i]);
        }
    }

    for (int i = 0; i < nr_paths; i++)
    {
        free(paths[i]);
    }

    free(target_dir);
    free(suffix);
    free(cwd);
    
    return EXIT_SUCCESS;
}