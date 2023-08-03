#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
//#include <sys/ioctl.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAXSIZE 1000

// gcc fp.c -o fp -lreadline
// gcc fp.c -o fp -lreadline -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-return-type

void console()
{
    printf("\n\n");
    printf("###############################################\n");
    printf("#      WELCOME TO MY BUDGET LINUX SHELL       #\n");
    printf("#             TO EXIT TYPE \"EXIT\"             #\n");
    printf("#                VERSION 2.0.7                #\n");
    printf("###############################################\n\n\n");
}

void printHelp()
{
    printf("Welcome to my shell!\n");
    printf("In here you can run any command you want\n");
    printf("However, the \"cp\",\"mv\" and \"dirname\" are implemented by me.\n");
    printf("Arguments implemented for CP: -i -t -v\n");
    printf("Arguments implemented for MV: -i -t -s (with -b)\n");
    printf("Besides that, you can use any amount of piping you'd like\n");
    printf("And of course, redirects. These being: > >> and <\n");
    printf("That being said, if you need more help\n");
    printf("with any command, I recommend running the\n");
    printf("\"man *commandName*\" command.\n");
    printf("Thank you and if the terminal breaks, do let me know!\n");
}

char commandHistory[MAXSIZE][MAXSIZE];
int commandHistoryAmount = 0;

int containsSlash(char *string)
{
    int counter = 0;

    for (size_t i = 0; i < strlen(string); i++)
    {
        if (string[i] == '/' /*|| string[i]=='\\'*/)
        {
            counter++;
        }
    }
    return counter;
}

int containsBSlash(char *string)
{
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (string[i] == 92)
        {
            return 1;
        }
    }
    return 0;
}

void mv(int argc, char **argv)
{
    int pid = fork();
    int status = 0;

    if (pid == 0)
    {
        if (execv("bin/mv", argv) < 0)
        {

            printf("Execution failed or the command doesn't exist on this system!\n");
            perror(*argv);
            exit(1);
        }
    }
    else
    {
        while (wait(&status) != pid)
            ;
    }
}

void cp(int argc, char **argv)
{
    int pid = fork();
    int status = 0;

    if (pid == 0)
    {
        if (execv("bin/cp", argv) < 0)
        {

            printf("Execution failed or the command doesn't exist on this system!\n");
            perror(*argv);
            exit(1);
        }
    }
    else
    {
        while (wait(&status) != pid)
            ;
    }
}

void removeTillBackslash(char *input, char *leftovers)
{
    int after = 0;

    if (containsSlash(input) > 0)
    {
        for (int i = strlen(input) - 1; i >= 0; i--)
        {
            if (/*input[i]!='\\' &&*/ input[i] != '/')
            {
                after++;
            }
            else
                break;
        }

        strcpy(leftovers, input + strlen(input) - after);
        input[strlen(input) - after - 1] = '\0';
    }
}

int countPipes(char *str)
{
    int pipes = 0;
    int inQuotes = 0;

    for (size_t i = 0; i < strlen(str); i++)
    {

        if (str[i] == '\"' && inQuotes == 0)
            inQuotes = 1;

        else if (str[i] == '\"' && inQuotes == 1)
            inQuotes = 0;

        if (str[i] == '|' && inQuotes == 0)
            pipes++;
    }
    return pipes;
}

int countSpaces(char *str)
{
    int counter = 0;

    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ')
            counter++;
    }
    return counter;
}

int countDashes(char *str)
{
    int counter = 0;

    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '-')
            counter++;
    }
    return counter;
}

void dirnameRemove(char* input)
{
    int Scounter = containsSlash(input);
    char *leftover = malloc(sizeof(char) * strlen(input) + 1);

    if (Scounter == 0)
        printf(".\n");

    else if (Scounter == 1 && input[strlen(input)-1] == '/')
        printf(".\n");

    else
    {
        removeTillBackslash(input, leftover);

        printf("%s\n", input);
    } 
}

void dirname(int argc, char **argv)
{
    for(int i=1; argv[i] != NULL; i++)
        dirnameRemove(argv[i]);
}

void printCommandHistory()
{
    for (int i = commandHistoryAmount - 1; i >= 0; i--)
    {
        printf("%d -> %s\n", i, commandHistory[i]);
    }
}

