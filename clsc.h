#include <sys/types.h> // Requrerido por pid_t.

// Constantes para deixar os tópicos da saída em negrito.
#define ABRE_NEGRITO "\x1b[1m"
#define FECHA_NEGRITO "\x1b[0m"

// Idioma escolhido.
#define PORTUGUES_BRASIL "pt-BR"

// Caracteres utilizados na função contaLinhas, onde se substitui o '\n' por '\0'.
#define CARACTERE_NULO '\0'
#define FIM_DE_LINHA '\n'

// Caracteres usados para as excessões na contagem das linhas.
#define BARRA '/'
#define ASTERISCO '*'
#define COMENTARIO_DE_LINHA "//"
#define ABRE_BLOCO_COMENTARIO "/*"
#define FECHA_BLOCO_COMENTARIO "*/"

// Caracteres separadores de strings: espaço e tabulação horizontal.
#define ESPACO ' '
#define TABULACAO_HORIZONTAL '\t'

// Caracteres usados na identificação da extensão desejada.
#define PONTO '.'
#define EXTENSAO ".c"

// Identificação dos diretórios que não devem ser considerados na contagem de diretórios e subdiretórios.
#define DIRETORIO_PONTO "."
#define DIRETORIO_PONTO_PONTO ".."

// Mensagens de possíveis erros do programa.
#define ERRO_CRIA_PROCESSO "\nErro: não foi possível criar o processo!\n\n"
#define ERRO_ABRE_ARQUIVO "\nErro: não foi possível abrir o arquivo!\n\n"
#define ERRO_ABRE_DIRETORIO "\nErro: não foi possível abrir o diretório!\n\n"
#define ERRO_EXTENSAO_INVALIDA "\nErro: o arquivo contém uma extensão inválida!\n\n"

// Mensagem explicativa do uso correto do comando.
#define EXEMPLO_USO "\nExemplo de Uso: %s <nomeArquivo ou nomeDiretório>\n\n", argv[0]

// Tamanho máximo do caminho de diretórios e arquivos.
#define TAMANHO_MAX_CAMINHO 512

// Tamanho máximo dos nomes dos arquivos.
#define TAMANHO_MAX_NOME_ARQUIVO 256

// Tamanho usado para formatar a saída de tempo inicial e final.
#define TAMANHO_FORMATO_TEMPO 9

// Tamanho usado para definir a capacidade de armazenamento do buffer.
#define TAMANHO_BUFFER_LINHA 5000

// Estrutura que armazena o nome de cada arquivo e a contagem de linhas de cada um dos arquivos.
typedef struct { 
   char nomeArq[TAMANHO_MAX_NOME_ARQUIVO];
   unsigned instrucoes,
            comentarios,
            linhasVazias;
}Contador;

// Estrutura que armazena a soma de linhas vazias, comentários e instruções dos arquivos.
typedef struct {
   int qntdArquivos;
   unsigned totalVazias,
            totalComentarios,
            totalInstrucoes;
}Resultado;

// Estrutura que armazena o tempo inicial, final e a duração do programa.
typedef struct{
   char inicio[TAMANHO_FORMATO_TEMPO];
   char termino[TAMANHO_FORMATO_TEMPO];
   unsigned duracao;
}TempoExecucao;

// Cria o processo. Retorna  o PID do processo em caso de sucesso ou -1 se não foi possível criar o processo.
pid_t criarProcesso();

/* Procura a última ocorrência do (.) no nome do arquivo. Retorna NULL se não for encontrado um (.).
   Compara o último ponto com a extensão .c. Se orquivo tem a extensão .c retorna 1, caso contrário, retorna 0. */
int verificaExtensao(const char *nomeArq);

// Verifica se o argumento passado pelo usuário é um diretório, retorna 1 se for um diretório e 0 caso contrário.
int verificaDiretorio(const char *nomeDir);

// Função que calcula a duração do programa em segundos.
double calculaDuracao(time_t inicio, time_t fim);

// Função para imprimir a quantidade de arquivos e o resultado da soma das linhas.
void saidaContagem(Resultado d);

// Função para imprimir o tempo necessário para execução do programa.
void saidaTempo(TempoExecucao t);

// Função para remover os espaços no começo e no final de cada linha.
void removerEspacos(char *linha);

// Função para contar o número de linhas vazias, comentários e instruções de um código-fonte.
int contaLinhas(void *arg);

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes armazenando os nomes dos 
   arquivos que contenham a extensão (.c).*/
int processaDiretorio(const char *nomeDir, Contador *contagemArquivos, Resultado *dados);

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes verificando a existência de arquivos (.c). 
   Retorna a quantidade de arquivos encontrados.*/
int contaArquivos(const char *nomeDir);

/* Função chamada quando o argumento for um diretório, cria as threads para cada arquivo, chamando contaLinhas. Faz a soma das linhas vazias, 
   comentário e linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processaArquivos(Contador *contagemArquivos, Resultado dados);

/* Função chamada caso o argumento passado seja apenas um arquivo. Cria uma thread para realizar o cálculo de linhas vazias, comentários e
   linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processaArquivo(const char *nomeArq);

/* Com base no argumento recebido pela linha de comando verifica se é um diretório ou um arquivo válido e chama a função necessária para 
   execução do argumento. Pega o tempo inicial e final e calcula a duração de execução.*/
void processoFilho(char *argv);

// Lê o argumento fornecido na linha de comando, se a entrada for válida cria o processo filho e aguarda.
int clsc(int argc, char *argv[]);