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
pid_t criarProcesso() {
    pid_t pid = fork();

    if (pid < 0) {
        printf(ERRO_CRIA_PROCESSO);
        exit(EXIT_FAILURE);
    }
    
    return pid;
} // criarProcesso()

/* Procura a última ocorrência do (.) no nome do arquivo. Retorna NULL se não for encontrado nenhum (.), ou seja, não possui extensão.
   Compara o último ponto com a extensão (.c). Se orquivo tem a extensão (.c) retorna 1, caso contrário, retorna 0.*/
int verificaExtensao(const char *nomeArq) {
    const char *ponto = strrchr(nomeArq, PONTO); // Encontra a última ocorrência do (.).

    if (ponto == NULL)  // Não há extensão no nome do arquivo.
        return 0; 
    if (strcmp(ponto, EXTENSAO) == 0) 
        return 1; // Encontrou a extensão.
    
    return 0; // Extensão inválida.
} // verificaExtensao()

// Verifica se o argumento passado pelo usuário é um diretório, retorna 1 se for um diretório e 0 caso contrário.
int verificaDiretorio(const char *nomeDir){
    DIR *dir = opendir(nomeDir);
    if (dir != NULL) {
        closedir(dir);
        return 1; 
    }
    return 0;
} // verificaDiretorio()

// Função que calcula a duração do programa em segundos.
double calculaDuracao(time_t inicio, time_t fim){
    return difftime(inicio,fim);
}

// Função para imprimir a quantidade de arquivos e o resultado da soma das linhas.
void saidaContagem(Resultado d){
    printf("%s- Código-fonte C%s\n",ABRE_NEGRITO, FECHA_NEGRITO);
    printf("\tNº de Arquivos = %d\n",d.qntdArquivos);
    printf("\tLinhas vazias = %d\n",d.totalVazias);
    printf("\tLinhas de comentários = %d\n",d.totalComentarios);
    printf("\tLinhas de instruções = %d\n\n",d.totalInstrucoes);
} // saidaContagem()

// Função para imprimir o tempo necessário para execução do programa.
void saidaTempo(TempoExecucao t){
    printf("%s- Tempo%s\n",ABRE_NEGRITO, FECHA_NEGRITO);
    printf("\tInício.: %s\n",t.inicio);
    printf("\tTérmino: %s\n",t.termino);
    printf("\tDuração: %d segundos\n",t.duracao);
} // saidaTempo()

// Função para remover os espaços no começo e no final de cada linha.
void removerEspacos(char *linha) {
    int inicio = 0, fim = strlen(linha) - 1;

    // Removendo os espaços ou tabulações do início da linha.
    while (linha[inicio] == ESPACO || linha[inicio] == TABULACAO_HORIZONTAL)
        inicio++;

    // Remove os espaços ou tabulações do final da linha.
    while (fim >= inicio && (linha[fim] == ESPACO || linha[fim] == TABULACAO_HORIZONTAL || linha[fim] == FIM_DE_LINHA))
        fim--;

    // Ajusta a string para encerrar no ponto certo.
    int i, j = 0;
    for (i = inicio; i <= fim; i++) {
        linha[j++] = linha[i];
    }
    linha[j] = CARACTERE_NULO;  // Insere o terminador da string.
} // removerEspacos()