void updateCommandHistory(char *command)
{
    if (commandHistoryAmount == 0)
    {
        strcpy(commandHistory[commandHistoryAmount], command);
        commandHistoryAmount++;
    }

    else
    {
        for (int i = commandHistoryAmount; i > 0; i--)
        {
            strcpy(commandHistory[i], commandHistory[i - 1]);
        }
        strcpy(commandHistory[0], command);
        commandHistoryAmount++;
    }
}

void executeInternal(char *argumentsAsString, char **argv, bool *internalCommand)
{
    if (strcmp(argv[0], "dirname") == 0 || (strcmp(argv[0], "mv") == 0) || (strcmp(argv[0], "cp") == 0) || (strcmp(argv[0], "help") == 0) || (strcmp(argv[0], "history") == 0))
    {
        *internalCommand = true;
    }
    else
    {
        *internalCommand = false;
        return;
    }

    int argc;
    for (argc = 0; argv[argc] != NULL; argc++)
    {
    };

    int saved_stdout, saved_stdin, fd = -1;
    saved_stdout = dup(1);
    saved_stdin = dup(0);

    // fd = redir(argc,argv);

    int flag, pos = -1;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (argv[i + 1] != NULL)
            {
                pos = i;
                flag = 1;
                argv[i] = NULL;
            }
            else
            {
                printf("No redirect file specified.\n");
            }
        }
        else if (strcmp(argv[i], ">>") == 0)
        {
            if (argv[i + 1] != NULL)
            {
                pos = i;
                flag = 2;
                argv[i] = NULL;
            }
            else
            {
                printf("No redirect file specified.\n");
            }
        }
        else if (strcmp(argv[i], "<") == 0)
        {
            if (argv[i + 1] != NULL)
            {
                pos = i;
                flag = 3;
                argv[i] = NULL;
            }
            else
            {
                printf("No redirect file specified.\n");
            }
        }
    }

    if (flag == 1)
    {
        fd = open(argv[pos + 1], O_WRONLY | O_CREAT | O_TRUNC, 0);
        fchmod(fd, S_IROTH | S_IRGRP | S_IREAD | S_IWUSR | S_IRUSR);
        dup2(fd, 1);
        argv[pos] = NULL;
    }
    if (flag == 2)
    {
        fd = open(argv[pos + 1], O_APPEND | O_WRONLY | O_CREAT);
        fchmod(fd, S_IROTH | S_IRGRP | S_IREAD | S_IWUSR | S_IRUSR);
        dup2(fd, 1);
        argv[pos] = NULL;
    }
    if (flag == 3)
    {
        fd = open(argv[pos + 1], O_RDONLY);
        dup2(fd, 0);
        argv[pos] = NULL;
    }

    if (strcmp(argv[0], "dirname") == 0)
    {
        dirname(argc, argv);
        *internalCommand = true;
    }

    if (strcmp(argv[0], "mv") == 0)
    {
        mv(argc, argv);
        *internalCommand = true;
    }

    if (strcmp(argv[0], "cp") == 0)
    {
        cp(argc, argv);
        *internalCommand = true;
    }
    if (strcmp(argv[0], "history") == 0)
    {
        printCommandHistory();
        *internalCommand = true;
    }
    if (strcmp(argv[0], "help") == 0)
    {
        printHelp();
        *internalCommand = true;
    }

    /*if (fd != -1)*/
    close(fd);

    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
    close(saved_stdin);
    close(saved_stdout);
}

