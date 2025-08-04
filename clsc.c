#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <locale.h>
#include <time.h>       // Requerido para se calcular o tempo de execução do programa.
#include <threads.h>    // Requerido pela biblioteca de threads da Linguagem C11.
#include <unistd.h>     // Requerido por fork. 
#include <dirent.h>     // Requerido para se trabalhar com diretórios.
#include <sys/wait.h>   // Requerido por wait.
#include "clsc.h"       // line count source code

// Cria o processo. Retorna  o PID do processo em caso de sucesso ou -1 se não foi possível criar o processo.
pid_t createProcess() {
    pid_t pid = fork();

    if (pid < 0) {
        printf(ERROR_CREATE_PROCESS);
        exit(EXIT_FAILURE);
    }
    
    return pid;
} // createProcess()

/* Procura a última ocorrência do (.) no nome do arquivo. Retorna NULL se não for encontrado nenhum (.), ou seja, não possui extensão.
   Compara o último ponto com a extensão (.c). Se orquivo tem a extensão (.c) retorna 1, caso contrário, retorna 0.*/
int checkExtension(const char *filename) {
    const char *dot = strrchr(filename, DOT); // Encontra a última ocorrência do (.).

    if (dot == NULL)  // Não há extensão no nome do arquivo.
        return 0; 
    if (strcmp(dot, C_EXTENSION) == 0) 
        return 1; // Encontrou a extensão.
    
    return 0; // Extensão inválida.
} // checkExtension()

// Verifica se o argumento passado pelo usuário é um diretório, retorna 1 se for um diretório e 0 caso contrário.
int isDirectory(const char *dirName){
    DIR *dir = opendir(dirName);
    if (dir != NULL) {
        closedir(dir);
        return 1; 
    }
    return 0;
} // isDirectory()

// Função que calcula a duração do programa em segundos.
double calculateDuration(time_t start, time_t end){
    return difftime(start, end);
}

// Função para imprimir a quantidade de arquivos e o resultado da soma das linhas.
void printLineCount(Result result){
    printf("%s- Código-fonte C%s\n", BOLD_ON, BOLD_OFF);
    printf("\tNº de Arquivos = %d\n", result.fileCount);
    printf("\tLinhas vazias = %d\n", result.totalEmpty);
    printf("\tLinhas de comentários = %d\n", result.totalComments);
    printf("\tLinhas de instruções = %d\n\n", result.totalInstructions);
} // printLineCount()

// Função para imprimir o tempo necessário para execução do programa.
void printExecutionTime(ExecutionTime t){
    printf("%s- Tempo%s\n", BOLD_ON, BOLD_OFF);
    printf("\tInício.: %s\n", t.start);
    printf("\tTérmino: %s\n", t.end);
    printf("\tDuração: %d segundos\n", t.duration);
} // printExecutionTime()

// Função para remover os espaços no começo e no final de cada linha.
void trimLine(char *line) {
    int start = 0, end = strlen(line) - 1;

    // Removendo os espaços ou tabulações do início da linha.
    while (line[start] == SPACE || line[start] == HORIZONTAL_TAB)
        start++;

    // Remove os espaços ou tabulações do final da linha.
    while (end >= start && (line[end] == SPACE || line[end] == HORIZONTAL_TAB || line[end] == NEWLINE))
        end--;

    // Ajusta a string para encerrar no ponto certo.
    int i, j = 0;
    for (i = start; i <= end; i++) {
        line[j++] = line[i];
    }
    line[j] = NULL_CHAR;  // Insere o terminador da string.
} // trimLine()

// Função para contar o número de linhas vazias, comentários e instruções de um código-fonte.
int countLines(void *arg){
    LineCounter *counter = (LineCounter*)arg;
    FILE *file = fopen(counter->filename, "r");

    char line[sizeof(file) * LINE_BUFFER_SIZE];
    char *endBlock = strstr(line, CLOSE_BLOCK_COMMENT);
    char *lineComment = strstr(line, LINE_COMMENT);
    int inBlock = 0; // Flag que identifica se iniciou um bloco de comentário.

    counter->instructions = 0;
    counter->comments = 0;
    counter->emptyLines = 0;

    if (file == NULL) {
        printf(ERROR_OPEN_FILE);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (line[len - 1] == NEWLINE) {
            line[len - 1] = NULL_CHAR;
        }
        trimLine(line);

        // Verificações em blocos de comentários.
        if (inBlock == 1 && strstr(line, CLOSE_BLOCK_COMMENT) != NULL) {
            if (endBlock != NULL && *(endBlock + 2) != NULL_CHAR) {
                counter->instructions++;
            }
            else if (endBlock != NULL && lineComment != NULL && *(endBlock + 2) == SLASH && *(endBlock + 3) == SLASH) {
                counter->comments++; 
            }
            else {
                counter->comments++;
            }
            inBlock = 0;
        }
        else if (inBlock == 1 && line[0] != ASTERISK) {
            counter->comments++;
        }
        else if (strstr(line, OPEN_BLOCK_COMMENT) != NULL && strstr(line, CLOSE_BLOCK_COMMENT) != NULL && strstr(line, LINE_COMMENT) != NULL) {
            counter->instructions++;
        }
        else if (line[0] == SLASH && line[1] == ASTERISK) {
            counter->comments++;
            if (strstr(line, CLOSE_BLOCK_COMMENT) == NULL) {
                inBlock = 1;
            }
        }
        else if (inBlock == 1 && strlen(line) == 0) {
            counter->comments++;
        }
        else if (strlen(line) == 0) {
            counter->emptyLines++;
        }
        else if (line[0] == SLASH && line[1] == SLASH) {
            counter->comments++;
        }
        else {
            counter->instructions++;
        }
    }

    fclose(file);
    thrd_exit(EXIT_SUCCESS);
} // countLines()

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes armazenando os nomes dos 
   arquivos que contenham a extensão (.c).*/