// Função para contar o número de linhas vazias, comentários e instruções de um código-fonte.
int contaLinhas(void *arg){
    Contador *contador = (Contador*)arg;
    FILE *arq = fopen(contador->nomeArq, "r");
    
    char linha[sizeof(arq) * TAMANHO_BUFFER_LINHA];
    char *posicaoFechaBloco = strstr(linha, FECHA_BLOCO_COMENTARIO);
    char *posicaoComentarioLinha = strstr(linha, COMENTARIO_DE_LINHA);
    int blocoComentario=0; // Flag que identifica se iniciou um bloco de comentário.

    contador->instrucoes = 0;
    contador->comentarios = 0;
    contador->linhasVazias = 0;

    if (arq == NULL) {
        printf(ERRO_ABRE_ARQUIVO);
        exit(EXIT_FAILURE);
    }

    while(fgets(linha,sizeof(linha),arq)){
        size_t len = strlen(linha);
        if (linha[len - 1] == FIM_DE_LINHA) { // Substitui o '\n' por '\0'.
            linha[len - 1] = CARACTERE_NULO;  
        }
        removerEspacos(linha);
        
        // Verificações em blocos de comentários.
        if(blocoComentario==1 && strstr(linha, FECHA_BLOCO_COMENTARIO) != NULL){
            if(posicaoFechaBloco != NULL && *(posicaoFechaBloco+2) != CARACTERE_NULO){
                contador->instrucoes++;
            }
            else if (posicaoFechaBloco != NULL && posicaoComentarioLinha != NULL && *(posicaoFechaBloco + 2) == BARRA && *(posicaoFechaBloco + 3) == BARRA) {
                contador->comentarios++; 
            }
            else{
                contador->comentarios++;
            }
            blocoComentario=0;
        }
        else if(blocoComentario==1 && linha[0] != ASTERISCO){
            contador->comentarios++;
        }
        else if(strstr(linha, ABRE_BLOCO_COMENTARIO) != NULL && strstr(linha, FECHA_BLOCO_COMENTARIO) != NULL && strstr(linha, COMENTARIO_DE_LINHA) != NULL){
            contador->instrucoes++;
        }
        else if(linha[0] == BARRA && linha[1] == ASTERISCO) { // Verifica se é o início de um comentário em bloco.
            contador->comentarios++;
            if (strstr(linha, FECHA_BLOCO_COMENTARIO) == NULL) { // Verifica se "*/" não está presente na linha.
                blocoComentario = 1;
            }
        }
        else if (blocoComentario == 1 && strlen(linha) == 0) {
            contador->comentarios++; // Verifica se é uma linha vazia dentro de um comentário.
        }
        else if (strlen(linha) == 0) { // Verifica se a linha está vazia.
            contador->linhasVazias++; 
        }
        else if(linha[0] == BARRA && linha[1] == BARRA){ // Verifica se é comentário de linha.
            contador->comentarios++;
        }
        else{
            contador->instrucoes++;
        }
    }
    
    fclose(arq);

    thrd_exit(EXIT_SUCCESS);
} // contaLinhas()

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes armazenando os nomes dos 
   arquivos que contenham a extensão (.c).*/
int processaDiretorio(const char *nomeDir, Contador *contagemArquivos, Resultado *dados) { 
    DIR *dir = opendir(nomeDir);

    if (dir == NULL) {
        printf(ERRO_ABRE_DIRETORIO);
        exit(EXIT_FAILURE);
    }

    struct dirent *entrada;
    while((entrada = readdir(dir))!= NULL){
        if(entrada->d_type == DT_DIR && strcmp(entrada->d_name, DIRETORIO_PONTO) && strcmp(entrada->d_name, DIRETORIO_PONTO_PONTO)){
            char caminhoCompleto[TAMANHO_MAX_CAMINHO] = {CARACTERE_NULO};
            strcat(caminhoCompleto,nomeDir);
            strcat(caminhoCompleto,"/");
            strcat(caminhoCompleto, entrada->d_name);
            processaDiretorio(caminhoCompleto, contagemArquivos, dados); 
        } 
        else if(verificaExtensao(entrada->d_name) == 1){
            char caminhoCompleto[TAMANHO_MAX_CAMINHO] = {CARACTERE_NULO};
            strcat(caminhoCompleto,nomeDir);
            strcat(caminhoCompleto,"/");
            strcat(caminhoCompleto, entrada->d_name);
            
            // Armazenando os nomes dos arquivos.
            strcpy(contagemArquivos[dados->qntdArquivos].nomeArq, caminhoCompleto); 
            dados->qntdArquivos++;
        }
    }

    closedir(dir);

    return EXIT_SUCCESS;
} // processaDiretorio()

/* Função chamada caso o argumento passado seja um diretório. Percorre o diretório e subdiretórios existentes verificando a existência de arquivos (.c). 
   Retorna a quantidade de arquivos encontrados.*/
int contaArquivos(const char *nomeDir){
    DIR *dir = opendir(nomeDir);
    int quantidade=0;

    if (dir == NULL) {
        printf(ERRO_ABRE_DIRETORIO);
        exit(EXIT_FAILURE);
    }  

    struct dirent *entrada;
    while((entrada = readdir(dir))!= NULL){
        if(entrada->d_type == DT_DIR && strcmp(entrada->d_name, DIRETORIO_PONTO) && strcmp(entrada->d_name, DIRETORIO_PONTO_PONTO)){
            char caminhoCompleto[TAMANHO_MAX_CAMINHO] = {CARACTERE_NULO};
            strcat(caminhoCompleto,nomeDir);
            strcat(caminhoCompleto,"/");
            strcat(caminhoCompleto, entrada->d_name);
            quantidade += contaArquivos(caminhoCompleto); 
        }
        else if(verificaExtensao(entrada->d_name) == 1){
            // Armazenando a quantidade de arquivos que contenham a extensão desejada.
            quantidade++;
        }
    }

    closedir(dir);
    return quantidade;
}