void handleCommand(char **argv, bool *internalCommand)
{
    int argc;
    for (argc = 0; argv[argc] != NULL; argc++)
    {
    };
    int saved_stdout, saved_stdin, fd;
    pid_t pid;
    saved_stdout = dup(1);
    saved_stdin = dup(0);

    // fd = redir(argc,argv);

    int flag = 0, pos;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            flag = 1;
            if (argv[i + 1] != NULL)
            {
                pos = i;
            }
            argv[i] = NULL;
            break;
        }
        if (strcmp(argv[i], ">>") == 0)
        {
            flag = 2;
            if (argv[i + 1] != NULL)
            {
                pos = i;
            }
            argv[i] = NULL;
            break;
        }
        if (strcmp(argv[i], "<") == 0)
        {
            flag = 3;
            if (argv[i + 1] != NULL)
            {
                pos = i;
            }
            argv[i] = NULL;
            break;
        }
    }

    if (flag == 1)
    {
        fd = open(argv[pos + 1], O_WRONLY | O_CREAT | O_TRUNC, 0);
        fchmod(fd, S_IROTH | S_IRGRP | S_IREAD | S_IWUSR | S_IRUSR);
        dup2(fd, 1);
        argv[pos] = NULL;
    }
    if (flag == 2)
    {
        fd = open(argv[pos + 1], O_APPEND | O_WRONLY | O_CREAT);
        fchmod(fd, S_IROTH | S_IRGRP | S_IREAD | S_IWUSR | S_IRUSR);
        dup2(fd, 1);
        argv[pos] = NULL;
    }
    if (flag == 3)
    {
        fd = open(argv[pos + 1], O_RDONLY);
        dup2(fd, 0);
        argv[pos] = NULL;
    }

    if ((pid = fork()) < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0)
    {
        if (strcmp(*argv, "dirname") == 0)
        {
            *internalCommand = true;
            executeInternal("",argv,internalCommand);
        }

        else if (strcmp(*argv, "mv") == 0)
        {
            *internalCommand = true;
            executeInternal("",argv,internalCommand);
        }

        else if (strcmp(*argv, "cp") == 0)
        {
            *internalCommand = true;
            executeInternal("",argv,internalCommand);
        }

        else if (execvp(*argv, argv) < 0)
        {

            printf("Execution failed or the command doesn't exist on this system!\n");
            perror(*argv);
            exit(1);
        }
    }
    else
    {
        while (wait(0) != pid)
            ;
    }

    /*if (fd != -1)*/
    close(fd);

    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
    close(saved_stdin);
    close(saved_stdout);
}

char strip(char *str)
{
    int before = 0, after = 0;

    char *backupstr = malloc(sizeof(char) * strlen(str) + 1);
    strcpy(backupstr, str);

    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ' || str[i] == '\t')
        {
            before++;
        }
        else
            break;
    }

    // memcpy(str,str+before,strlen(str)+before+1);
    strcpy(str, backupstr + before);

    for (int i = strlen(str) - 1; i >= 0; i--)
    {
        if (str[i] == ' ')
        {
            after++;
        }
        else
            break;
    }

    str[strlen(str) - after] = '\0';
}

void pipeParse(char *line, char **commands)
{
    char *lineBackup = malloc(sizeof(char) * strlen(line) + 1), *token;
    int pos = 0;

    strcpy(lineBackup, line);

    // printf("|%s| -> ",line);

    token = strtok(line, "|");

    while (token != NULL)
    {
        strip(token);

        commands[pos] = malloc(sizeof(char) * strlen(token) + 1);
        strcpy(commands[pos], token);

        pos++;
        token = strtok(NULL, "|");
    }

    free(token);
}

void argsParse(char *line, char *args[], int argc)
{
    char *line_copy2 = malloc(sizeof(char) * strlen(line) + 1);
    strcpy(line_copy2, line);

    // args[argc] = malloc(sizeof(char));
    args[argc] = NULL;

    int i = 0;

    char *token = strtok(line_copy2, "\n ");
    while (token != NULL)
    {

        args[i] = malloc(strlen(token) + 1);

        strip(token);
        strcpy(args[i], token);

        token = strtok(NULL, " ");
        i++;
    }

    free(token);
}

void splitIntoCommandsAndArgs(char **linesToSplit, int pipes, char ***Stuff)
{
    for (int i = 0; i < pipes; i++)
    {
        int argCounter = countSpaces(linesToSplit[i]) + 1;

        char *arguments[argCounter + 1];
        Stuff[i] = malloc(argCounter * sizeof(char *));

        argsParse(linesToSplit[i], arguments, argCounter);

        memcpy(Stuff[i], arguments, (argCounter + 1) * sizeof(char *));
    }
    Stuff[pipes] = NULL;
}

char ***parse(char *line)
{
    int pipes = countPipes(line) + 1;

    char **commands = malloc((pipes + 1) * sizeof(char *)),
         ***arrayOfStuff = malloc((pipes + 1) * sizeof(char *));

    commands[pipes] = NULL;

    pipeParse(line, commands);
    splitIntoCommandsAndArgs(commands, pipes, arrayOfStuff);

    return arrayOfStuff;
}

