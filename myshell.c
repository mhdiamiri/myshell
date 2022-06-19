#include "myshell.h"

void shell_main_loop(){
    char *line;
    char **args;
    int status;
    signal(SIGINT, SIG_IGN);
    do{
        printf("> ");
        line = read_line();
        if(!line) break;
        args = split_lines(line);
        if(!args) break;
        status = execute_command(args, -1);
        free(line);
        free(args);
    } while(1);
}

char* read_line(){        
    char *line = NULL;
    line = (char*) malloc(2048 * sizeof(char));
    int iterator = 0;
    while(1){
        char c = getchar();
        if(c == ';' || c == '\n') break;
        line[iterator++] = c;
    }
    line[iterator] = 0;
    return line;
}

char** split_lines(char *line){       
    int bufsize = BUFFER_LEN, position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;
    token = strtok(line, " \t\r\n\a");
    while(token != NULL){
        tokens[position++] = token;
        if(position > bufsize){
            printf("Command line too long.\n");
            return NULL;
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

int execute_command(char** args, int in){
    if (!args || !args[0])
        return EXIT_SUCCESS;
    int status = 0;
    int index = 0;
    bool bg = false;
    for (int i = 0; args[i]; i++)
        if (!strcmp(args[i], "&"))
        {
            bg = true;
            index = i;
            break;
        }
    int saved_in = dup(0);
    int saved_out = dup(1);
    int fd[2] = {in, -1};

    int pipe_pos;
    for (pipe_pos = 0; args[pipe_pos]; pipe_pos++)
        if (!strcmp(args[pipe_pos], "|"))
            break;
    int pipe_fd[2] = {-1, -1};
    errno = 0;
    if (args[pipe_pos])
    {
        if (pipe(pipe_fd))
            perror("shell: pipe");
        if (dup2(pipe_fd[1], 1) == -1)
            perror("shell: pipe: redirect");
    }

    char **left_cmd;
    if (args[pipe_pos])
    {
        args[pipe_pos] = 0;
        left_cmd = args + pipe_pos + 1;
    }
    else
        left_cmd = 0;

    errno = 0;
    bool valid = true;

    if (pipe_fd[1] != -1)
        close(pipe_fd[1]);
    if (fd[0] != -1)
        close(fd[0]);
    if (fd[1] != -1)
        close(fd[1]);

    dup2(saved_in, 0);
    dup2(saved_out, 1);

    if (valid)
        execute_command(left_cmd, pipe_fd[0]);

    if (pipe_fd[0] != -1)
        close(pipe_fd[0]);


    if (valid){
        if (fd[0] != -1)
            dup2(fd[0], 0);
        if (fd[1] != -1)
            dup2(fd[1], 1);

        if (!strcmp(args[0], "exit"))
            exit(EXIT_SUCCESS);
        else if (!strcmp(args[0], "cd"))
            status = shell_cd(args);
        else{
            if (bg){
                char **args2 = malloc(index * sizeof(char*));
                for (int i = 0; i < index; i++){
                    args2[i] = malloc(BUFFER_LEN * sizeof(char));
                    args2[i] = args[i];
                }
                int j = 0;
                for(int i = index + 1; args[i]; i++, j++)
                    args[j] = args[i];
                args[j] = 0;

                status = lunch(args2, true);
                execute_command(args ,-1);
                return status;
            }else
                status = lunch(args, false);
        }
    }

    return status;
}
pid_t pid1, wpid;
void  controlC(int sig){
    kill(pid1, SIGINT);
    printf("> ");
}
int lunch(char** args, bool bg){
    int status = EXIT_SUCCESS;
    int pid = fork(); 
    
    if(pid == 0){       //child process
        status = EXIT_SUCCESS;
        pid1 = pid;
        if (bg) printf("background process [%d] started.\n", pid1);
        int s = execvp(args[0], args);
        if (bg) printf("\nbackground process [%d] terminated.\n", pid1);
        if (s == -1) {
            perror("shell");
            status = EXIT_FAILURE;
        }
        exit(status);
    }
    else if (pid < 0) {

    }
    else{
        // Parent process
        if (!bg){
            signal(SIGINT, controlC);
            do{
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            signal(SIGINT, SIG_IGN);        
        }
    }
    
    return status;
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        return EXIT_FAILURE;
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell: cd");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}