int processDirectory(const char *dirName, LineCounter *counters, Result *summary) {
    DIR *dir = opendir(dirName);

    if (dir == NULL) {
        printf(ERROR_OPEN_DIR);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, DIR_CURRENT) && strcmp(entry->d_name, DIR_PARENT)) {
            char fullPath[MAX_PATH_LENGTH] = {NULL_CHAR};
            strcat(fullPath, dirName);
            strcat(fullPath, "/");
            strcat(fullPath, entry->d_name);
            processDirectory(fullPath, counters, summary); 
        }
        else if (checkExtension(entry->d_name) == 1) {
            char fullPath[MAX_PATH_LENGTH] = {NULL_CHAR};
            strcat(fullPath, dirName);
            strcat(fullPath, "/");
            strcat(fullPath, entry->d_name);

            strcpy(counters[summary->fileCount].filename, fullPath);
            summary->fileCount++;
        }
    }

    closedir(dir);

    return EXIT_SUCCESS;
} // processDirectory()

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes verificando a existência de arquivos (.c). 
   Retorna a quantidade de arquivos encontrados.*/
int countFiles(const char *dirName){
    DIR *dir = opendir(dirName);
    int quantity = 0;

    if (dir == NULL) {
        printf(ERROR_OPEN_DIR);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, DIR_CURRENT) && strcmp(entry->d_name, DIR_PARENT)) {
            char fullPath[MAX_PATH_LENGTH] = {NULL_CHAR};
            strcat(fullPath, dirName);
            strcat(fullPath, "/");
            strcat(fullPath, entry->d_name);
            quantity += countFiles(fullPath);
        }
        else if (checkExtension(entry->d_name) == 1) {
            quantity++;
        }
    }

    closedir(dir);
    return quantity;
}

/* Função chamada quando o argumento for um diretório, cria as threads para cada arquivo, chamando countLines. Faz a soma das linhas vazias, 
   comentário e linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processFiles(LineCounter *counters, Result summary){
    thrd_t tid[summary.fileCount];

    for (int i = 0; i < summary.fileCount; i++) {
        thrd_create(&tid[i], countLines, &counters[i]);
    }
    for (int i = 0; i < summary.fileCount; i++) {
        thrd_join(tid[i], NULL);
    }

    for (int i = 0; i < summary.fileCount; i++) {
        summary.totalComments += counters[i].comments;
        summary.totalInstructions += counters[i].instructions;
        summary.totalEmpty += counters[i].emptyLines;
    }

    printLineCount(summary);
    return EXIT_SUCCESS;
}

/* Função chamada caso o argumento passado seja apenas um arquivo. Cria uma thread para realizar o cálculo de linhas vazias, comentários e
   linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processFile(const char *filename){
    thrd_t tid;
    LineCounter counter;
    Result summary;
    summary.fileCount = 1;

    strcpy(counter.filename, filename);

    thrd_create(&tid, countLines, &counter);
    thrd_join(tid, NULL);

    summary.totalComments = counter.comments;
    summary.totalInstructions = counter.instructions;
    summary.totalEmpty = counter.emptyLines;

    printLineCount(summary);
    return EXIT_SUCCESS;
} // processFile()

/* Com base no argumento recebido pela linha de comando verifica se é um diretório ou um arquivo válido e chama a função necessária para 
   execução do argumento. Pega o tempo inicial e final e calcula a duração de execução.*/
void childProcess(char *arg){
    Result summary = {0, 0, 0, 0};
    ExecutionTime timeInfo = {"", "", 0.0};

    time_t start, end;
    struct tm *info;

    time(&start);
    info = localtime(&start);
    strftime(timeInfo.start, sizeof(timeInfo.start), "%H:%M:%S", info);

    if (isDirectory(arg) == 1) {
        int quantity = countFiles(arg);
        LineCounter *counters = (LineCounter*)malloc(sizeof(LineCounter) * quantity);

        processDirectory(arg, counters, &summary);
        processFiles(counters, summary);
        free(counters);
    } else {
        if (checkExtension(arg) == 1)
            processFile(arg);
        else {
            printf(ERROR_INVALID_EXTENSION);
            exit(EXIT_FAILURE);
        }
    }

    time(&end);
    info = localtime(&end);
    strftime(timeInfo.end, sizeof(timeInfo.end), "%H:%M:%S", info);

    timeInfo.duration = calculateDuration(end, start);
    printExecutionTime(timeInfo);
} // childProcess()

// Lê o argumento fornecido na linha de comando, se a entrada for válida cria o processo filho e aguarda.
int clsc(int argc, char *argv[]){
    setlocale(LC_ALL, LANGUAGE);

    if (argc != 2) {
        printf(USAGE_EXAMPLE);
        return 1;
    }

    pid_t pid = createProcess();
    if (pid > 0) {
        int status;
        wait(&status);
    }
    else if (pid == 0) {
        childProcess(argv[1]);
    }
    else {
        exit(EXIT_FAILURE);
    }
} // clsc()

int main(int argc, char *argv[]){
    return clsc(argc, argv);
}