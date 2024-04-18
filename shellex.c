/* $begin shellmain */
#include "csapp.h"
#define MAXARGS   128
#define DEFAULT_PROMPT "sh257"

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
void handle_help();
void handle_id(int n);
void handle_exit();
void handle_cd(char **argv);

/* Ignore SIGINT and jump back to next prompt */
sigjmp_buf buf;
void sigint_handler(int signum) {
    printf("\n");
    siglongjmp(buf, 1);
}

int main(int argc, char *argv[]) 
{
    int opt;
    char *prompt = DEFAULT_PROMPT; /* Sets prompt to sh257> */
    /* Sets custom prompt with -p option */
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                prompt = optarg;
                break;
            default:
                fprintf(stderr, "Usage: ./sh257 [-p prompt]\n"); 
                break;
        }
    }

    char cmdline[MAXLINE]; /* Command line */
    /* Install signal handler */
    Signal(SIGINT, sigint_handler);

    while (1) {
        if (sigsetjmp(buf, 1)  == 0) { /* Sets jump value */
            /* Read */
            printf("%s> ", prompt);                   
            Fgets(cmdline, MAXLINE, stdin); 
            if (feof(stdin))
                exit(0);

            /* Evaluate */
            eval(cmdline);
        }
    } 
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execv() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
        return;   /* Ignore empty lines */

    if (!builtin_command(argv)) { 
        if ((pid = Fork()) == 0) {   /* Child runs user job */
            if (execvp(argv[0], argv) < 0) {
                printf("Execution failed (in fork)\n");
                printf("%s: Command not found.\n", argv[0]);
                exit(1);
            }
        }

        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                unix_error("waitfg: waitpid error");
            }
            else { /* Prints process exit status */
                printf("Process exited with status code %d\n", WEXITSTATUS(status));
            } 
        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit")) { /* exit command */
        handle_exit();
        return 1;  
    }
    else if (!strcmp(argv[0], "&")) {    /* Ignore singleton & */
        return 1;
    }
    else if (!strcmp(argv[0], "help")) { /* help command */
        handle_help();
        return 1;
    }
    else if (!strcmp(argv[0], "pid")) { /* pid command */
        handle_id(0);
        return 1;
    }
    else if (!strcmp(argv[0], "ppid")) { /* ppid command */
        handle_id(1);
        return 1;
    }
    else if (!strcmp(argv[0], "cd")) { /* cd command */
        handle_cd(argv);
        return 1;
    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

/* Built-in command functions */
void handle_help() {
    printf(" _____ _   _ _____ _____ ________  ___  _____ _   _  _____ _      _\n");     
    printf("/  __ \\ | | /  ___|_   _|  _  |  \\/  | /  ___| | | ||  ___| |    | |\n");   
    printf("| /  \\/ | | \\ `--.  | | | | | | .  . | \\ `--.| |_| || |__ | |    | |\n");    
    printf("| |   | | | |`--. \\ | | | | | | |\\/| |  `--. \\  _  ||  __|| |    | |\n");    
    printf("| \\__/\\ |_| /\\__/ / | | \\ \\_/ / |  | | /\\__/ / | | || |___| |____| |____\n");
    printf(" \\____/\\___/\\____/  \\_/  \\___/\\_|  |_/ \\____/\\_| |_/\\____/\\_____/\\_____/\n");
    printf("____ ___  ___ _____ _____  _____  _____  ______\n");
    printf("|  ___|  _  | ___ \\ /  __ \\|  \\/  |/  ___/  __ \\/ __  \\|  ___||___  /\n");
    printf("| |_  | | | | |_/ / | /  \\/| .  . |\\ `--.| /  \\/`' / /'|___ \\    / /\n"); 
    printf("|  _| | | | |    /  | |    | |\\/| | `--. \\ |      / /      \\ \\  / /\n");  
    printf("| |   \\ \\_/ / |\\ \\  | \\__/\\| |  | |/\\__/ / \\__/\\./ /___/\\__/ /./ /\n");   
    printf("\\_|    \\___/\\_| \\_|  \\____/\\_|  |_/\\____/ \\____/\\_____/\\____/ \\_/\n");    
    printf("________________________________________________________________________________________\n");
    printf("< DEVELOPER: KHUONG NGUYEN >   < USAGE: ./sh257 [-p prompt] to set custom user prompt! >\n");
    printf("----------------------------------------------------------------------------------------\n");
    printf("     \\                    / \\  //\\ BUILTIN COMMANDS:	- exit: terminate shell process\n");
    printf("      \\     |\\___/|      /   \\//  \\			- pid: display shell process id\n");
    printf("            /0  0  \\__  /    //  | \\ \\    		- ppid: display shell's parent process id\n");
    printf("           /     /  \\/_/    //   |  \\  \\  		- help: display help menu\n");
    printf("           @_^_@'/   \\/_   //    |   \\   \\ 		- cd: use cd to display current directory\n");
    printf("           //_^_/     \\/_ //     |    \\    \\		      or cd [path] to change directory\n");
    printf("        ( //) |        \\///      |     \\     \\\n");
    printf("      ( / /) _|_ /   )  //       |      \\     _\\\n");
    printf("    ( // /) '/,_ _ _/  ( ; -.    |    _ _\\.-~        .-~~~^-.\n");
    printf("  (( / / )) ,-{        _      `-.|.-~-.           .~         `.\n");
    printf(" (( // / ))  '/\\      /                 ~-. _ .-~      .-~^-.  \\\n");
    printf(" (( /// ))      `.   {            }                   /      \\  \\\n");
    printf("  (( / ))     .----~-.\\        \\-'                 .~         \\  `. \\^-.\n");
    printf("             ///.----..>        \\             _ -~             `.  ^-`  ^-_\n");
    printf("               ///-._ _ _ _ _ _ _}^ - - - - ~                     ~-- ,.-~\n");
    printf("                                                                  /.-~\n");
    printf("SYSTEM COMMANDS:   refer to man pages for additional commands\n\n");
}

void handle_id(int n) {
    pid_t id;
    switch (n) { 
        /* Displays shell's process id */
        case 0: 
            id = getpid();
            printf("%d\n", id);
            break;
            /* Displays shell's parent process id */
        case 1: 
            id = getppid();
            printf("%d\n", id);
            break;
    }
}

void handle_exit() {
    /* Sends terminate signal to self */
    raise(SIGTERM);    
}

void handle_cd(char **argv) {
    char path[MAXARGS];
    /* If path is null then print out current directory */
    if (argv[1] == NULL) {
        getcwd(path, sizeof(path));
        printf("%s\n", path);
        return;
    }
    /* checks if path is valid */
    getcwd(path, sizeof(path));
    strcat(path, "/");
    strcat(path, argv[1]);
    if (access(path, F_OK) != 0) {
        printf("%s: No such file or directory\n", path);
        return;
    } 
    /* changes to directory */
    chdir(argv[1]);
}

