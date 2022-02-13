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

char *read_whole_file(const char *file_name)
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

/* This routine reads the entire file into memory. */

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
        fprintf(stderr, red "Usage: ./mvcopy {source} {dest} [options]" reset "\n");
        exit(EXIT_FAILURE);
    }

    int opt;
    bool i_flag = false, t = false, v = false;
    char *target_dir = strdup("");
    char *suffix = strdup("~");

    char *cwd = (char *)malloc(sizeof(char) * (PATH_MAX));
    getcwd(cwd, PATH_MAX);

    char *home;
    home = getenv("HOME");

    char *temp;

    for (int i = 0; argv[i] != NULL; i++)
    {
        if (argv[i][0] == '~')
        {   
            temp = malloc(sizeof(char *) * (strlen(argv[i]) + strlen(home) + 3));
            sprintf(temp, "%s%s", home, argv[i] + 1);
            sprintf(argv[i], "%s", temp);
        }
    }

    while ((opt = getopt(argc, argv, "vit:")) != -1)
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

        case 'v':

            v = true;
            break;
        }
    }

    char **paths;
    int nr_paths = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-' && strcmp(argv[i], target_dir) != 0)
            nr_paths++;
    }

    paths = (char **)malloc(sizeof(char *) * (nr_paths) + 1);
    paths[nr_paths] = NULL;
    nr_paths = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-'  && strcmp(argv[i], target_dir) != 0 )
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
            nr_paths--;
            nr_modified = true;
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
                    printf("copymv: overwrite '%s%s'? (y/n) ", target_dir, name);
                    char res;

                    while ((res = getchar()) != EOF && res != '\n')
                       
                        if (!(tolower(res) == 'y'))
                        {
                            exit(EXIT_SUCCESS);
                        }
                }

                if (v) 
                    printf("'%s' -> '%s%s'\n",paths[0],target_dir,name);

                FILE *fptr = fopen(name, "w");
                fprintf(fptr, "%s", file);
                fclose(fptr);

                chmod(name, source_sb.st_mode);
                chdir(cwd);
            }

            if (!is_dest_dir)
            {
                file = read_whole_file(paths[0]);
                name = strdup(target_dir);
                bool file_exists = (access(name, F_OK) == 0);
                
                if (i_flag && file_exists)
                {
                    printf("copymv: overwrite '%s'? ", target_dir);
                    char res;
                    
                    while ((res = getchar()) != EOF && res != '\n')
                        if (!(tolower(res) == 'y'))
                        {
                            exit(EXIT_SUCCESS);
                        }
                }

                if (v) 
                    printf("'%s' -> '%s'\n",paths[0],name);

                FILE *fptr = fopen(name, "w");
                fprintf(fptr, "%s", file);
                fclose(fptr);
                chmod(name, source_sb.st_mode);
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
            struct stat file_sb;
            bool jump = false;
            stat(paths[i], &file_sb);

            unsigned char *file = read_whole_file(paths[i]);
            char *name = get_name(paths[i]);
            chdir(target_dir);

            if (v) 
                printf("'%s' -> '%s%s'\n",paths[i],target_dir,name);

            bool file_exists = (access(name, F_OK) == 0);

            if (i_flag && file_exists)
            {
                printf("copymv: overwrite '%s%s'? (y/n) ", target_dir, name);
                char res;
                
                while ((res = getchar()) != EOF && res != '\n')
                    if (!(tolower(res) == 'y'))
                    {
                        chdir(cwd);
                        jump = true;
                    }
            }

            if (jump) 
                continue;

            FILE *fptr = fopen(name, "w");
            fprintf(fptr, "%s", file);
            fclose(fptr);

            chmod(name, file_sb.st_mode);
            chdir(cwd);
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