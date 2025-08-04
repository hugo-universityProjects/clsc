#include <sys/types.h> // Requrerido por pid_t.

// Constantes para deixar os tópicos da saída em negrito.
#define BOLD_ON "\x1b[1m"
#define BOLD_OFF "\x1b[0m"

// Idioma escolhido.
#define LANGUAGE "pt-BR"

// Caracteres utilizados na função contaLinhas, onde se substitui o '\n' por '\0'.
#define NULL_CHAR '\0'
#define NEWLINE '\n'

// Caracteres usados para as excessões na contagem das linhas.
#define SLASH '/'
#define ASTERISK '*'
#define LINE_COMMENT "//"
#define OPEN_BLOCK_COMMENT "/*"
#define CLOSE_BLOCK_COMMENT "*/"

// Caracteres separadores de strings: espaço e tabulação horizontal.
#define SPACE ' '
#define HORIZONTAL_TAB '\t'

// Caracteres usados na identificação da extensão desejada.
#define DOT '.'
#define C_EXTENSION ".c"

// Identificação dos diretórios que não devem ser considerados na contagem de diretórios e subdiretórios.
#define DIR_CURRENT "."
#define DIR_PARENT ".."

// Mensagens de possíveis erros do programa.
#define ERROR_CREATE_PROCESS "\nErro: não foi possível criar o processo!\n\n"
#define ERROR_OPEN_FILE "\nErro: não foi possível abrir o arquivo!\n\n"
#define ERROR_OPEN_DIR "\nErro: não foi possível abrir o diretório!\n\n"
#define ERROR_INVALID_EXTENSION "\nErro: o arquivo contém uma extensão inválida!\n\n"

// Mensagem explicativa do uso correto do comando.
#define USAGE_EXAMPLE "\nExemplo de Uso: %s <nomeArquivo ou nomeDiretório>\n\n", argv[0]

// Tamanho máximo do caminho de diretórios e arquivos.
#define MAX_PATH_LENGTH 512

// Tamanho máximo dos nomes dos arquivos.
#define MAX_FILENAME_LENGTH 256

// Tamanho usado para formatar a saída de tempo inicial e final.
#define TIME_FORMAT_LENGTH 9

// Tamanho usado para definir a capacidade de armazenamento do buffer.
#define LINE_BUFFER_SIZE 5000

// Estrutura que armazena o nome de cada arquivo e a contagem de linhas de cada um dos arquivos.
typedef struct {
   char filename[MAX_FILENAME_LENGTH];
   unsigned instructions,
            comments,
            emptyLines;
} LineCounter;

// Estrutura que armazena a soma de linhas vazias, comentários e instruções dos arquivos.
typedef struct {
   int fileCount;
   unsigned totalEmpty,
            totalComments,
            totalInstructions;
} Result;

// Estrutura que armazena o tempo inicial, final e a duração do programa.
typedef struct{
   char start[TIME_FORMAT_LENGTH];
   char end[TIME_FORMAT_LENGTH];
   unsigned duration;
} ExecutionTime;

// Cria o processo. Retorna o PID do processo em caso de sucesso ou -1 se não foi possível criar o processo.
pid_t createProcess();

/* Procura a última ocorrência do (.) no nome do arquivo. Retorna NULL se não for encontrado um (.).
   Compara o último ponto com a extensão .c. Se orquivo tem a extensão .c retorna 1, caso contrário, retorna 0. */
int checkExtension(const char *filename);

// Verifica se o argumento passado pelo usuário é um diretório, retorna 1 se for um diretório e 0 caso contrário.
int isDirectory(const char *dirName);

// Função que calcula a duração do programa em segundos.
double calculateDuration(time_t start, time_t end);

// Função para imprimir a quantidade de arquivos e o resultado da soma das linhas.
void printResult(Result r);

// Função para imprimir o tempo necessário para execução do programa.
void printExecutionTime(ExecutionTime t);

// Função para remover os espaços no começo e no final de cada linha.
void trimLine(char *line);

// Função para contar o número de linhas vazias, comentários e instruções de um código-fonte.
int countLines(void *arg);

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes armazenando os nomes dos 
   arquivos que contenham a extensão (.c).*/
int processDirectory(const char *dirName, LineCounter *fileCounters, Result *summary);

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes verificando a existência de arquivos (.c). 
   Retorna a quantidade de arquivos encontrados.*/
int countFiles(const char *dirName);

/* Função chamada quando o argumento for um diretório, cria as threads para cada arquivo, chamando countLines. Faz a soma das linhas vazias, 
   comentário e linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processFiles(LineCounter *fileCounters, Result summary);

/* Função chamada caso o argumento passado seja apenas um arquivo. Cria uma thread para realizar o cálculo de linhas vazias, comentários e
   linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processFile(const char *filename);

/* Com base no argumento recebido pela linha de comando verifica se é um diretório ou um arquivo válido e chama a função necessária para 
   execução do argumento. Pega o tempo inicial e final e calcula a duração de execução.*/
void childProcess(char *argv);

// Lê o argumento fornecido na linha de comando, se a entrada for válida cria o processo filho e aguarda.
int clsc(int argc, char *argv[]);