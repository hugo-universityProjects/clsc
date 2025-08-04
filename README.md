# 🧮 TSI-clsc (Count Line Source Code)

Um contador de linhas de código-fonte em C desenvolvido como projeto da disciplina de **Sistema Operacionais (TSI)**.  
O `clsc` percorre arquivos e diretórios, analisando arquivos com extensão `.c` para contar linhas de **código**, **comentários** e **linhas vazias**, utilizando **threads** para acelerar o processamento.

---

## 🚀 Funcionalidades

- Aceita como argumento arquivos e/ou diretórios.
- Processa apenas arquivos com extensão `.c`.
- Utiliza threads para análise simultânea de arquivos.
- Classifica cada linha como:
  - Linha de instrução (código válido)
  - Comentário (`//`, `/* ... */`)
  - Linha vazia

---

## 📦 Tecnologias Utilizadas

- Linguagem C (padrão C11)
- Biblioteca `threads.h` para multithreading
- Diretivas POSIX (`<dirent.h>`, `<unistd.h>`, etc.)

---

## ▶️ Como Executar

`./clsc caminho/para/arquivo_ou_diretorio`

- Exemplos:
  - `./clsc exemplo.c`
  - `./clsc src/`

--- 

## 📫 Autor

Desenvolvido por Hugo Vinícius Rodrigues Pereira. *Contribuições são bem-vindas!*
github.com/hugovrp
linkedin.com/in/hugovrp