static void pipeline(char ***cmd, bool internalCommand, char *argumentsAsString, char **budgetargv)
{
    int fd[2], backup = 0;
    pid_t pid;
    while (*cmd != NULL)
    {
        pipe(fd);
        pid = fork();
        if (pid < 0)
        {
            printf("ERROR FORKING");
            exit(-1);
        }
        else if (pid == 0)
        {
            dup2(backup, 0);

            if (*(cmd + 1) != NULL)
            {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            handleCommand((*cmd), &internalCommand);
            exit(1);
        }
        else
        {
            wait(NULL);
            close(fd[1]);
            cmd++;
            backup = fd[0];
        }
    }
}
int exec_pipe(char *line, bool internalCommand, char *argumentsAsString, char **budgetargv)
{
    char ***commands = parse(line);
    pipeline(commands, internalCommand, argumentsAsString, budgetargv);
    free(commands);
}

int main(int argc, char **argv)
{
    bool fromCLI = argc > 1 ? true : false;
    if (!fromCLI) 
        console();

    char *input, *inputcopy, inputcopy2[MAXSIZE],
    *argumentsAsString, **budgetargv,
    inputMessage[MAXSIZE],
    cwd[256],*home;

    int budgetargc;

    bool internalCommand = false, 
    willToContinue = true, 
    fSlash = false, copyable = false;

    getcwd(cwd, sizeof(cwd));
    strcpy(inputMessage, "");

    home = getenv("HOME");

    sprintf(inputMessage, "\033[0;33mIN: %s\n\033[0;32m%s : \033[0m", cwd, "<Custom Shell Command>");

    while (willToContinue)
    {
        if (fromCLI)
            willToContinue = false;
        internalCommand = false;
        if (!fromCLI)
            input = readline(inputMessage);
        else
            input = strdup(argv[1]);

        if (strcmp(input, "") == 0)
        {
            printf("Input a command.\n");
        }

        else
        {

            inputcopy = malloc(strlen(input) + 1);

            strip(input);
            strcpy(inputcopy, input);

            updateCommandHistory(input);
            add_history(input);

            char *token;
            budgetargc = 0;

            token = strtok(inputcopy, "\n ");

            while (token != NULL)
            {
                budgetargc++;
                token = strtok(NULL, "\n ");
            }

            budgetargv = malloc((sizeof(char *)) * budgetargc + 1);
            budgetargv[budgetargc] = NULL;

            argumentsAsString = malloc(sizeof(char) + strlen(input) + 1 * (budgetargc - 1) + 1);
            inputcopy = strdup(input);

            budgetargc = 0;

            token = strtok(inputcopy, "\n ");
            while (token != NULL)
            {
                bool esc = containsBSlash(token);
                if (esc)
                {
                    char *right_token;

                    right_token = strdup(token);
                    right_token[strlen(token) - 1] = '\0';

                    token = strtok(NULL, "\n ");

                    if (token != NULL)
                    {
                        char *temp = malloc(sizeof(char) * (strlen(right_token) + strlen(token) + 1));

                        strcpy(temp, right_token);
                        strcat(temp, " ");
                        strcat(temp, token);

                        budgetargv[budgetargc] = strdup(temp);

                        budgetargc++;

                        free(temp);

                        token = strtok(NULL, "\n ");
                    }

                    else
                    {
                        printf("<< After a forward slash you may wanna add a space when writing paths. >>\n");
                    }
                }
                if (!esc)
                {
                    budgetargv[budgetargc] = strdup(token);
                    
                    budgetargc++;

                    token = strtok(NULL, "\n ");
                }
            }

            budgetargv[budgetargc] = NULL;

            if (budgetargc > 1)
            {
                for (int i = 1; i < budgetargc; i++)
                {
                    strcat(argumentsAsString, " ");
                    strcat(argumentsAsString, budgetargv[i]);
                }

                strip(argumentsAsString);

            }

            for (int i = 1; i < budgetargc; i++)
            {
                if (budgetargv[i][0] == '~')
                {
                    char *argvcopy;

                    argvcopy = strdup(budgetargv[i] + 1);

                    budgetargv[i] = malloc((strlen(argvcopy) + strlen(home) + 1) * sizeof(char));

                    sprintf(budgetargv[i], "%s%s", home, argvcopy);
                }
            }

            if (strcmp(budgetargv[0], "exit") == 0)
            {
                printf("EXITED SHELL\n");
                system("clear");

                return 0;
            }

            if (countPipes(input) > 0)
            {
                exec_pipe(input, internalCommand, argumentsAsString, budgetargv);

                continue;
            }
            executeInternal(argumentsAsString, budgetargv, &internalCommand);

            if (internalCommand == false)
            {
                handleCommand(budgetargv,&internalCommand);
            }
        

            free(input);
            free(inputcopy);

            for (int i = 0; budgetargv[i + 1] != NULL; i++)
            {
                free(budgetargv[i]);
            }

        }
    }
}