/* Função chamada quando o argumento for um diretório, cria as threads para cada arquivo, chamando contaLinhas. Faz a soma das linhas vazias, 
   comentário e linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processaArquivos(Contador *contagemArquivos, Resultado dados){
    thrd_t tid[dados.qntdArquivos]; // thread identificafion.

    for(int i=0; i < dados.qntdArquivos; i++){
        thrd_create(&tid[i], contaLinhas, &contagemArquivos[i]);
    }
    for(int i=0; i < dados.qntdArquivos; i++){
        thrd_join(tid[i], NULL);
    }

    for(int i=0; i<dados.qntdArquivos; i++){
        dados.totalComentarios += contagemArquivos[i].comentarios;
    }
    for(int i=0; i<dados.qntdArquivos; i++){
        dados.totalInstrucoes += contagemArquivos[i].instrucoes;
    }
    for(int i=0; i<dados.qntdArquivos; i++){
        dados.totalVazias += contagemArquivos[i].linhasVazias;
    }

    saidaContagem(dados);

    return EXIT_SUCCESS;
}

/* Função chamada caso o argumento passado seja apenas um arquivo. Cria uma thread para realizar o cálculo de linhas vazias, comentários e
   linhas de instruções. Por fim chama a função para exibir o resultado do cálculo das linhas.*/
int processaArquivo(const char *nomeArq){
    thrd_t tid; // thread identificafion.
    Contador contagem;
    Resultado dados;
    dados.qntdArquivos = 1;
    
    strcpy(contagem.nomeArq, nomeArq);

    for(int i=0; i < dados.qntdArquivos; i++){
        thrd_create(&tid, contaLinhas, &contagem);
    }
    for(int i=0; i < dados.qntdArquivos; i++){
        thrd_join(tid, NULL);
    }

    dados.totalComentarios = contagem.comentarios;
    dados.totalInstrucoes = contagem.instrucoes;
    dados.totalVazias = contagem.linhasVazias;

    saidaContagem(dados);
            
    return EXIT_SUCCESS;
} // processaArquivo()

/* Com base no argumento recebido pela linha de comando verifica se é um diretório ou um arquivo válido e chama a função necessária para 
   execução do argumento. Pega o tempo inicial e final e calcula a duração de execução.*/
void processoFilho(char *argv){
    Resultado dados = {0, 0, 0, 0}; // Inicializa os contadores.
    TempoExecucao tempo = {"", "", 0.0};

    time_t inicio, fim;
    struct tm *info;

    time(&inicio); // Armazena o tempo inicial.
    info = localtime(&inicio);
    strftime(tempo.inicio, sizeof(tempo.inicio), "%H:%M:%S", info); // Formata a hora no padrão, horas, minutos e segundos.

    if(verificaDiretorio(argv) == 1){
        int quantidade = contaArquivos(argv);
        Contador *contagemArquivos = (Contador*)malloc(sizeof(Contador)*quantidade);

        processaDiretorio(argv, contagemArquivos, &dados); 
        processaArquivos(contagemArquivos, dados);
        free(contagemArquivos);
    }
    else{
        if(verificaExtensao(argv) == 1)
            processaArquivo(argv);
        else {
            printf(ERRO_EXTENSAO_INVALIDA);
            exit(EXIT_FAILURE);
        }
    }

    time(&fim); // Armazena o tempo final.
    info = localtime(&fim);
    strftime(tempo.termino, sizeof(tempo.termino), "%H:%M:%S", info); // Formata a hora no padrão, horas, minutos e segundos.

    tempo.duracao = calculaDuracao(fim, inicio); // Calcula a duração de execução.

    saidaTempo(tempo);
} // processoFilho()

// Lê o argumento fornecido na linha de comando, se a entrada for válida cria o processo filho e aguarda.
int clsc(int argc, char *argv[]){
    setlocale(LC_ALL,PORTUGUES_BRASIL);
    // Verifica se o usuário omitiu o segundo argumento, exibindo uma mensagem que demonstra como o comando deve ser executado.
    if (argc != 2) {
        printf(EXEMPLO_USO);
        return 1;
    }

    pid_t pid = criarProcesso();
    if(pid > 0){
        int status;
        wait(&status); // Aguarda a conclusão do processo filho.
    }
    else 
        if (pid == 0)
            processoFilho(argv[1]);
        else 
            exit(EXIT_FAILURE);
} // clsc()

int main(int argc, char *argv[]){
    return clsc(argc,argv);
}