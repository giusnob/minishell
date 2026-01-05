# 📚 GUIDA COMPLETA MINISHELL - TEORIA E LOGICA

**Created with ❤️ by Giusmery & Giulia** 🌙

---

## 📖 INDICE

1. [Introduzione](#1-introduzione)
2. [Architettura Generale](#2-architettura-generale)
3. [Flusso di Esecuzione](#3-flusso-di-esecuzione)
4. [Modulo 1: Lexer](#4-modulo-1-lexer)
5. [Modulo 2: Parser](#5-modulo-2-parser)
6. [Modulo 3: Expander](#6-modulo-3-expander)
7. [Modulo 4: Executor](#7-modulo-4-executor)
8. [Modulo 5: Builtins](#8-modulo-5-builtins)
9. [Modulo 6: Redirections](#9-modulo-6-redirections)
10. [Modulo 7: Pipes](#10-modulo-7-pipes)
11. [Modulo 8: Signals](#11-modulo-8-signals)
12. [Modulo 9: Environment](#12-modulo-9-environment)
13. [Gestione Memoria](#13-gestione-memoria)
14. [Error Handling](#14-error-handling)
15. [Best Practices](#15-best-practices)
16. [Tips per la Valutazione](#16-tips-per-la-valutazione)
17. [Fix Recenti e Miglioramenti](#17-fix-recenti-e-miglioramenti)
18. [Conclusione](#18-conclusione)

---

# 1. INTRODUZIONE

## 1.1 Cos'è una Shell?

Una **shell** è un'interfaccia tra l'utente e il sistema operativo che interpreta ed esegue comandi.

### Esempio di funzionamento:

```bash
$ echo hello world
hello world
```

**Cosa fa la shell:**
1. Legge l'input: `echo hello world`
2. Identifica il comando: `echo`
3. Estrae gli argomenti: `["hello", "world"]`
4. Esegue il comando
5. Mostra il risultato

---

## 1.2 Pipeline di Elaborazione

```
┌─────────────┐    ┌────────┐    ┌────────┐    ┌──────────┐    ┌──────────┐
│ Input Utente│ -> │ Lexer  │ -> │ Parser │ -> │ Expander │ -> │ Executor │
└─────────────┘    └────────┘    └────────┘    └──────────┘    └──────────┘
                       ↓             ↓              ↓               ↓
                    Tokens      Comandi       Variabili      Esecuzione
                                               espanse
```

**Fasi di elaborazione:**

| Fase | Input | Output | Funzione |
|------|-------|--------|----------|
| **Lexer** | String | Token list | Divide l'input in parole e operatori |
| **Parser** | Token list | Command list | Organizza i token in comandi strutturati |
| **Expander** | Command args | Expanded args | Espande variabili d'ambiente ($VAR) |
| **Executor** | Command list | Exit status | Esegue i comandi |

---

## 1.3 Funzionalità Implementate

### ✅ Caratteristiche Supportate

| Categoria | Funzionalità | Descrizione |
|-----------|--------------|-------------|
| **Interfaccia** | Prompt interattivo | `minishell$ ` con readline |
| **Comandi** | Esterni | `/bin/ls`, `/bin/cat`, ecc. |
| | Builtins | `echo`, `cd`, `pwd`, `export`, `unset`, `env`, `exit` |
| **Operatori** | Pipes | `\|` per connettere comandi |
| | Redirections | `<`, `>`, `>>`, `<<` |
| **Espansione** | Variabili | `$VAR`, `$?` |
| **Quotes** | Singole | `'...'` preserva letterale |
| | Doppie | `"..."` espande variabili |
| **Segnali** | Ctrl-C | Nuovo prompt |
| | Ctrl-D | Uscita shell |
| | Ctrl-\ | Ignorato |

### ❌ Funzionalità NON Implementate

- `&&` e `||` (operatori logici)
- `;` (separatore comandi)
- Wildcards (`*`)
- Job control (`&`, `bg`, `fg`)
- Backslash escaping (`\`)
- Subshells `$(...)` o `` `...` ``

---

# 2. ARCHITETTURA GENERALE

## 2.1 Strutture Dati Principali

### Diagramma delle Relazioni

```
┌──────────────────────────────────────────────────────────────┐
│                         t_data                                │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ envp: char**           ← Environment variables         │  │
│  │ cmd_list: t_cmd*       ← Lista comandi parsed          │  │
│  │ last_exit_status: int  ← Exit status ($?)              │  │
│  │ stdin_backup: int      ← Backup stdin                  │  │
│  │ stdout_backup: int     ← Backup stdout                 │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                                  │
                                  ├──> t_cmd (linked list)
                                  │
                    ┌─────────────┴─────────────┐
                    │                           │
                    ▼                           ▼
              ┌──────────┐              ┌──────────┐
              │  t_cmd   │──next──>     │  t_cmd   │
              ├──────────┤              ├──────────┤
              │ args[]   │              │ args[]   │
              │ redirs   │              │ redirs   │
              └────┬─────┘              └────┬─────┘
                   │                         │
                   ▼                         ▼
              ┌──────────┐              ┌──────────┐
              │ t_redir  │              │ t_redir  │
              ├──────────┤              ├──────────┤
              │ type     │              │ type     │
              │ file     │              │ file     │
              └──────────┘              └──────────┘
```

---

### t_data - Struttura Globale

```c
typedef struct s_data
{
    char    **envp;              // Environment variables
    char    **export_marks;      // Variables marked for export (no value)
    t_cmd   *cmd_list;           // Lista comandi parsati
    int     last_exit_status;    // Exit status ultimo comando ($?)
    int     stdin_backup;        // Backup stdin originale
    int     stdout_backup;       // Backup stdout originale
} t_data;
```

**Campi dettagliati:**

| Campo | Tipo | Descrizione | Esempio |
|-------|------|-------------|---------|
| `envp` | `char**` | Copia modificabile dell'environment | `["PATH=/bin", "HOME=/Users/giusmery", NULL]` |
| `export_marks` | `char**` | Variabili marcate per export senza valore | `["TESTVAR", "MYVAR", NULL]` |
| `cmd_list` | `t_cmd*` | Lista linked di comandi da eseguire | Punta al primo comando |
| `last_exit_status` | `int` | Exit code ultimo comando (per `$?`) | `0` (success), `127` (not found) |
| `stdin_backup` | `int` | File descriptor backup di stdin | Usato per restore dopo redirections |
| `stdout_backup` | `int` | File descriptor backup di stdout | Usato per restore dopo redirections |

---

### t_token - Token del Lexer

```c
typedef enum e_token_type
{
    TOKEN_WORD,          // Parola: "echo", "hello", "/bin/ls"
    TOKEN_PIPE,          // |
    TOKEN_REDIR_IN,      // 
    TOKEN_REDIR_OUT,     // >
    TOKEN_REDIR_APPEND,  // >>
    TOKEN_REDIR_HEREDOC  // 
} t_token_type;

typedef struct s_token
{
    t_token_type    type;    // Tipo del token
    char            *value;  // Valore string del token
    struct s_token  *next;   // Prossimo token (lista collegata)
} t_token;
```

**Esempio pratico:**

Input: `echo hello | cat`

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ TOKEN_WORD  │ -> │ TOKEN_WORD  │ -> │ TOKEN_PIPE  │ -> │ TOKEN_WORD  │ -> NULL
│ "echo"      │    │ "hello"     │    │ "|"         │    │ "cat"       │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
```

---

### t_cmd - Comando Strutturato

```c
typedef struct s_cmd
{
    char            **args;     // Array di argomenti NULL-terminated
    t_redir         *redirs;    // Lista di redirections
    struct s_cmd    *next;      // Prossimo comando (per pipes)
} t_cmd;
```

**Esempio pratico:**

Input: `cat < input.txt > output.txt | wc -l`

```
┌────────────────────────┐         ┌────────────────────────┐
│ CMD 1                  │  next   │ CMD 2                  │
├────────────────────────┤  -----> ├────────────────────────┤
│ args: ["cat", NULL]    │         │ args: ["wc", "-l", NULL]│
│ redirs: [<input.txt]   │         │ redirs: NULL           │
│         [>output.txt]  │         │                        │
└────────────────────────┘         └────────────────────────┘
```

---

### t_redir - Redirection

```c
typedef enum e_redir_type
{
    REDIR_IN,       // <  input da file
    REDIR_OUT,      // >  output su file (truncate)
    REDIR_APPEND,   // >> output su file (append)
    REDIR_HEREDOC   // << heredoc
} t_redir_type;

typedef struct s_redir
{
    t_redir_type    type;       // Tipo di redirection
    char            *file;      // Nome file o delimiter heredoc
    struct s_redir  *next;      // Prossima redirection
} t_redir;
```

**Tabella tipi di redirection:**

| Tipo | Simbolo | Descrizione | Esempio |
|------|---------|-------------|---------|
| `REDIR_IN` | `<` | Input da file | `cat < file.txt` |
| `REDIR_OUT` | `>` | Output su file (sovrascrive) | `echo hi > out.txt` |
| `REDIR_APPEND` | `>>` | Output su file (append) | `echo hi >> out.txt` |
| `REDIR_HEREDOC` | `<<` | Input da heredoc | `cat << EOF` |

---

## 2.2 Organizzazione File

```
minishell/
│
├── includes/
│   └── minishell.h              ← Header con struct e prototipi
│
├── src/
│   ├── main.c                   ← Entry point e main loop
│   ├── init.c                   ← Inizializzazione e cleanup
│   ├── signals.c                ← Gestione segnali (SIGINT, ecc.)
│   │
│   ├── lexer.c                  ← Funzione principale lexer
│   ├── lexer_utils.c            ← Token creation, skip_spaces
│   ├── lexer_handlers.c         ← Handle pipe, redirections
│   │
│   ├── parser.c                 ← Funzione principale parser
│   ├── parser_utils.c           ← Create cmd, add args
│   ├── parser_handlers.c        ← Handle redirections, process tokens
│   │
│   ├── expand.c                 ← Espansione variabili
│   ├── expand_utils.c           ← ft_itoa, ft_strlen, ft_isalnum
│   ├── expand_helpers.c         ← Helper per espansione complessa
│   │
│   ├── executor.c               ← Esecuzione comandi
│   ├── path_utils.c             ← Ricerca comando in PATH
│   │
│   ├── pipes.c                  ← Gestione pipeline
│   ├── pipes_utils.c            ← Utility per pipes
│   ├── pipes_helpers.c          ← Helper setup pipes
│   │
│   ├── redirections.c           ← Applicazione redirections
│   ├── redirections_utils.c     ← Gestione heredoc
│   │
│   ├── builtins.c               ← Dispatcher builtins
│   ├── builtin_echo.c           ← Implementazione echo
│   ├── builtin_cd.c             ← Implementazione cd
│   ├── builtin_pwd.c            ← Implementazione pwd
│   ├── builtin_export.c         ← Implementazione export
│   ├── builtin_unset.c          ← Implementazione unset
│   ├── builtin_env.c            ← Implementazione env
│   ├── builtin_exit.c           ← Implementazione exit
│   │
│   ├── env_utils.c              ← Get/copy environment
│   ├── env_utils2.c             ← Set/unset environment
│   │
│   ├── utils.c                  ← String utilities
│   ├── utils2.c                 ← Array utilities (split, free)
│   ├── utils_fd.c               ← I/O utilities
│   │
│   └── error.c                  ← Gestione errori
│
└── Makefile                     ← Compilazione
```

---

# 3. FLUSSO DI ESECUZIONE

## 3.1 Main Loop

```c
int main(int argc, char **argv, char **envp)
{
    t_data  data;
    char    *input;
    
    (void)argc;
    (void)argv;
    
    // ═══════════════════════════════════════════════════════
    // FASE 1: INIZIALIZZAZIONE
    // ═══════════════════════════════════════════════════════
    init_data(&data, envp);      // Copia envp, backup stdin/stdout
    setup_signals();              // Configura handler SIGINT e SIGQUIT
    
    // ═══════════════════════════════════════════════════════
    // FASE 2: MAIN LOOP
    // ═══════════════════════════════════════════════════════
    while (1)
    {
        input = readline("minishell$ ");  // Legge input con history
        
        if (!input)                        // Ctrl-D → EOF
        {
            ft_putendl_fd("exit", STDOUT_FILENO);
            break;
        }
        
        if (*input)                        // Solo se non vuoto
        {
            add_history(input);            // Aggiungi a history readline
            process_input(&data, input);   // ← ELABORA COMANDO
        }
        
        free(input);
    }
    
    // ═══════════════════════════════════════════════════════
    // FASE 3: CLEANUP E USCITA
    // ═══════════════════════════════════════════════════════
    cleanup_data(&data);
    return (data.last_exit_status);
}
```

### Diagramma di Flusso Main Loop

```
                    ┌─────────────────┐
                    │  INIZIALIZZA    │
                    │  - copy envp    │
                    │  - setup signals│
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │ readline()      │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  input == NULL? │
                    └────┬───────┬────┘
                         │ YES   │ NO
                         │       │
                    ┌────▼────┐  │
                    │  EXIT   │  │
                    └─────────┘  │
                                 │
                        ┌────────▼────────┐
                        │ input non vuoto?│
                        └────┬───────┬────┘
                             │ YES   │ NO
                             │       │
                    ┌────────▼────┐  │
                    │add_history()│  │
                    └────────┬────┘  │
                             │       │
                    ┌────────▼────┐  │
                    │process_input│  │
                    └────────┬────┘  │
                             │       │
                    ┌────────▼───────▼┐
                    │   free(input)   │
                    └────────┬─────────┘
                             │
                    ┌────────▼────────┐
                    │   LOOP BACK     │
                    └─────────────────┘
```

---

## 3.2 Process Input - Pipeline Completa

```c
void process_input(t_data *data, char *input)
{
    t_token *tokens;
    
    // ═══════════════════════════════════════════════════════
    // FASE 1: LEXICAL ANALYSIS (LEXER)
    // Input: String grezza
    // Output: Lista di token
    // ═══════════════════════════════════════════════════════
    tokens = lexer(input, data);
    if (!tokens)
        return;
    
    // ═══════════════════════════════════════════════════════
    // FASE 2: SYNTACTIC ANALYSIS (PARSER)
    // Input: Lista token
    // Output: Lista comandi strutturati
    // ═══════════════════════════════════════════════════════
    data->cmd_list = parser(tokens);
    if (!data->cmd_list)
    {
        free_tokens(tokens);
        return;
    }
    
    // ═══════════════════════════════════════════════════════
    // FASE 3: EXECUTION
    // Input: Lista comandi
    // Output: Exit status
    // Nota: L'espansione avviene dentro l'executor
    // ═══════════════════════════════════════════════════════
    data->last_exit_status = executor(data);
    
    // ═══════════════════════════════════════════════════════
    // FASE 4: CLEANUP
    // ═══════════════════════════════════════════════════════
    free_tokens(tokens);
    free_cmd_list(data->cmd_list);
}
```

---

## 3.3 Esempio Pratico Completo

### Input dell'Utente

```bash
echo "hello $USER" | cat > output.txt
```

### FASE 1: LEXER - Tokenizzazione

```
Input string:
"echo \"hello $USER\" | cat > output.txt"

┌─────────────────────────────────────────────────────────────┐
│                      LEXER PROCESS                          │
└─────────────────────────────────────────────────────────────┘

Step 1: Trova "echo" → TOKEN_WORD
Step 2: Trova "hello $USER" (dentro "...") → TOKEN_WORD
        (espansione avviene in questo punto!)
        "hello $USER" → "hello giusmery"
Step 3: Trova "|" → TOKEN_PIPE
Step 4: Trova "cat" → TOKEN_WORD
Step 5: Trova ">" → TOKEN_REDIR_OUT
Step 6: Trova "output.txt" → TOKEN_WORD

Output token list:
┌──────────────┐   ┌─────────────────┐   ┌────────────┐
│ TOKEN_WORD   │ → │ TOKEN_WORD      │ → │ TOKEN_PIPE │ →
│ "echo"       │   │ "hello giusmery"│   │ "|"        │
└──────────────┘   └─────────────────┘   └────────────┘
   ┌────────────┐   ┌────────────────┐   ┌────────────────┐
 → │ TOKEN_WORD │ → │ TOKEN_REDIR_OUT│ → │ TOKEN_WORD     │ → NULL
   │ "cat"      │   │ ">"            │   │ "output.txt"   │
   └────────────┘   └────────────────┘   └────────────────┘
```

---

### FASE 2: PARSER - Costruzione AST

```
Input: Token list

┌─────────────────────────────────────────────────────────────┐
│                      PARSER PROCESS                         │
└─────────────────────────────────────────────────────────────┘

Process fino a PIPE:
  - TOKEN_WORD "echo" → args[0]
  - TOKEN_WORD "hello giusmery" → args[1]
  - TOKEN_PIPE → Fine CMD 1

Process dopo PIPE:
  - TOKEN_WORD "cat" → args[0]
  - TOKEN_REDIR_OUT ">" → redirection OUT
  - TOKEN_WORD "output.txt" → file della redirection

Output command list:

┌──────────────────────────────────┐
│ CMD 1                            │
├──────────────────────────────────┤
│ args[0] = "echo"                 │
│ args[1] = "hello giusmery"       │
│ args[2] = NULL                   │
│ redirs = NULL                    │
│                                  │
│ next ─────────────────────┐      │
└──────────────────────────┼──────┘
                           │
                           ▼
           ┌───────────────────────────────┐
           │ CMD 2                         │
           ├───────────────────────────────┤
           │ args[0] = "cat"               │
           │ args[1] = NULL                │
           │                               │
           │ redirs:                       │
           │   ┌─────────────────────────┐ │
           │   │ type: REDIR_OUT         │ │
           │   │ file: "output.txt"      │ │
           │   │ next: NULL              │ │
           │   └─────────────────────────┘ │
           │                               │
           │ next = NULL                   │
           └───────────────────────────────┘
```

---

### FASE 3: EXECUTOR - Esecuzione

```
┌─────────────────────────────────────────────────────────────┐
│                    EXECUTOR PROCESS                         │
└─────────────────────────────────────────────────────────────┘

Rileva: 2 comandi → È una PIPELINE

Step 1: Crea pipe
        pipe_fd[0] = 3 (read end)
        pipe_fd[1] = 4 (write end)

Step 2: Fork processo per CMD 1 (echo)
        ┌─────────────────────────────┐
        │ PROCESSO FIGLIO 1           │
        ├─────────────────────────────┤
        │ dup2(pipe_fd[1], STDOUT)    │ ← stdout → pipe
        │ close(pipe_fd[0])            │
        │ close(pipe_fd[1])            │
        │ execve("/bin/echo",          │
        │        ["echo","hello        │
        │         giusmery"],          │
        │        envp)                 │
        └─────────────────────────────┘

Step 3: Fork processo per CMD 2 (cat)
        ┌─────────────────────────────┐
        │ PROCESSO FIGLIO 2           │
        ├─────────────────────────────┤
        │ dup2(pipe_fd[0], STDIN)     │ ← stdin ← pipe
        │ close(pipe_fd[0])            │
        │ close(pipe_fd[1])            │
        │                              │
        │ apply_redirections():        │
        │   fd = open("output.txt",   │
        │            O_WRONLY|         │
        │            O_CREAT|O_TRUNC)  │
        │   dup2(fd, STDOUT)           │ ← stdout → file
        │   close(fd)                  │
        │                              │
        │ execve("/bin/cat",           │
        │        ["cat"],              │
        │        envp)                 │
        └─────────────────────────────┘

Step 4: Processo padre
        - close(pipe_fd[0])
        - close(pipe_fd[1])
        - waitpid(pid1)
        - waitpid(pid2)
        - return exit_status

┌─────────────────────────────────────────────────────────────┐
│                         RISULTATO                           │
└─────────────────────────────────────────────────────────────┘

File "output.txt" contiene:
hello giusmery
```

---

# 4. MODULO 1: LEXER

## 4.1 Cosa Fa il Lexer?

Il **Lexer** (o Lexical Analyzer) trasforma una stringa grezza in una sequenza di **token**.

### Trasformazione

```
INPUT  → "echo hello | cat"
         
OUTPUT → [WORD:"echo"] → [WORD:"hello"] → [PIPE:"|"] → [WORD:"cat"] → NULL
```

---

## 4.2 Tipi di Token

### Enumerazione Token Types

```c
typedef enum e_token_type
{
    TOKEN_WORD,          // Parola generica
    TOKEN_PIPE,          // |
    TOKEN_REDIR_IN,      // 
    TOKEN_REDIR_OUT,     // >
    TOKEN_REDIR_APPEND,  // >>
    TOKEN_REDIR_HEREDOC  // 
} t_token_type;
```

### Tabella Riconoscimento

| Input | Tipo Token | Descrizione | Azione |
|-------|------------|-------------|--------|
| `echo` | `TOKEN_WORD` | Parola normale | Estrae fino a spazio/operatore |
| `"hello world"` | `TOKEN_WORD` | Stringa quoted | Estrae preservando spazi |
| `\|` | `TOKEN_PIPE` | Pipe | Crea token pipe |
| `<` | `TOKEN_REDIR_IN` | Input redirect | Crea token redirect in |
| `>` | `TOKEN_REDIR_OUT` | Output redirect | Crea token redirect out |
| `>>` | `TOKEN_REDIR_APPEND` | Append redirect | Crea token append |
| `<<` | `TOKEN_REDIR_HEREDOC` | Heredoc | Crea token heredoc |

---

## 4.3 Algoritmo del Lexer

### Pseudo-codice

```
FUNZIONE lexer(input_string):
    tokens = lista_vuota
    posizione = 0
    
    MENTRE posizione < lunghezza(input_string):
        
        1. Salta spazi e tab
           → avanza posizione
        
        2. SE carattere corrente è operatore (|, <, >):
           → crea token operatore
           → gestisci operatori doppi (>>, <<)
           → aggiungi a tokens
           → avanza posizione
        
        3. ALTRIMENTI SE carattere corrente non è fine stringa:
           → estrai parola (gestendo quotes)
           → crea token WORD
           → aggiungi a tokens
           → avanza posizione
    
    RITORNA tokens
```

---

### Implementazione C

```c
t_token *lexer(char *input, t_data *data)
{
    t_token *tokens;
    t_token *new_token;
    
    tokens = NULL;
    
    while (*input)
    {
        // ────────────────────────────────────────────────
        // STEP 1: Salta whitespace
        // ────────────────────────────────────────────────
        skip_spaces(&input);
        
        // ────────────────────────────────────────────────
        // STEP 2: Riconosci operatori
        // ────────────────────────────────────────────────
        if (*input == '|' || *input == '<' || *input == '>')
        {
            new_token = handle_operator(&input);
        }
        // ────────────────────────────────────────────────
        // STEP 3: Estrai parola (con gestione quotes)
        // ────────────────────────────────────────────────
        else if (*input)
        {
            new_token = handle_word(&input, data);
        }
        else
            break;
        
        // ────────────────────────────────────────────────
        // STEP 4: Aggiungi token alla lista
        // ────────────────────────────────────────────────
        if (new_token)
            add_token(&tokens, new_token);
    }
    
    return (tokens);
}
```

---

## 4.4 Gestione Quotes

### Single Quotes (`'`) - Letterale

**Comportamento:** Tutto tra `'...'` è trattato **letteralmente** (nessuna espansione).

```bash
echo 'hello $USER'
# Output: hello $USER
```

**Implementazione:**

```c
char *extract_single_quote(char **input)
{
    char *start;
    int len;
    
    (*input)++;  // Salta '
    start = *input;
    len = 0;
    
    // Conta caratteri fino a prossimo '
    while ((*input)[len] && (*input)[len] != '\'')
        len++;
    
    // Estrai substring
    char *result = ft_strndup(start, len);
    
    // Avanza oltre contenuto
    *input += len;
    
    // Salta ' finale se presente
    if (**input == '\'')
        (*input)++;
    
    return (result);
}
```

---

### Double Quotes (`"`) - Espande Variabili

**Comportamento:** Espande `$VAR` ma preserva spazi.

```bash
echo "hello $USER"
# Output: hello giusmery
```

**Implementazione:**

```c
char *extract_double_quote(char **input, t_data *data)
{
    char *start;
    char *raw;
    char *expanded;
    int len;
    
    (*input)++;  // Salta "
    start = *input;
    len = 0;
    
    // Conta caratteri fino a prossimo "
    while ((*input)[len] && (*input)[len] != '"')
        len++;
    
    // Estrai substring grezza
    raw = ft_strndup(start, len);
    
    // ══════════════════════════════════════════════════
    // ESPANDI VARIABILI ($VAR, $?)
    // ══════════════════════════════════════════════════
    expanded = expand_variables(raw, data);
    free(raw);
    
    // Avanza oltre contenuto
    *input += len;
    
    // Salta " finale se presente
    if (**input == '"')
        (*input)++;
    
    return (expanded);
}
```

### Tabella Comparativa Quotes

| Tipo | Simbolo | Espande `$VAR`? | Preserva Spazi? | Esempio |
|------|---------|----------------|----------------|---------|
| **Single** | `'...'` | ❌ No | ✅ Sì | `'$USER'` → `$USER` |
| **Double** | `"..."` | ✅ Sì | ✅ Sì | `"$USER"` → `giusmery` |
| **Nessuna** | - | ✅ Sì | ❌ No (split) | `$USER` → `giusmery` |

---

## 4.5 Gestione Operatori

### Handle Operator - Dispatcher

```c
t_token *handle_operator(char **input)
{
    if (**input == '|')
        return (handle_pipe(input));
    else if (**input == '<')
        return (handle_redir_in(input));
    else if (**input == '>')
        return (handle_redir_out(input));
    
    return (NULL);
}
```

---

### Handle Pipe

```c
t_token *handle_pipe(char **input)
{
    (*input)++;  // Consuma '|'
    return (create_token(TOKEN_PIPE, ft_strdup("|")));
}
```

---

### Handle Redirection In (`<` e `<<`)

```c
t_token *handle_redir_in(char **input)
{
    // Controlla se è heredoc (<<)
    if ((*input)[1] == '<')
    {
        (*input) += 2;  // Consuma '<<'
        return (create_token(TOKEN_REDIR_HEREDOC, ft_strdup("<<")));
    }
    
    // Altrimenti è redirection normale (<)
    (*input)++;  // Consuma '<'
    return (create_token(TOKEN_REDIR_IN, ft_strdup("<")));
}
```

---

### Handle Redirection Out (`>` e `>>`)

```c
t_token *handle_redir_out(char **input)
{
    // Controlla se è append (>>)
    if ((*input)[1] == '>')
    {
        (*input) += 2;  // Consuma '>>'
        return (create_token(TOKEN_REDIR_APPEND, ft_strdup(">>")));
    }
    
    // Altrimenti è redirection normale (>)
    (*input)++;  // Consuma '>'
    return (create_token(TOKEN_REDIR_OUT, ft_strdup(">")));
}
```

---

## 4.6 Esempio Completo Step-by-Step

### Input

```bash
echo "hello $USER" > output.txt
```

### Esecuzione Dettagliata

```
┌─────────────────────────────────────────────────────────────────┐
│                    LEXER STEP-BY-STEP                           │
└─────────────────────────────────────────────────────────────────┘

Posizione iniziale: "echo \"hello $USER\" > output.txt"
                     ^

────────────────────────────────────────────────────────────────
STEP 1: skip_spaces()
────────────────────────────────────────────────────────────────
Nessuno spazio all'inizio
Posizione: "echo \"hello $USER\" > output.txt"
            ^

────────────────────────────────────────────────────────────────
STEP 2: handle_word() → estrae "echo"
────────────────────────────────────────────────────────────────
Carattere corrente: 'e' (non operatore, non quote)
Estrae fino a spazio o operatore: "echo"
Token creato: [TOKEN_WORD:"echo"]
Posizione: " \"hello $USER\" > output.txt"
           ^

────────────────────────────────────────────────────────────────
STEP 3: skip_spaces()
────────────────────────────────────────────────────────────────
Salta 1 spazio
Posizione: "\"hello $USER\" > output.txt"
            ^

────────────────────────────────────────────────────────────────
STEP 4: handle_word() → estrae quoted string
────────────────────────────────────────────────────────────────
Carattere corrente: '"'
Chiama extract_double_quote():
  - Salta '"'
  - Estrae: "hello $USER"
  - Espande $USER → "hello giusmery"
  - Salta '"' finale
Token creato: [TOKEN_WORD:"hello giusmery"]
Posizione: " > output.txt"
           ^

────────────────────────────────────────────────────────────────
STEP 5: skip_spaces()
────────────────────────────────────────────────────────────────
Salta 1 spazio
Posizione: "> output.txt"
            ^

────────────────────────────────────────────────────────────────
STEP 6: handle_operator() → redirection out
────────────────────────────────────────────────────────────────
Carattere corrente: '>'
Prossimo carattere: ' ' (non '>')
È REDIR_OUT (non APPEND)
Token creato: [TOKEN_REDIR_OUT:">"]
Posizione: " output.txt"
           ^

────────────────────────────────────────────────────────────────
STEP 7: skip_spaces()
────────────────────────────────────────────────────────────────
Salta 1 spazio
Posizione: "output.txt"
            ^

────────────────────────────────────────────────────────────────
STEP 8: handle_word() → estrae "output.txt"
────────────────────────────────────────────────────────────────
Carattere corrente: 'o' (non operatore, non quote)
Estrae fino a fine stringa: "output.txt"
Token creato: [TOKEN_WORD:"output.txt"]
Posizione: ""
            ^

────────────────────────────────────────────────────────────────
FINE LEXER
────────────────────────────────────────────────────────────────
```

### Output Finale Token List

```
┌──────────────┐   ┌─────────────────┐   ┌────────────────┐   ┌────────────────┐
│ TOKEN_WORD   │ → │ TOKEN_WORD      │ → │ TOKEN_REDIR_OUT│ → │ TOKEN_WORD     │ → NULL
│ "echo"       │   │ "hello giusmery"│   │ ">"            │   │ "output.txt"   │
└──────────────┘   └─────────────────┘   └────────────────┘   └────────────────┘
```

---

# 5. MODULO 2: PARSER

## 5.1 Cosa Fa il Parser?

Il **Parser** (o Syntactic Analyzer) trasforma una lista di **token** in una lista di **comandi strutturati**.

### Trasformazione

```
INPUT  → [WORD:"echo"] → [WORD:"hello"] → [PIPE:"|"] → [WORD:"cat"]

OUTPUT → CMD1{args:["echo","hello"]} → CMD2{args:["cat"]}
```

---

## 5.2 Algoritmo del Parser

### Pseudo-codice

```
FUNZIONE parser(tokens):
    cmd_list = lista_vuota
    current_token = primo_token
    
    MENTRE current_token esiste:
        
        1. Crea nuovo comando vuoto
        
        2. MENTRE current_token != PIPE E current_token esiste:
           
           SE token è WORD:
              → aggiungi ad args del comando
           
           SE token è REDIR (< > >> <<):
              → crea redirection
              → token successivo deve essere WORD (filename)
              → aggiungi a redirs del comando
           
           Avanza al prossimo token
        
        3. Aggiungi comando a cmd_list
        
        4. SE current_token è PIPE:
              → avanza al prossimo token (salta PIPE)
    
    RITORNA cmd_list
```

---

### Implementazione C

```c
t_cmd *parser(t_token *tokens)
{
    t_cmd   *cmd_list;
    t_cmd   *new_cmd;
    t_token *current;
    
    cmd_list = NULL;
    current = tokens;
    
    while (current)
    {
        // ═══════════════════════════════════════════════════
        // Processa un comando (fino a PIPE o fine)
        // ═══════════════════════════════════════════════════
        new_cmd = process_command(&current);
        
        if (!new_cmd)  // Errore di parsing
        {
            free_cmd_list(cmd_list);
            return (NULL);
        }
        
        // ═══════════════════════════════════════════════════
        // Aggiungi comando alla lista
        // ═══════════════════════════════════════════════════
        add_cmd(&cmd_list, new_cmd);
        
        // ═══════════════════════════════════════════════════
        // Se c'è PIPE, avanza oltre
        // ═══════════════════════════════════════════════════
        if (current && current->type == TOKEN_PIPE)
            current = current->next;
    }
    
    return (cmd_list);
}
```

---

## 5.3 Process Command

```c
t_cmd *process_command(t_token **tokens)
{
    t_cmd   *cmd;
    t_token *current;
    
    // ═══════════════════════════════════════════════════════
    // Crea struttura comando vuota
    // ═══════════════════════════════════════════════════════
    cmd = create_cmd();
    if (!cmd)
        return (NULL);
    
    current = *tokens;
    
    // ═══════════════════════════════════════════════════════
    // Processa token fino a PIPE o fine lista
    // ═══════════════════════════════════════════════════════
    while (current && current->type != TOKEN_PIPE)
    {
        // ───────────────────────────────────────────────────
        // Token è WORD → aggiungi agli argomenti
        // ───────────────────────────────────────────────────
        if (current->type == TOKEN_WORD)
        {
            if (!add_args_to_cmd(cmd, &current))
                return (free_cmd(cmd), NULL);
        }
        // ───────────────────────────────────────────────────
        // Token è REDIRECTION → gestisci redirection
        // ───────────────────────────────────────────────────
        else if (current->type >= TOKEN_REDIR_IN &&
                 current->type <= TOKEN_REDIR_HEREDOC)
        {
            if (!handle_redirection(cmd, &current))
                return (free_cmd(cmd), NULL);
        }
        else
            break;
    }
    
    // ═══════════════════════════════════════════════════════
    // Aggiorna puntatore tokens
    // ═══════════════════════════════════════════════════════
    *tokens = current;
    
    // ═══════════════════════════════════════════════════════
    // Valida comando (deve avere almeno un argomento)
    // ═══════════════════════════════════════════════════════
    if (!cmd->args)
    {
        free_cmd(cmd);
        return (NULL);
    }
    
    return (cmd);
}
```

---

## 5.4 Gestione Argomenti

### Add Args to Command

```c
int add_args_to_cmd(t_cmd *cmd, t_token **tokens)
{
    t_token *current;
    int     arg_count;
    int     i;
    
    current = *tokens;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Conta argomenti consecutivi (TOKEN_WORD)
    // ═══════════════════════════════════════════════════════
    arg_count = count_args(current);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Alloca array argomenti (+1 per NULL)
    // ═══════════════════════════════════════════════════════
    cmd->args = malloc(sizeof(char *) * (arg_count + 1));
    if (!cmd->args)
        return (0);
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Copia argomenti nell'array
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (current && current->type == TOKEN_WORD)
    {
        cmd->args[i] = ft_strdup(current->value);
        if (!cmd->args[i])
            return (0);  // Errore allocazione
        i++;
        current = current->next;
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Termina array con NULL
    // ═══════════════════════════════════════════════════════
    cmd->args[i] = NULL;
    
    // ═══════════════════════════════════════════════════════
    // STEP 5: Aggiorna puntatore token
    // ═══════════════════════════════════════════════════════
    *tokens = current;
    
    return (1);
}
```

### Count Args Helper

```c
int count_args(t_token *tokens)
{
    int count;
    
    count = 0;
    while (tokens && tokens->type == TOKEN_WORD)
    {
        count++;
        tokens = tokens->next;
    }
    
    return (count);
}
```

---

## 5.5 Gestione Redirections

### Handle Redirection

```c
int handle_redirection(t_cmd *cmd, t_token **tokens)
{
    t_token      *current;
    t_redir_type type;
    t_redir      *new_redir;
    
    current = *tokens;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Determina tipo redirection
    // ═══════════════════════════════════════════════════════
    type = get_redir_type(current->type);
    if (type == -1)
        return (0);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Avanza al token successivo (deve essere WORD)
    // ═══════════════════════════════════════════════════════
    current = current->next;
    
    if (!current || current->type != TOKEN_WORD)
    {
        print_error("parser", "syntax error near redirection");
        return (0);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Crea struttura redirection
    // ═══════════════════════════════════════════════════════
    new_redir = create_redir(type, current->value);
    if (!new_redir)
        return (0);
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Aggiungi a lista redirections del comando
    // ═══════════════════════════════════════════════════════
    add_redir(&cmd->redirs, new_redir);
    
    // ═══════════════════════════════════════════════════════
    // STEP 5: Aggiorna puntatore token
    // ═══════════════════════════════════════════════════════
    *tokens = current->next;
    
    return (1);
}
```

### Get Redirection Type Helper

```c
t_redir_type get_redir_type(t_token_type token_type)
{
    if (token_type == TOKEN_REDIR_IN)
        return (REDIR_IN);
    else if (token_type == TOKEN_REDIR_OUT)
        return (REDIR_OUT);
    else if (token_type == TOKEN_REDIR_APPEND)
        return (REDIR_APPEND);
    else if (token_type == TOKEN_REDIR_HEREDOC)
        return (REDIR_HEREDOC);
    
    return (-1);  // Tipo non valido
}
```

---

## 5.6 Esempio Completo Step-by-Step

### Input Token List

```
[WORD:"cat"] → [REDIR_IN:"<"] → [WORD:"in.txt"] → 
[REDIR_OUT:">"] → [WORD:"out.txt"] → [PIPE:"|"] → 
[WORD:"wc"] → [WORD:"-l"] → NULL
```

### Esecuzione Dettagliata

```
┌─────────────────────────────────────────────────────────────────┐
│                    PARSER STEP-BY-STEP                          │
└─────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════
COMANDO 1
═══════════════════════════════════════════════════════════════════

────────────────────────────────────────────────────────────────
STEP 1: process_command() - Inizia
────────────────────────────────────────────────────────────────
Crea cmd vuoto:
  cmd->args = NULL
  cmd->redirs = NULL
  cmd->next = NULL

────────────────────────────────────────────────────────────────
STEP 2: Token [WORD:"cat"]
────────────────────────────────────────────────────────────────
Tipo: TOKEN_WORD
Azione: add_args_to_cmd()
  - Conta args consecutivi: 1
  - Alloca cmd->args[2]  (1 arg + NULL)
  - cmd->args[0] = "cat"
  - cmd->args[1] = NULL
Avanza token

────────────────────────────────────────────────────────────────
STEP 3: Token [REDIR_IN:"<"]
────────────────────────────────────────────────────────────────
Tipo: TOKEN_REDIR_IN
Azione: handle_redirection()
  - Tipo = REDIR_IN
  - Avanza token (deve essere WORD)
  - Token successivo: [WORD:"in.txt"] ✓
  - Crea redir:
      type = REDIR_IN
      file = "in.txt"
      next = NULL
  - Aggiungi a cmd->redirs
Avanza token (dopo "in.txt")

────────────────────────────────────────────────────────────────
STEP 4: Token [REDIR_OUT:">"]
────────────────────────────────────────────────────────────────
Tipo: TOKEN_REDIR_OUT
Azione: handle_redirection()
  - Tipo = REDIR_OUT
  - Avanza token (deve essere WORD)
  - Token successivo: [WORD:"out.txt"] ✓
  - Crea redir:
      type = REDIR_OUT
      file = "out.txt"
      next = NULL
  - Aggiungi a cmd->redirs (dopo REDIR_IN)
Avanza token (dopo "out.txt")

────────────────────────────────────────────────────────────────
STEP 5: Token [PIPE:"|"]
────────────────────────────────────────────────────────────────
Tipo: TOKEN_PIPE
Azione: Fine CMD 1
Stato finale CMD 1:
  args = ["cat", NULL]
  redirs = [REDIR_IN:"in.txt"] → [REDIR_OUT:"out.txt"] → NULL

Aggiungi CMD 1 a cmd_list
Salta PIPE
Avanza token

═══════════════════════════════════════════════════════════════════
COMANDO 2
═══════════════════════════════════════════════════════════════════

────────────────────────────────────────────────────────────────
STEP 6: process_command() - Inizia
────────────────────────────────────────────────────────────────
Crea nuovo cmd vuoto

────────────────────────────────────────────────────────────────
STEP 7: Token [WORD:"wc"]
────────────────────────────────────────────────────────────────
Tipo: TOKEN_WORD
Azione: add_args_to_cmd()
  - Conta args consecutivi: 2 ("wc" e "-l")
  - Alloca cmd->args[3]  (2 args + NULL)
  - cmd->args[0] = "wc"
  - cmd->args[1] = "-l"
  - cmd->args[2] = NULL
Avanza token (oltre "-l")

────────────────────────────────────────────────────────────────
STEP 8: Fine token list
────────────────────────────────────────────────────────────────
current = NULL
Fine CMD 2
Stato finale CMD 2:
  args = ["wc", "-l", NULL]
  redirs = NULL

Aggiungi CMD 2 a cmd_list (CMD 1->next = CMD 2)

═══════════════════════════════════════════════════════════════════
FINE PARSER
═══════════════════════════════════════════════════════════════════
```

### Output Finale Command List

```
┌──────────────────────────────────────┐
│ CMD 1                                │
├──────────────────────────────────────┤
│ args[0] = "cat"                      │
│ args[1] = NULL                       │
│                                      │
│ redirs:                              │
│   ┌────────────────────────────────┐ │
│   │ REDIR_IN                       │ │
│   │ file: "in.txt"                 │ │
│   │ next ───────┐                  │ │
│   └─────────────┼──────────────────┘ │
│                 │                    │
│          ┌──────▼──────────────────┐ │
│          │ REDIR_OUT               │ │
│          │ file: "out.txt"         │ │
│          │ next: NULL              │ │
│          └─────────────────────────┘ │
│                                      │
│ next ─────────────────┐              │
└───────────────────────┼──────────────┘
                        │
                        ▼
        ┌───────────────────────────────┐
        │ CMD 2                         │
        ├───────────────────────────────┤
        │ args[0] = "wc"                │
        │ args[1] = "-l"                │
        │ args[2] = NULL                │
        │                               │
        │ redirs: NULL                  │
        │ next: NULL                    │
        └───────────────────────────────┘
```

---

# 6. MODULO 3: EXPANDER

## 6.1 Cosa Fa l'Expander?

L'**Expander** sostituisce le variabili d'ambiente (`$VAR`) con i loro valori.

### Trasformazione

```
INPUT  → "hello $USER, pwd is $PWD"
         (con USER=giusmery, PWD=/home/minishell)

OUTPUT → "hello giusmery, pwd is /home/minishell"
```

---

## 6.2 Variabili Supportate

### Tabella Variabili

| Sintassi | Descrizione | Esempio | Espansione |
|----------|-------------|---------|------------|
| `$VAR` | Variabile ambiente | `$USER` | `"giusmery"` |
| `$?` | Exit status ultimo comando | `$?` | `"0"` o `"127"` |
| `$` | Dollar solo (non seguito da valido) | `$ ` | `"$"` |
| `$123` | Variabili numeriche (non supportate) | `$1` | `""` (vuoto) |

---

## 6.3 Algoritmo dell'Expander

### Pseudo-codice

```
FUNZIONE expand_variables(str, data):
    result = stringa_vuota
    i = 0
    
    MENTRE i < lunghezza(str):
        
        SE str[i] == '$' E c'è carattere successivo:
            
            1. Estrai nome variabile
               → da $ fino a carattere non valido
               → validi: a-z A-Z 0-9 _ ?
            
            2. Cerca valore in environment
               → get_env_value(nome, data->envp)
            
            3. SE trovato:
                  → appendi valore a result
               ALTRIMENTI:
                  → appendi stringa vuota
            
            4. Avanza i oltre nome variabile
        
        ALTRIMENTI:
            → appendi str[i] a result
            → i++
    
    RITORNA result
```

---

### Implementazione C

```c
char *expand_variables(char *str, t_data *data)
{
    t_expand_ctx ctx;
    char         *result;
    int          i;
    int          j;
    
    if (!str)
        return (NULL);
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Calcola lunghezza necessaria
    // ═══════════════════════════════════════════════════════
    result = malloc(sizeof(char) * (calculate_expanded_len(str, data) + 1));
    if (!result)
        return (NULL);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Inizializza contesto
    // ═══════════════════════════════════════════════════════
    i = 0;
    j = 0;
    init_expand_ctx(&ctx, str, data);
    ctx.result = result;
    ctx.i = &i;
    ctx.j = &j;
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Scansiona e espandi
    // ═══════════════════════════════════════════════════════
    while (str[i])
    {
        if (str[i] == '$' && str[i + 1])
        {
            // ───────────────────────────────────────────────
            // Espandi variabile
            // ───────────────────────────────────────────────
            process_expansion(&ctx);
        }
        else
        {
            // ───────────────────────────────────────────────
            // Copia carattere normale
            // ───────────────────────────────────────────────
            result[j++] = str[i++];
        }
    }
    
    result[j] = '\0';
    return (result);
}
```

---

## 6.4 Estrazione Nome Variabile

### Extract Var Name

```c
char *extract_var_name(char *str, int *len)
{
    int i;
    
    i = 1;  // Salta '$'
    
    // ═══════════════════════════════════════════════════════
    // CASO SPECIALE: $?
    // ═══════════════════════════════════════════════════════
    if (str[i] == '?')
    {
        *len = 2;  // '$' + '?'
        return (ft_strdup("?"));
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO NORMALE: Estrai nome (lettere, numeri, _)
    // ═══════════════════════════════════════════════════════
    while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
        i++;
    
    *len = i;  // Lunghezza totale incluso '$'
    
    // Ritorna nome SENZA '$'
    return (ft_strndup(str + 1, i - 1));
}
```

### Tabella Caratteri Validi

| Tipo | Caratteri | Descrizione |
|------|-----------|-------------|
| **Lettere** | `a-z A-Z` | Case sensitive |
| **Numeri** | `0-9` | Ma non come primo carattere |
| **Underscore** | `_` | Valido ovunque |
| **Question** | `?` | Solo `$?` (exit status) |

---

## 6.5 Espansione Singola Variabile

### Expand Single Var

```c
char *expand_single_var(char *var_name, t_data *data)
{
    char *value;
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: $? (exit status)
    // ═══════════════════════════════════════════════════════
    if (ft_strcmp(var_name, "?") == 0)
        return (ft_itoa(data->last_exit_status));
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Variabile normale
    // ═══════════════════════════════════════════════════════
    value = get_env_value(var_name, data->envp);
    
    if (!value)
        return (ft_strdup(""));  // Variabile non trovata → ""
    
    return (value);
}
```

### Tabella Comportamenti

| Input | Valore in ENV | Output |
|-------|---------------|--------|
| `$USER` | `"giusmery"` | `"giusmery"` |
| `$HOME` | `"/Users/giusmery"` | `"/Users/giusmery"` |
| `$NOTEXIST` | - | `""` (stringa vuota) |
| `$?` | - | `"0"` (o altro exit code) |

---

## 6.6 Esempio Completo Step-by-Step

### Setup

```c
// Dati iniziali
str = "User: $USER, Home: $HOME, Status: $?"

data->envp = [
    "USER=giusmery",
    "HOME=/Users/giusmery",
    "PATH=/bin:/usr/bin",
    NULL
]

data->last_exit_status = 0
```

### Esecuzione Dettagliata

```
┌─────────────────────────────────────────────────────────────────┐
│                  EXPANDER STEP-BY-STEP                          │
└─────────────────────────────────────────────────────────────────┘

Input: "User: $USER, Home: $HOME, Status: $?"
        ^

────────────────────────────────────────────────────────────────
STEP 1: i=0-5 → Caratteri normali
────────────────────────────────────────────────────────────────
Copia: "User: "
result = "User: "
i = 6

────────────────────────────────────────────────────────────────
STEP 2: i=6 → Trova '$USER'
────────────────────────────────────────────────────────────────
Carattere: '$'
Prossimo carattere: 'U' (valido)

Chiama extract_var_name():
  str + i = "$USER, Home: $HOME, Status: $?"
  Salta '$'
  Estrae fino a non-valido: "USER"
  var_len = 5  ('$' + "USER")
  return "USER"

Chiama expand_single_var("USER", data):
  Cerca in envp:
    envp[0] = "USER=giusmery"
    Confronta "USER" con "USER" → MATCH!
    Estrae valore dopo '=': "giusmery"
    return "giusmery"

Copia "giusmery" in result:
  result = "User: giusmery"
  
Avanza i di 5 (lunghezza "$USER"):
  i = 11

────────────────────────────────────────────────────────────────
STEP 3: i=11-18 → Caratteri normali
────────────────────────────────────────────────────────────────
Copia: ", Home: "
result = "User: giusmery, Home: "
i = 19

────────────────────────────────────────────────────────────────
STEP 4: i=19 → Trova '$HOME'
────────────────────────────────────────────────────────────────
Carattere: '$'
Prossimo carattere: 'H' (valido)

Chiama extract_var_name():
  str + i = "$HOME, Status: $?"
  Estrae: "HOME"
  var_len = 5
  return "HOME"

Chiama expand_single_var("HOME", data):
  Cerca in envp:
    envp[0] = "USER=giusmery" → no match
    envp[1] = "HOME=/Users/giusmery" → MATCH!
    Estrae valore: "/Users/giusmery"
    return "/Users/giusmery"

Copia "/Users/giusmery" in result:
  result = "User: giusmery, Home: /Users/giusmery"
  
Avanza i di 5:
  i = 24

────────────────────────────────────────────────────────────────
STEP 5: i=24-34 → Caratteri normali
────────────────────────────────────────────────────────────────
Copia: ", Status: "
result = "User: giusmery, Home: /Users/giusmery, Status: "
i = 35

────────────────────────────────────────────────────────────────
STEP 6: i=35 → Trova '$?'
────────────────────────────────────────────────────────────────
Carattere: '$'
Prossimo carattere: '?' (speciale)

Chiama extract_var_name():
  str + i = "$?"
  Rileva '?' → caso speciale
  var_len = 2
  return "?"

Chiama expand_single_var("?", data):
  Rileva "?" → exit status
  Converte data->last_exit_status (0) a stringa
  return "0"

Copia "0" in result:
  result = "User: giusmery, Home: /Users/giusmery, Status: 0"
  
Avanza i di 2:
  i = 37

────────────────────────────────────────────────────────────────
STEP 7: Fine stringa
────────────────────────────────────────────────────────────────
i = 37 (lunghezza stringa)
Termina result con '\0'

═══════════════════════════════════════════════════════════════════
RISULTATO FINALE
═══════════════════════════════════════════════════════════════════
```

### Output

```
"User: giusmery, Home: /Users/giusmery, Status: 0"
```

---
# 7. MODULO 4: EXECUTOR

## 7.1 Cosa Fa l'Executor?

L'**Executor** è responsabile dell'esecuzione effettiva dei comandi, gestendo:
- **Builtins** (comandi integrati nella shell)
- **Comandi esterni** (programmi in `/bin`, `/usr/bin`, ecc.)
- **Pipeline** (comandi connessi con `|`)

### Diagramma Decisionale

```
┌─────────────────────────┐
│   executor(data)        │
└───────────┬─────────────┘
            │
            ▼
    ┌───────────────┐
    │ cmd_list?     │
    └───┬───────┬───┘
        │ YES   │ NO → return last_exit_status
        ▼       │
    ┌───────────┴───────┐
    │ cmd_list->next?   │  (più di un comando?)
    └───┬───────────┬───┘
        │ YES       │ NO
        │           │
        ▼           ▼
┌───────────────┐  ┌──────────────────┐
│ PIPELINE      │  │ SINGLE COMMAND   │
│execute_       │  │execute_single_   │
│pipeline()     │  │cmd()             │
└───────────────┘  └────────┬─────────┘
                            │
                            ▼
                   ┌────────────────┐
                   │ is_builtin()?  │
                   └────┬───────┬───┘
                        │ YES   │ NO
                        ▼       ▼
                ┌─────────────┐ ┌──────────────┐
                │execute_     │ │exec_external_│
                │builtin()    │ │cmd()         │
                └─────────────┘ └──────────────┘
```

---

## 7.2 Executor Principale

```c
int executor(t_data *data)
{
    // ═══════════════════════════════════════════════════════
    // CASO 1: Nessun comando
    // ═══════════════════════════════════════════════════════
    if (!data->cmd_list)
        return (data->last_exit_status);
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Pipeline (più comandi con pipe)
    // ═══════════════════════════════════════════════════════
    if (data->cmd_list->next)
        return (execute_pipeline(data));
    
    // ═══════════════════════════════════════════════════════
    // CASO 3: Comando singolo
    // ═══════════════════════════════════════════════════════
    return (execute_single_cmd(data, data->cmd_list));
}
```

---

## 7.3 Esecuzione Comando Singolo

```c
int execute_single_cmd(t_data *data, t_cmd *cmd)
{
    // ═══════════════════════════════════════════════════════
    // Controlla se è un builtin
    // ═══════════════════════════════════════════════════════
    if (is_builtin(cmd->args[0]))
        return (execute_builtin(cmd, data));
    
    // ═══════════════════════════════════════════════════════
    // Altrimenti è comando esterno
    // ═══════════════════════════════════════════════════════
    return (exec_external_cmd(data, cmd));
}
```

---

## 7.4 Riconoscimento Builtins

### Is Builtin

```c
int is_builtin(char *cmd)
{
    if (ft_strcmp(cmd, "echo") == 0)
        return (1);
    if (ft_strcmp(cmd, "cd") == 0)
        return (1);
    if (ft_strcmp(cmd, "pwd") == 0)
        return (1);
    if (ft_strcmp(cmd, "export") == 0)
        return (1);
    if (ft_strcmp(cmd, "unset") == 0)
        return (1);
    if (ft_strcmp(cmd, "env") == 0)
        return (1);
    if (ft_strcmp(cmd, "exit") == 0)
        return (1);
    
    return (0);
}
```

### Tabella Builtins

| Builtin | Funzione | Descrizione |
|---------|----------|-------------|
| `echo` | `builtin_echo()` | Stampa argomenti |
| `cd` | `builtin_cd()` | Cambia directory |
| `pwd` | `builtin_pwd()` | Stampa directory corrente |
| `export` | `builtin_export()` | Esporta variabili |
| `unset` | `builtin_unset()` | Rimuove variabili |
| `env` | `builtin_env()` | Stampa environment |
| `exit` | `builtin_exit()` | Esce dalla shell |

---

### Execute Builtin

```c
int execute_builtin(t_cmd *cmd, t_data *data)
{
    if (ft_strcmp(cmd->args[0], "echo") == 0)
        return (builtin_echo(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "cd") == 0)
        return (builtin_cd(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "pwd") == 0)
        return (builtin_pwd(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "export") == 0)
        return (builtin_export(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "unset") == 0)
        return (builtin_unset(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "env") == 0)
        return (builtin_env(cmd->args, data));
    if (ft_strcmp(cmd->args[0], "exit") == 0)
        return (builtin_exit(cmd->args, data));
    
    return (ERROR);
}
```

---

## 7.5 Esecuzione Comando Esterno

### Process Flow

```
┌─────────────────────────────────────────────────────────────┐
│             EXEC EXTERNAL COMMAND FLOW                      │
└─────────────────────────────────────────────────────────────┘

1. find_command_path()
   ├─ Se contiene '/' → usa path diretto
   └─ Altrimenti → cerca in $PATH
   
2. fork()
   ├─ pid == -1 → ERRORE
   ├─ pid == 0  → PROCESSO FIGLIO
   │              ├─ apply_redirections()
   │              └─ execve()
   └─ pid > 0   → PROCESSO PADRE
                  ├─ waitpid()
                  └─ return exit_status
```

---

### Implementazione

```c
int exec_external_cmd(t_data *data, t_cmd *cmd)
{
    pid_t   pid;
    int     status;
    char    *cmd_path;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Trova path del comando
    // ═══════════════════════════════════════════════════════
    cmd_path = find_command_path(cmd->args[0], data->envp);
    if (!cmd_path)
    {
        print_error(cmd->args[0], "command not found");
        return (CMD_NOT_FOUND);  // 127
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Fork processo
    // ═══════════════════════════════════════════════════════
    pid = fork();
    if (pid == -1)
    {
        print_error("fork", "failed");
        free(cmd_path);
        return (ERROR);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: PROCESSO FIGLIO
    // ═══════════════════════════════════════════════════════
    if (pid == 0)
    {
        // ───────────────────────────────────────────────────
        // Applica redirections (se presenti)
        // ───────────────────────────────────────────────────
        if (apply_redirections(cmd) != SUCCESS)
            exit(ERROR);
        
        // ───────────────────────────────────────────────────
        // Esegui comando (sostituisce processo)
        // ───────────────────────────────────────────────────
        if (execve(cmd_path, cmd->args, data->envp) == -1)
        {
            print_error(cmd->args[0], "execution failed");
            free(cmd_path);
            exit(CMD_NOT_EXECUTABLE);  // 126
        }
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: PROCESSO PADRE
    // ═══════════════════════════════════════════════════════
    free(cmd_path);
    waitpid(pid, &status, 0);  // Aspetta terminazione figlio
    
    // ═══════════════════════════════════════════════════════
    // STEP 5: Analizza exit status
    // ═══════════════════════════════════════════════════════
    if (WIFEXITED(status))
        return (WEXITSTATUS(status));      // Exit normale
    if (WIFSIGNALED(status))
        return (128 + WTERMSIG(status));   // Terminato da segnale
    
    return (SUCCESS);
}
```

### Tabella Exit Codes

| Code | Macro | Significato |
|------|-------|-------------|
| `0` | `SUCCESS` | Comando eseguito con successo |
| `1` | `ERROR` | Errore generico |
| `126` | `CMD_NOT_EXECUTABLE` | Comando non eseguibile |
| `127` | `CMD_NOT_FOUND` | Comando non trovato |
| `128+N` | - | Comando terminato dal segnale N |

---

## 7.6 Ricerca PATH

### Find Command Path

```c
char *find_command_path(char *cmd, char **envp)
{
    char    **paths;
    char    *path_value;
    char    *full_path;
    int     i;
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: Path assoluto o relativo (contiene '/')
    // ═══════════════════════════════════════════════════════
    if (ft_strchr(cmd, '/'))
    {
        if (is_executable(cmd))
            return (ft_strdup(cmd));
        return (NULL);
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Cerca in $PATH
    // ═══════════════════════════════════════════════════════
    
    // Step 1: Ottieni valore di PATH
    path_value = get_env_value("PATH", envp);
    if (!path_value)
        return (NULL);
    
    // Step 2: Dividi PATH in directory
    paths = ft_split(path_value, ':');
    free(path_value);
    if (!paths)
        return (NULL);
    
    // Step 3: Prova ogni directory
    i = 0;
    while (paths[i])
    {
        full_path = check_path_dir(paths[i], cmd);
        if (full_path)  // Trovato!
        {
            free_array(paths);
            return (full_path);
        }
        i++;
    }
    
    // Step 4: Non trovato in nessuna directory
    free_array(paths);
    return (NULL);
}
```

---

### Check Path Dir Helper

```c
char *check_path_dir(char *dir, char *cmd)
{
    char *temp;
    char *full_path;
    
    // Costruisci path completo: "/bin" + "/" + "ls" = "/bin/ls"
    temp = ft_strjoin(dir, "/");
    full_path = ft_strjoin(temp, cmd);
    free(temp);
    
    // Controlla se è eseguibile
    if (is_executable(full_path))
        return (full_path);
    
    // Non eseguibile, libera e ritorna NULL
    free(full_path);
    return (NULL);
}
```

---

### Is Executable Helper

```c
int is_executable(char *path)
{
    struct stat statbuf;
    
    // Controlla se file esiste
    if (stat(path, &statbuf) == -1)
        return (0);
    
    // Controlla se è file regolare
    if (!S_ISREG(statbuf.st_mode))
        return (0);
    
    // Controlla permessi di esecuzione
    if (access(path, X_OK) == -1)
        return (0);
    
    return (1);
}
```

---

### Esempio Ricerca PATH

```
Comando: "ls"
PATH="/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

┌────────────────────────────────────────────────────────┐
│              RICERCA COMANDO IN PATH                   │
└────────────────────────────────────────────────────────┘

Step 1: Split PATH
  paths[0] = "/usr/local/bin"
  paths[1] = "/usr/bin"
  paths[2] = "/bin"
  paths[3] = "/usr/sbin"
  paths[4] = "/sbin"
  paths[5] = NULL

Step 2: Prova /usr/local/bin/ls
  → stat() → file non esiste
  → continua

Step 3: Prova /usr/bin/ls
  → stat() → file esiste ✓
  → S_ISREG() → è file regolare ✓
  → access(X_OK) → eseguibile ✓
  → TROVATO!

Result: "/usr/bin/ls"
```

---

# 8. MODULO 5: BUILTINS

## 8.1 Perché Esistono i Builtins?

I **builtins** sono comandi **integrati nella shell** che **DEVONO** essere eseguiti nel processo della shell stessa, non in un processo figlio.

### Motivazione

```
┌─────────────────────────────────────────────────────────────┐
│                   PROBLEMA CON FORK                         │
└─────────────────────────────────────────────────────────────┘

Scenario: cd /tmp eseguito in processo figlio

SHELL (padre)              PROCESSO FIGLIO
├─ pwd: /home             ├─ pwd: /home
├─ fork() ────────────────→├─ cd /tmp
│                          ├─ pwd: /tmp ✓
│                          └─ exit
├─ wait()
├─ pwd: /home ✗           (cambio perso!)
```

**Soluzione:** I builtins che modificano lo stato della shell (come `cd`, `export`, `exit`) **DEVONO** essere eseguiti direttamente nella shell.

---

## 8.2 Lista Builtins Implementati

| Builtin | Categoria | Motivo Builtin |
|---------|-----------|----------------|
| `echo` | Output | Compatibilità (flag `-n`) |
| `cd` | Filesystem | Cambia directory della shell |
| `pwd` | Filesystem | Stampa directory corrente |
| `export` | Environment | Modifica environment della shell |
| `unset` | Environment | Modifica environment della shell |
| `env` | Environment | Stampa environment |
| `exit` | Control | Termina la shell stessa |

---

## 8.3 ECHO

### Sintassi

```bash
echo [-n] [string ...]
```

### Opzioni

| Flag | Descrizione |
|------|-------------|
| `-n` | Non stampa newline finale |

---

### Implementazione

```c
int builtin_echo(char **args, t_data *data)
{
    int i;
    int newline;
    
    (void)data;  // Non usato
    
    i = 1;
    newline = 1;  // Default: stampa newline
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Controlla flag -n
    // ═══════════════════════════════════════════════════════
    if (args[1] && ft_strcmp(args[1], "-n") == 0)
    {
        newline = 0;
        i = 2;  // Salta "-n"
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Stampa argomenti
    // ═══════════════════════════════════════════════════════
    while (args[i])
    {
        ft_putstr_fd(args[i], STDOUT_FILENO);
        
        // Stampa spazio tra argomenti (ma non dopo ultimo)
        if (args[i + 1])
            ft_putstr_fd(" ", STDOUT_FILENO);
        
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Stampa newline (se non -n)
    // ═══════════════════════════════════════════════════════
    if (newline)
        ft_putstr_fd("\n", STDOUT_FILENO);
    
    return (SUCCESS);
}
```

### Esempi

```bash
echo hello world
# Output: hello world\n

echo -n hello
# Output: hello (senza \n)

echo "hello    world"
# Output: hello    world\n

echo
# Output: \n
```

---

## 8.4 CD

### Sintassi

```bash
cd [directory]
```

### Comportamenti

| Input | Azione |
|-------|--------|
| `cd` | Va a `$HOME` |
| `cd path` | Va a `path` |
| `cd -` | Va a `$OLDPWD` (directory precedente) |

---

### Implementazione

```c
int builtin_cd(char **args, t_data *data)
{
    char *path;
    char cwd[1024];
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Determina directory target
    // ═══════════════════════════════════════════════════════
    if (!args[1])  // Nessun argomento → HOME
    {
        path = get_env_value("HOME", data->envp);
        if (!path)
        {
            print_error("cd", "HOME not set");
            return (ERROR);
        }
    }
    else  // Argomento specificato
    {
        path = args[1];
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Cambia directory
    // ═══════════════════════════════════════════════════════
    if (chdir(path) == -1)
    {
        print_error("cd", path);
        if (!args[1])
            free(path);
        return (ERROR);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Aggiorna PWD e OLDPWD nell'environment
    // ═══════════════════════════════════════════════════════
    if (getcwd(cwd, sizeof(cwd)))
    {
        char *old_pwd = get_env_value("PWD", data->envp);
        
        set_env_value("OLDPWD", old_pwd, data);
        set_env_value("PWD", cwd, data);
        
        free(old_pwd);
    }
    
    if (!args[1])
        free(path);
    
    return (SUCCESS);
}
```

### Esempi

```bash
cd /tmp
pwd
# Output: /tmp

cd ..
pwd
# Output: /

cd
pwd
# Output: /Users/giusmery (HOME)
```

---

## 8.5 PWD

### Sintassi

```bash
pwd
```

### Implementazione

```c
int builtin_pwd(char **args, t_data *data)
{
    char cwd[1024];
    
    (void)args;  // Non usato
    (void)data;  // Non usato
    
    // ═══════════════════════════════════════════════════════
    // Ottieni directory corrente
    // ═══════════════════════════════════════════════════════
    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putendl_fd(cwd, STDOUT_FILENO);
        return (SUCCESS);
    }
    
    // ═══════════════════════════════════════════════════════
    // Errore (directory cancellata, permessi, ecc.)
    // ═══════════════════════════════════════════════════════
    print_error("pwd", "getcwd failed");
    return (ERROR);
}
```

### Esempio

```bash
pwd
# Output: /Users/giusmery/minishell
```

---

## 8.6 EXPORT

### Sintassi

```bash
export [VAR=value ...]
```

### Comportamenti

| Input | Azione |
|-------|--------|
| `export` | Stampa tutte le variabili in formato `declare -x` |
| `export VAR=value` | Crea/aggiorna variabile |
| `export VAR1=a VAR2=b` | Crea/aggiorna multiple variabili |

---

### Implementazione

```c
int builtin_export(char **args, t_data *data)
{
    char *key;
    char *value;
    int  i;
    int  ret;
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: Nessun argomento → stampa variabili
    // ═══════════════════════════════════════════════════════
    if (!args[1])
    {
        print_export_vars(data);
        return (SUCCESS);
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Esporta variabili
    // ═══════════════════════════════════════════════════════
    ret = SUCCESS;
    i = 1;
    while (args[i])
    {
        // ───────────────────────────────────────────────────
        // Parsa argomento (KEY=value)
        // ───────────────────────────────────────────────────
        if (parse_export_arg(args[i], &key, &value))
        {
            set_env_value(key, value, data);
            free(key);
            free(value);
        }
        else
        {
            print_error("export", "invalid identifier");
            ret = ERROR;
        }
        i++;
    }
    
    return (ret);
}
```

---

### Parse Export Arg

```c
int parse_export_arg(char *arg, char **key, char **value)
{
    char *equal;
    
    // Trova '='
    equal = ft_strchr(arg, '=');
    if (!equal)
        return (0);  // Deve contenere '='
    
    // Estrai key (prima di '=')
    *key = ft_strndup(arg, equal - arg);
    
    // Estrai value (dopo '=')
    *value = ft_strdup(equal + 1);
    
    // Valida identifier (solo lettere, numeri, _)
    if (!is_valid_identifier(*key))
    {
        free(*key);
        free(*value);
        return (0);
    }
    
    return (1);
}
```

---

### Print Export Vars

```c
void print_export_vars(t_data *data)
{
    int i;
    
    i = 0;
    while (data->envp[i])
    {
        print_single_export(data->envp[i]);
        i++;
    }
}

void print_single_export(char *env_var)
{
    int j;
    
    ft_putstr_fd("declare -x ", STDOUT_FILENO);
    
    // Stampa key
    j = 0;
    while (env_var[j] && env_var[j] != '=')
    {
        write(STDOUT_FILENO, &env_var[j], 1);
        j++;
    }
    
    // Se ha valore, stampa con quotes
    if (env_var[j] == '=')
    {
        write(STDOUT_FILENO, "=\"", 2);
        j++;
        while (env_var[j])
        {
            write(STDOUT_FILENO, &env_var[j], 1);
            j++;
        }
        write(STDOUT_FILENO, "\"", 1);
    }
    
    write(STDOUT_FILENO, "\n", 1);
}
```

### Esempi

```bash
export TEST=hello
echo $TEST
# Output: hello

export
# Output:
# declare -x HOME="/Users/giusmery"
# declare -x PATH="/bin:/usr/bin"
# declare -x TEST="hello"
# ...
```

---

## 8.7 UNSET

### Sintassi

```bash
unset [VAR ...]
```

### Implementazione

```c
int builtin_unset(char **args, t_data *data)
{
    int i;
    
    // Nessun argomento → non fa nulla
    if (!args[1])
        return (SUCCESS);
    
    // ═══════════════════════════════════════════════════════
    // Rimuovi ogni variabile specificata
    // ═══════════════════════════════════════════════════════
    i = 1;
    while (args[i])
    {
        unset_env_value(args[i], data);
        i++;
    }
    
    return (SUCCESS);
}
```

### Esempio

```bash
export TEST=hello
echo $TEST
# Output: hello

unset TEST
echo $TEST
# Output: (vuoto)
```

---

## 8.8 ENV

### Sintassi

```bash
env
```

### Implementazione

```c
int builtin_env(char **args, t_data *data)
{
    int i;
    
    (void)args;  // Non usato
    
    // ═══════════════════════════════════════════════════════
    // Stampa tutte le variabili d'ambiente
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (data->envp[i])
    {
        ft_putendl_fd(data->envp[i], STDOUT_FILENO);
        i++;
    }
    
    return (SUCCESS);
}
```

### Esempio

```bash
env
# Output:
# PATH=/bin:/usr/bin
# HOME=/Users/giusmery
# USER=giusmery
# ...
```

---

## 8.9 EXIT

### Sintassi

```bash
exit [n]
```

### Comportamenti

| Input | Azione |
|-------|--------|
| `exit` | Esce con `$?` (last exit status) |
| `exit 0` | Esce con codice 0 |
| `exit 42` | Esce con codice 42 |
| `exit abc` | Errore: numeric argument required |
| `exit 1 2` | Errore: too many arguments (NON esce) |

---

### Implementazione

```c
int builtin_exit(char **args, t_data *data)
{
    int exit_code;
    
    ft_putendl_fd("exit", STDOUT_FILENO);
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: Nessun argomento
    // ═══════════════════════════════════════════════════════
    if (!args[1])
    {
        cleanup_data(data);
        exit(data->last_exit_status);
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Argomento non numerico
    // ═══════════════════════════════════════════════════════
    if (!is_numeric(args[1]))
    {
        print_error("exit", "numeric argument required");
        cleanup_data(data);
        exit(2);
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 3: Troppi argomenti
    // ═══════════════════════════════════════════════════════
    if (args[2])
    {
        print_error("exit", "too many arguments");
        return (ERROR);  // NON esce!
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 4: Exit con codice specificato
    // ═══════════════════════════════════════════════════════
    exit_code = ft_atoi(args[1]);
    cleanup_data(data);
    exit(exit_code % 256);  // Modulo 256 (range 0-255)
}
```

---

### Is Numeric Helper

```c
int is_numeric(char *str)
{
    int i;
    
    i = 0;
    
    // Salta segno iniziale (+/-)
    if (str[i] == '+' || str[i] == '-')
        i++;
    
    // Deve avere almeno una cifra
    if (!str[i])
        return (0);
    
    // Tutte le cifre devono essere numeri
    while (str[i])
    {
        if (!ft_isdigit(str[i]))
            return (0);
        i++;
    }
    
    return (1);
}
```

### Esempi

```bash
exit
# Esce con last exit status

exit 0
# Esce con 0

exit 42
# Esce con 42

exit 999
# Esce con 231 (999 % 256)

exit hello
# Output: minishell: exit: numeric argument required
# Esce con 2

exit 1 2
# Output: minishell: exit: too many arguments
# NON esce, ritorna al prompt
```

---

# 9. MODULO 6: REDIRECTIONS

## 9.1 Tipi di Redirection

### Enumerazione

```c
typedef enum e_redir_type
{
    REDIR_IN,       // <  - Input da file
    REDIR_OUT,      // >  - Output su file (truncate)
    REDIR_APPEND,   // >> - Output su file (append)
    REDIR_HEREDOC   // << - Input da heredoc
} t_redir_type;
```

### Tabella Completa

| Tipo | Simbolo | Flag `open()` | FD Target | Descrizione |
|------|---------|---------------|-----------|-------------|
| `REDIR_IN` | `<` | `O_RDONLY` | `STDIN` | Legge input da file |
| `REDIR_OUT` | `>` | `O_WRONLY\|O_CREAT\|O_TRUNC` | `STDOUT` | Scrive output su file (sovrascrive) |
| `REDIR_APPEND` | `>>` | `O_WRONLY\|O_CREAT\|O_APPEND` | `STDOUT` | Scrive output su file (append) |
| `REDIR_HEREDOC` | `<<` | - (usa pipe) | `STDIN` | Legge input da heredoc |

---

## 9.2 Come Funzionano le Redirections?

Le redirections **modificano stdin/stdout** del processo usando la system call `dup2()`.

### Diagramma Concettuale

```
┌─────────────────────────────────────────────────────────────┐
│                  BEFORE REDIRECTION                         │
└─────────────────────────────────────────────────────────────┘

PROCESSO
├─ FD 0 (STDIN)  ────────> Terminal Input
├─ FD 1 (STDOUT) ────────> Terminal Output
└─ FD 2 (STDERR) ────────> Terminal Error


┌─────────────────────────────────────────────────────────────┐
│           AFTER: cat < input.txt > output.txt               │
└─────────────────────────────────────────────────────────────┘

PROCESSO
├─ FD 0 (STDIN)  ────────> input.txt  (via dup2)
├─ FD 1 (STDOUT) ────────> output.txt (via dup2)
└─ FD 2 (STDERR) ────────> Terminal Error

FD 3: input.txt  (aperto e poi chiuso)
FD 4: output.txt (aperto e poi chiuso)
```

---

## 9.3 Input Redirection (`<`)

### Comportamento

```bash
cat < file.txt
```

**Cosa succede:**
1. Apre `file.txt` in lettura → `fd = 3`
2. `dup2(3, 0)` → stdin (FD 0) ora punta a `file.txt`
3. `close(3)` → chiude FD originale
4. `cat` legge da stdin (che ora è `file.txt`)

---

### Esempio Dettagliato

```
┌─────────────────────────────────────────────────────────────┐
│              INPUT REDIRECTION STEP-BY-STEP                 │
└─────────────────────────────────────────────────────────────┘

Comando: cat < file.txt
File descriptor table iniziale:

FD  │ Punta a
────┼─────────────────
 0  │ Terminal (stdin)
 1  │ Terminal (stdout)
 2  │ Terminal (stderr)

────────────────────────────────────────────────────────────
STEP 1: open("file.txt", O_RDONLY)
────────────────────────────────────────────────────────────
FD  │ Punta a
────┼─────────────────
 0  │ Terminal (stdin)
 1  │ Terminal (stdout)
 2  │ Terminal (stderr)
 3  │ file.txt ← NUOVO

────────────────────────────────────────────────────────────
STEP 2: dup2(3, 0)
────────────────────────────────────────────────────────────
Duplica FD 3 su FD 0 (stdin)

FD  │ Punta a
────┼─────────────────
 0  │ file.txt ← MODIFICATO!
 1  │ Terminal (stdout)
 2  │ Terminal (stderr)
 3  │ file.txt

────────────────────────────────────────────────────────────
STEP 3: close(3)
────────────────────────────────────────────────────────────
FD  │ Punta a
────┼─────────────────
 0  │ file.txt
 1  │ Terminal (stdout)
 2  │ Terminal (stderr)
 3  │ [chiuso]

────────────────────────────────────────────────────────────
STEP 4: execve("/bin/cat", ...)
────────────────────────────────────────────────────────────
cat legge da stdin (FD 0) → legge da file.txt ✓
```

---

## 9.4 Output Redirection (`>`)

### Comportamento

```bash
echo hello > file.txt
```

**Cosa succede:**
1. Apre `file.txt` in scrittura (truncate) → `fd = 3`
2. `dup2(3, 1)` → stdout (FD 1) ora punta a `file.txt`
3. `close(3)` → chiude FD originale
4. `echo hello` scrive su stdout (che ora è `file.txt`)

---

### Flags di `open()`

```c
fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
```

| Flag | Descrizione |
|------|-------------|
| `O_WRONLY` | Apri in sola scrittura |
| `O_CREAT` | Crea file se non esiste |
| `O_TRUNC` | Tronca file a 0 byte se esiste (sovrascrive) |
| `0644` | Permessi: `rw-r--r--` |

---

## 9.5 Append Redirection (`>>`)

### Comportamento

```bash
echo world >> file.txt
```

**Differenza con `>`:**
- `>` → **sovrascrive** (O_TRUNC)
- `>>` → **appende** (O_APPEND)

### Flags di `open()`

```c
fd = open("file.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
```

| Flag | Descrizione |
|------|-------------|
| `O_APPEND` | Scrive sempre alla fine del file |

---

### Esempio Comparativo

```bash
# Scenario 1: >
echo hello > test.txt
cat test.txt
# Output: hello

echo world > test.txt
cat test.txt
# Output: world  (hello è stato sovrascritto!)

# Scenario 2: >>
echo hello > test.txt
cat test.txt
# Output: hello

echo world >> test.txt
cat test.txt
# Output:
# hello
# world  (aggiunto!)
```

---

## 9.6 Implementazione Apply Redirections

### Funzione Principale

```c
int apply_redirections(t_cmd *cmd)
{
    t_redir *redir;
    int     fd;
    
    redir = cmd->redirs;
    
    // ═══════════════════════════════════════════════════════
    // Itera su tutte le redirections del comando
    // ═══════════════════════════════════════════════════════
    while (redir)
    {
        // ───────────────────────────────────────────────────
        // STEP 1: Apri file/heredoc
        // ───────────────────────────────────────────────────
        fd = open_redir_file(redir);
        if (fd == -1)
            return (ERROR);
        
        // ───────────────────────────────────────────────────
        // STEP 2: Applica redirection (dup2)
        // ───────────────────────────────────────────────────
        if (apply_single_redir(redir, fd) == ERROR)
            return (ERROR);
        
        redir = redir->next;
    }
    
    return (SUCCESS);
}
```

---

### Open Redir File

```c
int open_redir_file(t_redir *redir)
{
    int fd;
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: Input (<)
    // ═══════════════════════════════════════════════════════
    if (redir->type == REDIR_IN)
    {
        fd = open(redir->file, O_RDONLY);
        if (fd == -1)
            print_error(redir->file, "no such file or directory");
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Output (>)
    // ═══════════════════════════════════════════════════════
    else if (redir->type == REDIR_OUT)
    {
        fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
            print_error(redir->file, "cannot create file");
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 3: Append (>>)
    // ═══════════════════════════════════════════════════════
    else if (redir->type == REDIR_APPEND)
    {
        fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1)
            print_error(redir->file, "cannot create file");
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 4: Heredoc (<<)
    // ═══════════════════════════════════════════════════════
    else if (redir->type == REDIR_HEREDOC)
    {
        fd = handle_heredoc(redir->file);
    }
    
    return (fd);
}
```

---

### Apply Single Redir

```c
int apply_single_redir(t_redir *redir, int fd)
{
    int target_fd;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Determina target FD
    // ═══════════════════════════════════════════════════════
    if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
        target_fd = STDIN_FILENO;   // FD 0
    else
        target_fd = STDOUT_FILENO;  // FD 1
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Duplica FD
    // ═══════════════════════════════════════════════════════
    if (dup2(fd, target_fd) == -1)
    {
        close(fd);
        print_error("dup2", "failed");
        return (ERROR);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Chiudi FD originale
    // ═══════════════════════════════════════════════════════
    close(fd);
    
    return (SUCCESS);
}
```

---

## 9.7 Heredoc (`<<`)

### Cos'è un Heredoc?

Un **heredoc** permette di passare input multilinea a un comando fino a un **delimiter**.

### Sintassi

```bash
cat << EOF
line 1
line 2
line 3
EOF
```

**Output:**
```
line 1
line 2
line 3
```

---

### Come Funziona Internamente

```
┌─────────────────────────────────────────────────────────────┐
│                    HEREDOC MECHANISM                        │
└─────────────────────────────────────────────────────────────┘

1. Crea PIPE
   ├─ pipe_fd[0] = read end
   └─ pipe_fd[1] = write end

2. LOOP: Leggi linee con readline("> ")
   ├─ Se linea == delimiter → BREAK
   └─ Altrimenti → write(pipe_fd[1], linea)

3. Chiudi write end
   └─ close(pipe_fd[1])

4. Return read end
   └─ return pipe_fd[0]  (usato come stdin)
```

---

### Implementazione

```c
int handle_heredoc(char *delimiter)
{
    int  pipe_fd[2];
    char *line;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Crea pipe
    // ═══════════════════════════════════════════════════════
    if (pipe(pipe_fd) == -1)
    {
        print_error("heredoc", "pipe failed");
        return (-1);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Leggi linee fino a delimiter
    // ═══════════════════════════════════════════════════════
    while (1)
    {
        line = readline("> ");
        
        // ───────────────────────────────────────────────────
        // Fine heredoc (EOF o delimiter)
        // ───────────────────────────────────────────────────
        if (!line || ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // ───────────────────────────────────────────────────
        // Scrivi linea nella pipe
        // ───────────────────────────────────────────────────
        write(pipe_fd[1], line, ft_strlen(line));
        write(pipe_fd[1], "\n", 1);
        
        free(line);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Chiudi write end
    // ═══════════════════════════════════════════════════════
    close(pipe_fd[1]);
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Return read end (usato come stdin)
    // ═══════════════════════════════════════════════════════
    return (pipe_fd[0]);
}
```

---

### Esempio Dettagliato

```
┌─────────────────────────────────────────────────────────────┐
│              HEREDOC EXECUTION FLOW                         │
└─────────────────────────────────────────────────────────────┘

Comando: cat << END
> hello
> world
> END

────────────────────────────────────────────────────────────
STEP 1: pipe(pipe_fd)
────────────────────────────────────────────────────────────
pipe_fd[0] = 3 (read end)
pipe_fd[1] = 4 (write end)

────────────────────────────────────────────────────────────
STEP 2: readline("> ") → "hello"
────────────────────────────────────────────────────────────
line = "hello"
line != "END" → continua
write(4, "hello", 5)
write(4, "\n", 1)
free(line)

Contenuto pipe: "hello\n"

────────────────────────────────────────────────────────────
STEP 3: readline("> ") → "world"
────────────────────────────────────────────────────────────
line = "world"
line != "END" → continua
write(4, "world", 5)
write(4, "\n", 1)
free(line)

Contenuto pipe: "hello\nworld\n"

────────────────────────────────────────────────────────────
STEP 4: readline("> ") → "END"
────────────────────────────────────────────────────────────
line = "END"
line == "END" → BREAK
free(line)

────────────────────────────────────────────────────────────
STEP 5: close(pipe_fd[1])
────────────────────────────────────────────────────────────
Chiude write end

FD  │ Punta a
────┼─────────────────
 3  │ pipe (read)  ✓
 4  │ [chiuso]

────────────────────────────────────────────────────────────
STEP 6: return pipe_fd[0] (FD 3)
────────────────────────────────────────────────────────────
Questo FD viene usato come stdin da apply_redirections():
  dup2(3, 0) → stdin legge dalla pipe

────────────────────────────────────────────────────────────
STEP 7: cat esegue
────────────────────────────────────────────────────────────
cat legge da stdin (FD 0) → legge dalla pipe
Output:
hello
world
```

---

## 9.8 Multiple Redirections

Minishell supporta **multiple redirections** sullo stesso comando!

### Regola: Last Wins

Quando ci sono multiple redirections dello stesso tipo, **l'ultima vince**.

---

### Esempio 1: Multiple Input

```bash
cat < file1.txt < file2.txt
```

**Cosa succede:**
1. Apre `file1.txt` → `dup2(fd, 0)` → stdin = file1.txt
2. Apre `file2.txt` → `dup2(fd, 0)` → stdin = file2.txt (sovrascrive!)
3. `cat` legge da `file2.txt` ✓

---

### Esempio 2: Multiple Output

```bash
echo hello > file1.txt > file2.txt
```

**Cosa succede:**
1. Apre `file1.txt` → `dup2(fd, 1)` → stdout = file1.txt
2. Apre `file2.txt` → `dup2(fd, 1)` → stdout = file2.txt (sovrascrive!)
3. `echo hello` scrive su `file2.txt` ✓

**Risultato:**
- `file1.txt` → vuoto (creato ma non scritto)
- `file2.txt` → "hello"

---

### Esempio 3: Input + Output

```bash
cat < input.txt > output.txt
```

**Cosa succede:**
1. Apre `input.txt` → `dup2(fd, 0)` → stdin = input.txt
2. Apre `output.txt` → `dup2(fd, 1)` → stdout = output.txt
3. `cat` legge da `input.txt` e scrive su `output.txt` ✓

**Risultato:** Copia `input.txt` → `output.txt`

---

### Esempio 4: Redirections + Pipe

```bash
cat < input.txt | grep hello > output.txt
```

**Cosa succede:**

```
CMD 1: cat < input.txt
  ├─ stdin = input.txt
  └─ stdout = pipe

CMD 2: grep hello > output.txt
  ├─ stdin = pipe
  └─ stdout = output.txt

Flow: input.txt → cat → pipe → grep → output.txt
```

---

## 9.9 Ordine di Applicazione

### Regola Importante

Le redirections sono applicate **nell'ordine in cui appaiono** nella lista.

```c
// Lista redirections per: cat < in.txt > out.txt
cmd->redirs:
  [0] → type=REDIR_IN,  file="in.txt"
  [1] → type=REDIR_OUT, file="out.txt"

// Applicate in ordine:
1. Applica REDIR_IN  → stdin  = in.txt
2. Applica REDIR_OUT → stdout = out.txt
```

---

# 10. MODULO 7: PIPES

## 10.1 Cos'è una Pipe?

Una **pipe** è un meccanismo IPC (Inter-Process Communication) che connette lo **stdout** di un comando allo **stdin** del comando successivo.

### Diagramma Concettuale

```
┌─────────────────────────────────────────────────────────────┐
│                    PIPELINE CONCEPT                         │
└─────────────────────────────────────────────────────────────┘

Comando: echo hello | cat | wc -l

       ┌─────────┐        ┌─────┐        ┌───────┐
       │  echo   │  pipe1 │ cat │  pipe2 │ wc -l │
       │  hello  ├───────>│     ├───────>│       │
       └─────────┘        └─────┘        └───────┘
          ↓                  ↓                ↓
       "hello\n"         "hello\n"          "1\n"
       
STDOUT ────┐         ┌──> STDIN
           │  PIPE   │
           └────┬────┘
                │
           [kernel buffer]
```

---

## 10.2 System Call `pipe()`

### Prototipo

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

### Comportamento

```c
int pipe_fd[2];
pipe(pipe_fd);

// Dopo la chiamata:
// pipe_fd[0] = read end  (input)
// pipe_fd[1] = write end (output)
```

### Tabella File Descriptors

| Index | Nome | Descrizione | Uso |
|-------|------|-------------|-----|
| `pipe_fd[0]` | Read end | Lettura dalla pipe | `read(pipe_fd[0], ...)` |
| `pipe_fd[1]` | Write end | Scrittura nella pipe | `write(pipe_fd[1], ...)` |

---

### Esempio Semplice

```c
int main(void)
{
    int     pipe_fd[2];
    pid_t   pid;
    char    buf[100];
    
    // Crea pipe
    pipe(pipe_fd);
    
    pid = fork();
    
    if (pid == 0)  // FIGLIO (scrittore)
    {
        close(pipe_fd[0]);              // Chiudi read end
        write(pipe_fd[1], "hello", 5);  // Scrivi nella pipe
        close(pipe_fd[1]);              // Chiudi write end
        exit(0);
    }
    else  // PADRE (lettore)
    {
        close(pipe_fd[1]);              // Chiudi write end
        read(pipe_fd[0], buf, 100);     // Leggi dalla pipe
        close(pipe_fd[0]);              // Chiudi read end
        
        printf("Letto: %s\n", buf);     // Output: hello
        wait(NULL);
    }
    
    return (0);
}
```

---

## 10.3 Pipeline con N Comandi

### Algoritmo Generale

```
Per ogni comando nella pipeline:
  1. Se NON è l'ultimo comando:
       → Crea nuova pipe (curr_pipe)
  
  2. Fork processo figlio
  
  3. Nel FIGLIO:
       → Se NON è il primo:
           dup2(prev_pipe[0], STDIN)   (leggi da pipe precedente)
       → Se NON è l'ultimo:
           dup2(curr_pipe[1], STDOUT)  (scrivi su pipe corrente)
       → Chiudi tutti i file descriptor delle pipe
       → Applica redirections (se presenti)
       → execve(comando)
  
  4. Nel PADRE:
       → Chiudi prev_pipe (se esiste)
       → Salva curr_pipe come prev_pipe
       → Continua al prossimo comando
  
  5. Dopo aver lanciato tutti i processi:
       → Aspetta tutti i figli (waitpid)
       → Return exit status dell'ultimo
```

---

### Diagramma Visuale

```
┌─────────────────────────────────────────────────────────────┐
│          PIPELINE: cmd1 | cmd2 | cmd3                       │
└─────────────────────────────────────────────────────────────┘

Iterazione 1: cmd1
  ┌─────────────────────────────────────────────────────┐
  │ prev_pipe: NULL                                     │
  │ curr_pipe: [3,4]  (crea nuova pipe)                 │
  │                                                     │
  │ FIGLIO 1:                                           │
  │   NON primo? NO  → stdin  = terminal                │
  │   NON ultimo? SÌ → stdout = pipe[4]                 │
  │   execve(cmd1)                                      │
  │                                                     │
  │ PADRE:                                              │
  │   prev_pipe = [3,4]                                 │
  └─────────────────────────────────────────────────────┘

Iterazione 2: cmd2
  ┌─────────────────────────────────────────────────────┐
  │ prev_pipe: [3,4]                                    │
  │ curr_pipe: [5,6]  (crea nuova pipe)                 │
  │                                                     │
  │ FIGLIO 2:                                           │
  │   NON primo? SÌ  → stdin  = prev_pipe[3]            │
  │   NON ultimo? SÌ → stdout = curr_pipe[6]            │
  │   execve(cmd2)                                      │
  │                                                     │
  │ PADRE:                                              │
  │   close(prev_pipe[3,4])                             │
  │   prev_pipe = [5,6]                                 │
  └─────────────────────────────────────────────────────┘

Iterazione 3: cmd3
  ┌─────────────────────────────────────────────────────┐
  │ prev_pipe: [5,6]                                    │
  │ curr_pipe: NULL  (è l'ultimo)                       │
  │                                                     │
  │ FIGLIO 3:                                           │
  │   NON primo? SÌ  → stdin  = prev_pipe[5]            │
  │   NON ultimo? NO → stdout = terminal                │
  │   execve(cmd3)                                      │
  │                                                     │
  │ PADRE:                                              │
  │   close(prev_pipe[5,6])                             │
  └─────────────────────────────────────────────────────┘

Aspetta tutti:
  waitpid(pid1)
  waitpid(pid2)
  waitpid(pid3) → return exit_status
```

---

## 10.4 Implementazione Execute Pipeline

```c
int execute_pipeline(t_data *data)
{
    int     *pids;
    int     num_cmds;
    int     prev_pipe[2];
    int     has_prev;
    int     i;
    t_cmd   *current;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Conta comandi e alloca array PIDs
    // ═══════════════════════════════════════════════════════
    num_cmds = count_commands(data->cmd_list);
    pids = malloc(sizeof(int) * num_cmds);
    if (!pids)
        return (ERROR);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Inizializza
    // ═══════════════════════════════════════════════════════
    prev_pipe[0] = -1;
    prev_pipe[1] = -1;
    has_prev = 0;
    i = 0;
    current = data->cmd_list;
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Esegui ogni comando nella pipeline
    // ═══════════════════════════════════════════════════════
    while (current)
    {
        pids[i] = process_pipeline_cmd(data, current, 
                                        prev_pipe, &has_prev);
        
        if (pids[i] == -1)
        {
            free(pids);
            return (ERROR);
        }
        
        current = current->next;
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Aspetta tutti i processi
    // ═══════════════════════════════════════════════════════
    int exit_status = wait_all_processes(pids, num_cmds);
    free(pids);
    
    return (exit_status);
}
```

---

## 10.5 Process Pipeline Command

```c
int process_pipeline_cmd(t_data *data, t_cmd *current,
                         int *prev_pipe, int *has_prev)
{
    t_pipe_data pipe_data;
    int         curr_pipe[2];
    int         pid;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Crea pipe se NON ultimo comando
    // ═══════════════════════════════════════════════════════
    if (current->next)
    {
        if (pipe(curr_pipe) == -1)
        {
            print_error("pipe", "failed");
            return (-1);
        }
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Setup struttura pipe_data
    // ═══════════════════════════════════════════════════════
    setup_pipe_data(&pipe_data, 
                    (*has_prev ? prev_pipe : NULL),
                    curr_pipe,
                    current->next == NULL);
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Esegui comando (fork + exec)
    // ═══════════════════════════════════════════════════════
    pid = exec_pipeline_cmd(data, current, &pipe_data);
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: PADRE - Cleanup pipes
    // ═══════════════════════════════════════════════════════
    if (*has_prev)
        close_pipes(prev_pipe);
    
    // ═══════════════════════════════════════════════════════
    // STEP 5: Salva curr_pipe come prev_pipe per prossima iterazione
    // ═══════════════════════════════════════════════════════
    if (current->next)
        save_pipe(prev_pipe, curr_pipe, has_prev);
    
    return (pid);
}
```

---

## 10.6 Exec Pipeline Command (Processo Figlio)

```c
int exec_pipeline_cmd(t_data *data, t_cmd *cmd, t_pipe_data *pipe_data)
{
    pid_t   pid;
    char    *cmd_path;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Trova path comando
    // ═══════════════════════════════════════════════════════
    cmd_path = find_command_path(cmd->args[0], data->envp);
    if (!cmd_path)
    {
        print_error(cmd->args[0], "command not found");
        return (CMD_NOT_FOUND);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Fork
    // ═══════════════════════════════════════════════════════
    pid = fork();
    if (pid == -1)
    {
        print_error("fork", "failed");
        free(cmd_path);
        return (ERROR);
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: PROCESSO FIGLIO
    // ═══════════════════════════════════════════════════════
    if (pid == 0)
    {
        // ───────────────────────────────────────────────────
        // Setup stdin/stdout per pipeline
        // ───────────────────────────────────────────────────
        setup_child_pipes(pipe_data);
        
        // ───────────────────────────────────────────────────
        // Applica redirections (se presenti)
        // ───────────────────────────────────────────────────
        if (cmd->redirs && apply_redirections(cmd) != SUCCESS)
            exit(ERROR);
        
        // ───────────────────────────────────────────────────
        // Esegui comando
        // ───────────────────────────────────────────────────
        if (execve(cmd_path, cmd->args, data->envp) == -1)
        {
            print_error(cmd->args[0], "execution failed");
            free(cmd_path);
            exit(CMD_NOT_EXECUTABLE);
        }
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: PROCESSO PADRE
    // ═══════════════════════════════════════════════════════
    free(cmd_path);
    return (pid);
}
```

---

## 10.7 Setup Child Pipes

```c
void setup_child_pipes(t_pipe_data *pipe_data)
{
    // ═══════════════════════════════════════════════════════
    // CASO 1: NON primo comando → leggi da prev_pipe
    // ═══════════════════════════════════════════════════════
    if (pipe_data->prev_pipe && pipe_data->prev_pipe[0] != -1)
    {
        // Redirigi stdin sulla pipe precedente
        dup2(pipe_data->prev_pipe[0], STDIN_FILENO);
        
        // Chiudi entrambi i lati della prev_pipe
        close(pipe_data->prev_pipe[0]);
        close(pipe_data->prev_pipe[1]);
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: NON ultimo comando → scrivi su curr_pipe
    // ═══════════════════════════════════════════════════════
    if (!pipe_data->is_last)
    {
        // Chiudi read end (non serve al figlio)
        close(pipe_data->curr_pipe[0]);
        
        // Redirigi stdout sulla pipe corrente
        dup2(pipe_data->curr_pipe[1], STDOUT_FILENO);
        
        // Chiudi write end originale
        close(pipe_data->curr_pipe[1]);
    }
}
```

---

## 10.8 Struttura t_pipe_data

```c
typedef struct s_pipe_data
{
    int *prev_pipe;    // Puntatore a pipe precedente [read, write]
    int *curr_pipe;    // Puntatore a pipe corrente [read, write]
    int is_last;       // 1 se è l'ultimo comando, 0 altrimenti
} t_pipe_data;
```

### Perché questa struttura?

**Problema:** La norminette limita a **4 parametri** per funzione.

**Soluzione:** Raggruppa parametri correlati in una struct.

```c
// ❌ Troppi parametri (5):
exec_pipeline_cmd(data, cmd, prev_pipe, curr_pipe, is_last)

// ✅ Con struct (3 parametri):
exec_pipeline_cmd(data, cmd, &pipe_data)
```

---

## 10.9 Esempio Completo Step-by-Step

### Comando

```bash
echo hello | cat | wc -l
```

### Setup Iniziale

```
num_cmds = 3
pids = [?, ?, ?]
prev_pipe = [-1, -1]
has_prev = 0
```

---

### Iterazione 1: `echo hello`

```
┌─────────────────────────────────────────────────────────────┐
│                    COMANDO 1: echo hello                    │
└─────────────────────────────────────────────────────────────┘

────────────────────────────────────────────────────────────
STEP 1: Crea pipe (curr_pipe)
────────────────────────────────────────────────────────────
current->next esiste (cat) → crea pipe
pipe(curr_pipe) → curr_pipe = [3, 4]

────────────────────────────────────────────────────────────
STEP 2: Setup pipe_data
────────────────────────────────────────────────────────────
pipe_data.prev_pipe = NULL      (has_prev = 0)
pipe_data.curr_pipe = [3, 4]
pipe_data.is_last = 0            (current->next != NULL)

────────────────────────────────────────────────────────────
STEP 3: Fork processo 1
────────────────────────────────────────────────────────────
pid = fork() → pid1 = 1234

FIGLIO (pid == 0):
  setup_child_pipes():
    prev_pipe == NULL → stdin = terminal (non modificato)
    is_last == 0      → stdout = curr_pipe[4]
      dup2(4, STDOUT_FILENO)
      close(3)
      close(4)
  
  execve("/bin/echo", ["echo", "hello"], envp)

PADRE (pid > 0):
  pids[0] = 1234
  has_prev = 0 → non chiude prev_pipe
  
  Salva curr_pipe come prev_pipe:
    prev_pipe[0] = 3
    prev_pipe[1] = 4
    has_prev = 1

FD del PADRE dopo fork:
  FD 3: pipe read end
  FD 4: pipe write end
```

---

### Iterazione 2: `cat`

```
┌─────────────────────────────────────────────────────────────┐
│                      COMANDO 2: cat                         │
└─────────────────────────────────────────────────────────────┘

────────────────────────────────────────────────────────────
STEP 1: Crea pipe (curr_pipe)
────────────────────────────────────────────────────────────
current->next esiste (wc) → crea pipe
pipe(curr_pipe) → curr_pipe = [5, 6]

────────────────────────────────────────────────────────────
STEP 2: Setup pipe_data
────────────────────────────────────────────────────────────
pipe_data.prev_pipe = [3, 4]     (has_prev = 1)
pipe_data.curr_pipe = [5, 6]
pipe_data.is_last = 0            (current->next != NULL)

────────────────────────────────────────────────────────────
STEP 3: Fork processo 2
────────────────────────────────────────────────────────────
pid = fork() → pid2 = 1235

FIGLIO (pid == 0):
  setup_child_pipes():
    prev_pipe != NULL → stdin = prev_pipe[3]
      dup2(3, STDIN_FILENO)
      close(3)
      close(4)
    
    is_last == 0 → stdout = curr_pipe[6]
      close(5)
      dup2(6, STDOUT_FILENO)
      close(6)
  
  execve("/bin/cat", ["cat"], envp)

PADRE (pid > 0):
  pids[1] = 1235
  has_prev = 1 → chiude prev_pipe:
    close(3)
    close(4)
  
  Salva curr_pipe come prev_pipe:
    prev_pipe[0] = 5
    prev_pipe[1] = 6

FD del PADRE dopo fork:
  FD 3, 4: chiusi
  FD 5: pipe read end
  FD 6: pipe write end
```

---

### Iterazione 3: `wc -l`

```
┌─────────────────────────────────────────────────────────────┐
│                    COMANDO 3: wc -l                         │
└─────────────────────────────────────────────────────────────┘

────────────────────────────────────────────────────────────
STEP 1: NON crea pipe
────────────────────────────────────────────────────────────
current->next == NULL (è l'ultimo) → NON crea pipe

────────────────────────────────────────────────────────────
STEP 2: Setup pipe_data
────────────────────────────────────────────────────────────
pipe_data.prev_pipe = [5, 6]     (has_prev = 1)
pipe_data.curr_pipe = NULL
pipe_data.is_last = 1            (current->next == NULL)

────────────────────────────────────────────────────────────
STEP 3: Fork processo 3
────────────────────────────────────────────────────────────
pid = fork() → pid3 = 1236

FIGLIO (pid == 0):
  setup_child_pipes():
    prev_pipe != NULL → stdin = prev_pipe[5]
      dup2(5, STDIN_FILENO)
      close(5)
      close(6)
    
    is_last == 1 → stdout = terminal (non modificato)
  
  execve("/usr/bin/wc", ["wc", "-l"], envp)

PADRE (pid > 0):
  pids[2] = 1236
  has_prev = 1 → chiude prev_pipe:
    close(5)
    close(6)

FD del PADRE dopo fork:
  Tutti i FD delle pipe chiusi ✓
```

---

### Wait All Processes

```
┌─────────────────────────────────────────────────────────────┐
│                    WAIT ALL PROCESSES                       │
└─────────────────────────────────────────────────────────────┘

waitpid(1234, &status, 0)  // echo hello
  → exit_status = 0

waitpid(1235, &status, 0)  // cat
  → exit_status = 0

waitpid(1236, &status, 0)  // wc -l
  → exit_status = 0
  → QUESTO è il return value finale

return 0
```

---

### Output Finale

```
1
```

(Perché "hello" è una sola linea)

---

# 11. MODULO 8: SIGNALS

## 11.1 Segnali da Gestire

In minishell, dobbiamo gestire correttamente i seguenti segnali:

### Tabella Segnali

| Segnale | Combinazione | Numero | Comportamento Shell | Comportamento Comando |
|---------|--------------|--------|---------------------|----------------------|
| `SIGINT` | `Ctrl-C` | 2 | Nuovo prompt (non esce) | Interrompe comando |
| `SIGQUIT` | `Ctrl-\` | 3 | Ignorato | Ignorato |
| `EOF` | `Ctrl-D` | - | Esce dalla shell | - |

---

## 11.2 Comportamenti Richiesti

### Scenario 1: Shell in Attesa di Input

```bash
minishell$ ^C
minishell$ 
```

**Comportamento:**
- `Ctrl-C` → Mostra nuovo prompt (NON esce)
- Input corrente cancellato

---

### Scenario 2: Comando in Esecuzione

```bash
minishell$ cat
^C
minishell$ 
```

**Comportamento:**
- `Ctrl-C` → Interrompe `cat` (processo figlio)
- Shell rimane attiva e mostra nuovo prompt

---

### Scenario 3: Ctrl-D (EOF)

```bash
minishell$ ^D
exit
$
```

**Comportamento:**
- `Ctrl-D` → Esce dalla shell (come `exit`)
- Stampa "exit" prima di uscire

---

## 11.3 Setup Signals

### Implementazione

```c
void setup_signals(void)
{
    struct sigaction sa;
    
    // ═══════════════════════════════════════════════════════
    // SIGINT (Ctrl-C) → Custom handler
    // ═══════════════════════════════════════════════════════
    sa.sa_handler = handle_sigint;
    sa.sa_flags = SA_RESTART;         // Riavvia syscall interrotte
    sigemptyset(&sa.sa_mask);         // Nessun segnale bloccato
    sigaction(SIGINT, &sa, NULL);
    
    // ═══════════════════════════════════════════════════════
    // SIGQUIT (Ctrl-\) → Ignora
    // ═══════════════════════════════════════════════════════
    signal(SIGQUIT, SIG_IGN);
}
```

### Struttura sigaction

```c
struct sigaction {
    void (*sa_handler)(int);   // Funzione handler
    sigset_t sa_mask;          // Segnali da bloccare durante handler
    int sa_flags;              // Flag comportamentali
};
```

### Tabella Flags

| Flag | Descrizione |
|------|-------------|
| `SA_RESTART` | Riavvia automaticamente syscall interrotte dal segnale |
| `SA_NODEFER` | Permette la ricezione dello stesso segnale durante handler |
| `SA_RESETHAND` | Resetta handler a default dopo esecuzione |

---

## 11.4 Handle SIGINT (Ctrl-C)

### Variabile Globale

```c
volatile sig_atomic_t g_signal = 0;
```

**Perché `volatile sig_atomic_t`?**
- `volatile`: Il compilatore non ottimizza accessi (può cambiare da signal handler)
- `sig_atomic_t`: Tipo atomico garantito safe in signal handler

---

### Implementazione Handler

```c
void handle_sigint(int sig)
{
    (void)sig;  // Parametro non usato
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Salva segnale ricevuto
    // ═══════════════════════════════════════════════════════
    g_signal = SIGINT;
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Stampa newline
    // ═══════════════════════════════════════════════════════
    write(STDOUT_FILENO, "\n", 1);
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Gestione readline
    // ═══════════════════════════════════════════════════════
    rl_on_new_line();        // Notifica readline: nuova linea
    rl_replace_line("", 0);  // Cancella input corrente
    rl_redisplay();          // Mostra nuovo prompt
}
```

### Funzioni Readline

| Funzione | Descrizione |
|----------|-------------|
| `rl_on_new_line()` | Informa readline che si è su nuova linea |
| `rl_replace_line(str, clear)` | Sostituisce contenuto linea corrente |
| `rl_redisplay()` | Ridisegna prompt e linea |

---

### Diagramma Comportamento

```
┌─────────────────────────────────────────────────────────────┐
│                    SIGINT FLOW                              │
└─────────────────────────────────────────────────────────────┘

USER                    SHELL               KERNEL
  │                       │                    │
  ├─ digita "ls -l"       │                    │
  │                       │                    │
  ├─ preme Ctrl-C ────────┼───> SIGINT ────────>│
  │                       │                    │
  │                       │<─── chiama handler─┤
  │                       │                    │
  │                       ├─ write("\n")       │
  │                       ├─ rl_on_new_line()  │
  │                       ├─ rl_replace_line("",0)
  │                       ├─ rl_redisplay()    │
  │                       │                    │
  │<──── "minishell$ " ───┤                    │
  │                       │                    │
```

---

## 11.5 Ctrl-D (EOF)

`Ctrl-D` **NON** è un segnale, ma un carattere speciale che indica **End Of File**.

### Gestione nel Main Loop

```c
while (1)
{
    input = readline("minishell$ ");
    
    // ═══════════════════════════════════════════════════════
    // Ctrl-D → readline ritorna NULL
    // ═══════════════════════════════════════════════════════
    if (!input)
    {
        ft_putendl_fd("exit", STDOUT_FILENO);
        break;  // Esce dal loop → termina shell
    }
    
    // ... resto del codice
}
```

### Comportamento readline con EOF

```
┌─────────────────────────────────────────────────────────────┐
│                    CTRL-D BEHAVIOR                          │
└─────────────────────────────────────────────────────────────┘

Situazione 1: Linea VUOTA + Ctrl-D
  minishell$ ^D
  → readline() ritorna NULL
  → shell esce

Situazione 2: Linea CON TESTO + Ctrl-D
  minishell$ hello^D
  → readline() ritorna "hello"
  → shell processa "hello"
  → NON esce
```

---

## 11.6 Signals Durante Esecuzione Comando

### Comportamento Desiderato

```bash
minishell$ cat
^C
minishell$ 
```

**Cosa deve succedere:**
1. `cat` viene **interrotto** (riceve SIGINT)
2. Shell **NON** viene interrotta
3. Shell mostra **nuovo prompt**

---

### Come Funziona

```
┌─────────────────────────────────────────────────────────────┐
│              SIGNALS IN CHILD PROCESS                       │
└─────────────────────────────────────────────────────────────┘

SHELL (padre)                    CHILD (figlio - cat)
├─ fork() ─────────────────────> ├─ eredita signal handlers
│                                 ├─ execve("/bin/cat")
│                                 │  (sostituisce handlers con default)
│                                 │
│ USER preme Ctrl-C               │
├─ riceve SIGINT                  ├─ riceve SIGINT
│  (ma NON nella shell,           │  (handler default → termina)
│   nel terminale)                │
│                                 ├─ exit(128 + 2 = 130)
│                                 │
├─ waitpid(pid, &status, 0)       │
│  rileva terminazione da segnale │
│  WIFSIGNALED(status) == true    │
│  WTERMSIG(status) == 2 (SIGINT) │
│                                 │
├─ return 130 (128 + 2)           │
```

---

### Implementazione nel Waitpid

```c
int exec_external_cmd(t_data *data, t_cmd *cmd)
{
    pid_t pid;
    int   status;
    
    // ... fork e execve ...
    
    // ═══════════════════════════════════════════════════════
    // Aspetta processo figlio
    // ═══════════════════════════════════════════════════════
    waitpid(pid, &status, 0);
    
    // ═══════════════════════════════════════════════════════
    // Analizza come è terminato
    // ═══════════════════════════════════════════════════════
    
    // Caso 1: Terminato normalmente (exit)
    if (WIFEXITED(status))
        return (WEXITSTATUS(status));
    
    // Caso 2: Terminato da segnale (Ctrl-C, kill, ecc.)
    if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        return (128 + sig);  // Convenzione Bash
    }
    
    return (SUCCESS);
}
```

### Tabella Exit Codes da Segnali

| Segnale | Numero | Exit Code | Descrizione |
|---------|--------|-----------|-------------|
| `SIGINT` | 2 | 130 (128+2) | Ctrl-C |
| `SIGQUIT` | 3 | 131 (128+3) | Ctrl-\ |
| `SIGTERM` | 15 | 143 (128+15) | kill default |
| `SIGKILL` | 9 | 137 (128+9) | kill -9 |

---

## 11.7 Macros per Analisi Exit Status

### Tabella Macros

| Macro | Descrizione | Uso |
|-------|-------------|-----|
| `WIFEXITED(status)` | True se terminato con `exit()` | Check terminazione normale |
| `WEXITSTATUS(status)` | Estrae exit code | Ottieni valore passato a `exit()` |
| `WIFSIGNALED(status)` | True se terminato da segnale | Check terminazione anomala |
| `WTERMSIG(status)` | Estrae numero segnale | Ottieni quale segnale ha terminato |

---

### Esempio d'Uso

```c
int status;
waitpid(pid, &status, 0);

if (WIFEXITED(status))
{
    int exit_code = WEXITSTATUS(status);
    printf("Processo terminato con exit(%d)\n", exit_code);
}
else if (WIFSIGNALED(status))
{
    int sig = WTERMSIG(status);
    printf("Processo terminato da segnale %d\n", sig);
    
    if (sig == SIGINT)
        printf("Utente ha premuto Ctrl-C\n");
}
```

---

## 11.8 Perché Non Usiamo signal()?

### Differenze signal() vs sigaction()

| Aspetto | `signal()` | `sigaction()` |
|---------|-----------|---------------|
| **Portabilità** | Comportamento varia tra OS | Comportamento standardizzato POSIX |
| **Flags** | Nessun controllo | Flags dettagliati (SA_RESTART, ecc.) |
| **Signal mask** | Non gestibile | Gestibile (blocca segnali durante handler) |
| **Affidabilità** | Handler può resetarsi | Handler persiste |

### Esempio Problema con signal()

```c
// ❌ PROBLEMA con signal()
void handler(int sig)
{
    signal(SIGINT, handler);  // Deve re-registrarsi!
    // C'è una race condition qui!
    write(1, "SIGINT\n", 7);
}

signal(SIGINT, handler);
```

**Race condition:** Tra la ricezione del segnale e la ri-registrazione, un secondo SIGINT potrebbe usare il default handler (terminazione).

---

### Soluzione con sigaction()

```c
// ✅ CORRETTO con sigaction()
void handler(int sig)
{
    // Handler rimane registrato automaticamente!
    write(1, "SIGINT\n", 7);
}

struct sigaction sa;
sa.sa_handler = handler;
sa.sa_flags = SA_RESTART;
sigemptyset(&sa.sa_mask);
sigaction(SIGINT, &sa, NULL);
```

---

## 11.9 Signal-Safe Functions

### Regola Critica

**SOLO** le funzioni **async-signal-safe** possono essere chiamate dentro un signal handler!

### Funzioni SAFE

| Funzione | Descrizione |
|----------|-------------|
| `write()` | ✅ SAFE |
| `_exit()` | ✅ SAFE |
| `signal()` | ✅ SAFE |
| `sigaction()` | ✅ SAFE |

### Funzioni UNSAFE

| Funzione | Descrizione |
|----------|-------------|
| `printf()` | ❌ UNSAFE (usa malloc internamente) |
| `malloc()` | ❌ UNSAFE (non rientrante) |
| `free()` | ❌ UNSAFE (non rientrante) |
| Funzioni readline | ⚠️ UNSAFE (ma usate con attenzione) |

---

### Perché `printf()` è Unsafe?

```c
void handler(int sig)
{
    printf("Signal!\n");  // ❌ PERICOLOSO!
}
```

**Problema:**
1. Main sta eseguendo `printf("Hello")`
2. `printf()` ha acquisito lock interno
3. Arriva SIGINT → chiama handler
4. Handler chiama `printf()` → tenta di acquisire STESSO lock
5. **DEADLOCK!** 💀

---

### Versione Safe

```c
void handler(int sig)
{
    write(STDOUT_FILENO, "Signal!\n", 8);  // ✅ SAFE
}
```

---

## 11.10 Funzioni Readline in Signal Handler

### Eccezione Speciale

Minishell usa funzioni readline (`rl_on_new_line`, `rl_redisplay`) nel signal handler, che **tecnicamente non sono signal-safe**.

**Perché funziona?**
- Readline è progettato per gestire questo caso
- Le funzioni usate sono "abbastanza safe" nel contesto specifico
- È prassi comune nelle shell interattive

---

### Best Practice

```c
void handle_sigint(int sig)
{
    (void)sig;
    
    g_signal = SIGINT;  // ✅ SAFE (variabile globale)
    
    write(STDOUT_FILENO, "\n", 1);  // ✅ SAFE
    
    // ⚠️ Tecnicamente unsafe, ma accettato per shell interattive
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

---

## 11.11 Esempio Completo con Test

### Test Interattivo

```bash
# Test 1: Ctrl-C nel prompt
minishell$ ^C
minishell$ 
✅ PASS: Nuovo prompt, non esce

# Test 2: Ctrl-C durante comando
minishell$ sleep 10
^C
minishell$ echo $?
130
✅ PASS: Comando interrotto, exit code = 130 (128+2)

# Test 3: Ctrl-\ (deve essere ignorato)
minishell$ ^\ 
(nessun output)
✅ PASS: Ignorato

# Test 4: Ctrl-D
minishell$ ^D
exit
$
✅ PASS: Esce dalla shell

# Test 5: Ctrl-C con input parziale
minishell$ echo hello wor^C
minishell$ 
✅ PASS: Input cancellato, nuovo prompt
```

---

# 12. MODULO 9: ENVIRONMENT

## 12.1 Cos'è l'Environment?

L'**environment** è un array di stringhe passato a ogni processo, contenente variabili nel formato `"KEY=value"`.

### Esempio

```c
char **envp = [
    "PATH=/usr/bin:/bin",
    "HOME=/Users/giusmery",
    "USER=giusmery",
    "PWD=/Users/giusmery/minishell",
    "OLDPWD=/Users/giusmery",
    "SHELL=/bin/bash",
    "_=/usr/bin/env",
    NULL  // ← Sempre terminato da NULL
];
```

---

## 12.2 Perché Copiare l'Environment?

### Problema

```c
int main(int argc, char **argv, char **envp)
{
    // envp è READ-ONLY!
    // Non possiamo modificare direttamente envp
}
```

### Soluzione

```c
typedef struct s_data
{
    char **envp;  // ← Copia MODIFICABILE
    // ...
} t_data;

void init_data(t_data *data, char **envp)
{
    data->envp = copy_envp(envp);  // Copia profonda
}
```

---

## 12.3 Copy Environment

### Implementazione

```c
char **copy_envp(char **envp)
{
    char **copy;
    int  i;
    int  count;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Conta variabili
    // ═══════════════════════════════════════════════════════
    count = 0;
    while (envp[count])
        count++;
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Alloca array (+1 per NULL)
    // ═══════════════════════════════════════════════════════
    copy = malloc(sizeof(char *) * (count + 1));
    if (!copy)
        return (NULL);
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Copia ogni stringa
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (i < count)
    {
        copy[i] = ft_strdup(envp[i]);
        if (!copy[i])
        {
            free_array(copy);  // Cleanup parziale
            return (NULL);
        }
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Termina con NULL
    // ═══════════════════════════════════════════════════════
    copy[i] = NULL;
    
    return (copy);
}
```

---

## 12.4 Get Environment Value

### Implementazione

```c
char *get_env_value(char *key, char **envp)
{
    int i;
    int key_len;
    
    if (!key || !envp)
        return (NULL);
    
    key_len = ft_strlen(key);
    
    // ═══════════════════════════════════════════════════════
    // Cerca key nell'environment
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (envp[i])
    {
        // Confronta: "KEY" con inizio di "KEY=value"
        if (ft_strncmp(envp[i], key, key_len) == 0 &&
            envp[i][key_len] == '=')
        {
            // Ritorna valore (dopo '=')
            return (ft_strdup(envp[i] + key_len + 1));
        }
        i++;
    }
    
    // Non trovato
    return (NULL);
}
```

---

### Esempio d'Uso

```c
char *home = get_env_value("HOME", data->envp);
// home = "/Users/giusmery"

char *user = get_env_value("USER", data->envp);
// user = "giusmery"

char *notexist = get_env_value("NOTEXIST", data->envp);
// notexist = NULL

// IMPORTANTE: Ricordarsi di liberare!
free(home);
free(user);
```

---

## 12.5 Set Environment Value

### Algoritmo

```
1. Cerca se KEY esiste già
   → SE esiste:
       - Sostituisci valore
       - Return
   
2. SE NON esiste:
   → Aggiungi nuova variabile:
       - Alloca nuovo array (size + 1)
       - Copia vecchie variabili
       - Aggiungi nuova
       - Libera vecchio array
       - Return
```

---

### Implementazione

```c
int set_env_value(char *key, char *value, t_data *data)
{
    char *new_entry;
    char *temp;
    int  i;
    int  key_len;
    
    if (!key || !value)
        return (ERROR);
    
    key_len = ft_strlen(key);
    
    // ═══════════════════════════════════════════════════════
    // CASO 1: Variabile esiste → Sostituisci
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (data->envp[i])
    {
        if (ft_strncmp(data->envp[i], key, key_len) == 0 &&
            data->envp[i][key_len] == '=')
        {
            // Costruisci nuova entry "KEY=value"
            temp = ft_strjoin(key, "=");
            new_entry = ft_strjoin(temp, value);
            free(temp);
            
            // Sostituisci
            free(data->envp[i]);
            data->envp[i] = new_entry;
            
            return (SUCCESS);
        }
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // CASO 2: Variabile NON esiste → Aggiungi
    // ═══════════════════════════════════════════════════════
    return (add_new_env(data, key, value, i));
}
```

---

### Add New Env

```c
int add_new_env(t_data *data, char *key, char *value, int count)
{
    char **new_envp;
    char *temp;
    char *new_entry;
    int  i;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Alloca nuovo array (count + nuovo + NULL)
    // ═══════════════════════════════════════════════════════
    new_envp = malloc(sizeof(char *) * (count + 2));
    if (!new_envp)
        return (ERROR);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Copia vecchie variabili (PUNTATORI, non stringhe!)
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (i < count)
    {
        new_envp[i] = data->envp[i];  // ← Copia puntatore
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Aggiungi nuova variabile
    // ═══════════════════════════════════════════════════════
    temp = ft_strjoin(key, "=");
    new_entry = ft_strjoin(temp, value);
    free(temp);
    
    new_envp[i] = new_entry;
    new_envp[i + 1] = NULL;
    
    // ═══════════════════════════════════════════════════════
    // STEP 4: Libera SOLO vecchio array (non le stringhe!)
    // ═══════════════════════════════════════════════════════
    free(data->envp);
    data->envp = new_envp;
    
    return (SUCCESS);
}
```

---

### Diagramma Add New Env

```
┌─────────────────────────────────────────────────────────────┐
│                    ADD NEW VARIABLE                         │
└─────────────────────────────────────────────────────────────┘

BEFORE: envp con 3 variabili
┌─────────────────────────────────────────┐
│ envp[0] ───> "PATH=/bin"               │
│ envp[1] ───> "HOME=/Users/giusmery"    │
│ envp[2] ───> "USER=giusmery"           │
│ envp[3] = NULL                         │
└─────────────────────────────────────────┘

Aggiungiamo: TEST=hello

STEP 1: Alloca new_envp (4 variabili + NULL = 5)
┌──────────────────────────────────────────┐
│ new_envp[0] = ?                         │
│ new_envp[1] = ?                         │
│ new_envp[2] = ?                         │
│ new_envp[3] = ?                         │
│ new_envp[4] = ?                         │
└──────────────────────────────────────────┘

STEP 2: Copia puntatori
┌──────────────────────────────────────────┐
│ new_envp[0] ───> "PATH=/bin"  (stesso!)│
│ new_envp[1] ───> "HOME=/Users/giusmery"│
│ new_envp[2] ───> "USER=giusmery"       │
│ new_envp[3] = ?                         │
│ new_envp[4] = ?                         │
└──────────────────────────────────────────┘

STEP 3: Aggiungi nuova
┌──────────────────────────────────────────┐
│ new_envp[0] ───> "PATH=/bin"            │
│ new_envp[1] ───> "HOME=/Users/giusmery" │
│ new_envp[2] ───> "USER=giusmery"        │
│ new_envp[3] ───> "TEST=hello" (NUOVO!) │
│ new_envp[4] = NULL                      │
└──────────────────────────────────────────┘

STEP 4: Libera vecchio array
free(envp)  ← Solo l'array, NON le stringhe!

AFTER: data->envp = new_envp
┌──────────────────────────────────────────┐
│ data->envp[0] ───> "PATH=/bin"          │
│ data->envp[1] ───> "HOME=/Users/giusmery"│
│ data->envp[2] ───> "USER=giusmery"      │
│ data->envp[3] ───> "TEST=hello"         │
│ data->envp[4] = NULL                    │
└──────────────────────────────────────────┘
```

---

## 12.6 Unset Environment Value

### Algoritmo

```
1. Conta variabili che NON matchano key
2. Alloca nuovo array (count - 1)
3. Copia tutte TRANNE quella da rimuovere
4. Libera stringa rimossa
5. Libera vecchio array
6. Aggiorna data->envp
```

---

### Implementazione

```c
int unset_env_value(char *key, t_data *data)
{
    char **new_envp;
    int  key_len;
    
    if (!key)
        return (ERROR);
    
    key_len = ft_strlen(key);
    
    // ═══════════════════════════════════════════════════════
    // Copia environment escludendo key
    // ═══════════════════════════════════════════════════════
    new_envp = copy_env_without_key(data->envp, key, key_len);
    if (!new_envp)
        return (ERROR);
    
    free(data->envp);  // Libera vecchio array
    data->envp = new_envp;
    
    return (SUCCESS);
}
```

---

### Copy Env Without Key

```c
char **copy_env_without_key(char **envp, char *key, int key_len)
{
    char **new_envp;
    int  i;
    int  j;
    int  count;
    
    // ═══════════════════════════════════════════════════════
    // STEP 1: Conta variabili da mantenere
    // ═══════════════════════════════════════════════════════
    count = count_env_without_key(envp, key, key_len);
    
    // ═══════════════════════════════════════════════════════
    // STEP 2: Alloca nuovo array
    // ═══════════════════════════════════════════════════════
    new_envp = malloc(sizeof(char *) * (count + 1));
    if (!new_envp)
        return (NULL);
    
    // ═══════════════════════════════════════════════════════
    // STEP 3: Copia variabili (TRANNE key)
    // ═══════════════════════════════════════════════════════
    i = 0;
    j = 0;
    while (envp[i])
    {
        // Se NON matcha key → copia puntatore
        if (!(ft_strncmp(envp[i], key, key_len) == 0 &&
              envp[i][key_len] == '='))
        {
            new_envp[j] = envp[i];  // ← Copia puntatore
            j++;
        }
        else
        {
            // Matcha key → libera stringa
            free(envp[i]);
        }
        i++;
    }
    
    new_envp[j] = NULL;
    
    return (new_envp);
}
```

---

### Count Env Without Key Helper

```c
int count_env_without_key(char **envp, char *key, int key_len)
{
    int count;
    int i;
    
    count = 0;
    i = 0;
    
    while (envp[i])
    {
        // Conta se NON matcha key
        if (!(ft_strncmp(envp[i], key, key_len) == 0 &&
              envp[i][key_len] == '='))
        {
            count++;
        }
        i++;
    }
    
    return (count);
}
```

---

## 12.7 Free Environment

```c
void free_envp(char **envp)
{
    free_array(envp);
}

void free_array(char **arr)
{
    int i;
    
    if (!arr)
        return;
    
    // Libera ogni stringa
    i = 0;
    while (arr[i])
    {
        free(arr[i]);
        i++;
    }
    
    // Libera array
    free(arr);
}
```

---

# 13. GESTIONE MEMORIA

## 13.1 Regole Fondamentali

### Golden Rules

1. **Ogni `malloc()` ha un `free()`**
2. **Free in ordine inverso** all'allocazione
3. **NULL-check prima di usare puntatori**
4. **NULL-check prima di free** (best practice)
5. **Set to NULL dopo free** (prevenzione use-after-free)

---

## 13.2 Free Tokens

### Struttura Token List

```
┌─────────┐    ┌─────────┐    ┌─────────┐
│ token1  │ -> │ token2  │ -> │ token3  │ -> NULL
├─────────┤    ├─────────┤    ├─────────┤
│ value ──┼──> │ value ──┼──> │ value ──┼──> (stringhe)
└─────────┘    └─────────┘    └─────────┘
```

### Implementazione

```c
void free_tokens(t_token *tokens)
{
    t_token *tmp;
    
    while (tokens)
    {
        tmp = tokens;
        tokens = tokens->next;
        
        // ═══════════════════════════════════════════════════
        // Libera valore (stringa)
        // ═══════════════════════════════════════════════════
        if (tmp->value)
            free(tmp->value);
        
        // ═══════════════════════════════════════════════════
        // Libera struttura
        // ═══════════════════════════════════════════════════
        free(tmp);
    }
}
```

---

## 13.3 Free Command List

### Struttura Complessa

```
CMD 1                    CMD 2
┌────────────┐          ┌────────────┐
│ args ──────┼─> array  │ args ──────┼─> array
│ redirs ────┼─> list   │ redirs ────┼─> list
│ next ──────┼─────────>│ next = NULL│
└────────────┘          └────────────┘
```

### Implementazione

```c
void free_cmd_list(t_cmd *cmd_list)
{
    t_cmd *tmp_cmd;
    
    while (cmd_list)
    {
        tmp_cmd = cmd_list;
        cmd_list = cmd_list->next;
        
        // ═══════════════════════════════════════════════════
        // 1. Libera array argomenti
        // ═══════════════════════════════════════════════════
        if (tmp_cmd->args)
            free_array(tmp_cmd->args);
        
        // ═══════════════════════════════════════════════════
        // 2. Libera lista redirections
        // ═══════════════════════════════════════════════════
        if (tmp_cmd->redirs)
            free_redirs(tmp_cmd->redirs);
        
        // ═══════════════════════════════════════════════════
        // 3. Libera struttura comando
        // ═══════════════════════════════════════════════════
        free(tmp_cmd);
    }
}
```

---

## 13.4 Free Array

```c
void free_array(char **arr)
{
    int i;
    
    if (!arr)
        return;
    
    // ═══════════════════════════════════════════════════════
    // Libera ogni stringa
    // ═══════════════════════════════════════════════════════
    i = 0;
    while (arr[i])
    {
        free(arr[i]);
        i++;
    }
    
    // ═══════════════════════════════════════════════════════
    // Libera array di puntatori
    // ═══════════════════════════════════════════════════════
    free(arr);
}
```

---

## 13.5 Free Redirections

```c
void free_redirs(t_redir *redirs)
{
    t_redir *tmp;
    
    while (redirs)
    {
        tmp = redirs;
        redirs = redirs->next;
        
        // Libera filename/delimiter
        if (tmp->file)
            free(tmp->file);
        
        // Libera struttura
        free(tmp);
    }
}
```

---

## 13.6 Cleanup Data

```c
void cleanup_data(t_data *data)
{
    // ═══════════════════════════════════════════════════════
    // 1. Libera environment
    // ═══════════════════════════════════════════════════════
    if (data->envp)
        free_array(data->envp);
    
    // ═══════════════════════════════════════════════════════
    // 2. Libera command list
    // ═══════════════════════════════════════════════════════
    if (data->cmd_list)
        free_cmd_list(data->cmd_list);
    
    // ═══════════════════════════════════════════════════════
    // 3. Restore stdin/stdout
    // ═══════════════════════════════════════════════════════
    if (data->stdin_backup != -1)
    {
        dup2(data->stdin_backup, STDIN_FILENO);
        close(data->stdin_backup);
    }
    
    if (data->stdout_backup != -1)
    {
        dup2(data->stdout_backup, STDOUT_FILENO);
        close(data->stdout_backup);
    }
}
```

---

# 14. ERROR HANDLING

## 14.1 Exit Codes

### Definizioni

```c
#define SUCCESS              0    // Tutto OK
#define ERROR                1    // Errore generico
#define CMD_NOT_FOUND        127  // Comando non trovato
#define CMD_NOT_EXECUTABLE   126  // Comando non eseguibile
```

### Tabella Completa

| Code | Nome | Descrizione | Esempio |
|------|------|-------------|---------|
| `0` | `SUCCESS` | Comando eseguito con successo | `echo hello` |
| `1` | `ERROR` | Errore generico | `cd /nonexistent` |
| `2` | - | Uso scorretto builtin | `exit abc` |
| `126` | `CMD_NOT_EXECUTABLE` | File non eseguibile | `/etc/hosts` |
| `127` | `CMD_NOT_FOUND` | Comando non trovato | `notacommand` |
| `128+N` | - | Terminato da segnale N | `130` = Ctrl-C |

---

## 14.2 Print Error

```c
void print_error(char *cmd, char *msg)
{
    ft_putstr_fd("minishell: ", STDERR_FILENO);
    
    if (cmd)
    {
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(": ", STDERR_FILENO);
    }
    
    ft_putendl_fd(msg, STDERR_FILENO);
}
```

### Esempi Output

```c
print_error("cd", "No such file or directory");
// minishell: cd: No such file or directory

print_error("fork", "failed");
// minishell: fork: failed

print_error(NULL, "syntax error");
// minishell: syntax error
```

---

## 14.3 Gestione Errori Critici

### Malloc Failed

```c
ptr = malloc(size);
if (!ptr)
{
    print_error("malloc", "allocation failed");
    cleanup_data(data);
    exit(ERROR);
}
```

### Fork Failed

```c
pid = fork();
if (pid == -1)
{
    print_error("fork", "failed");
    return (ERROR);  // NON esce, ritorna errore
}
```

### Pipe Failed

```c
if (pipe(pipe_fd) == -1)
{
    print_error("pipe", "failed");
    return (ERROR);
}
```

---

# 15. BEST PRACTICES

## 15.1 Coding Standards

### 1. NULL-Check Sempre

```c
// ❌ NO:
char *str = malloc(100);
str[0] = 'a';

// ✅ SÌ:
char *str = malloc(100);
if (!str)
    return (NULL);
str[0] = 'a';
```

---

### 2. Chiudi File Descriptors

```c
// ❌ NO:
int fd = open("file.txt", O_RDONLY);
dup2(fd, STDIN_FILENO);

// ✅ SÌ:
int fd = open("file.txt", O_RDONLY);
dup2(fd, STDIN_FILENO);
close(fd);  // SEMPRE chiudi!
```

---

### 3. Free in Ordine Inverso

```c
// Allocazione:
char *a = malloc(10);
char *b = malloc(20);

// ✅ Free inverso:
free(b);
free(a);
```

---

### 4. Valida Input Utente

```c
// Prima di usare args[1]:
if (!args[1])
{
    print_error("cd", "missing argument");
    return (ERROR);
}

char *path = args[1];
```

---

### 5. Prototipi Statici

```c
// ✅ Helper functions private:
static int helper_function(int x);

// ❌ Non mettere helper nel .h:
// Solo funzioni pubbliche API nel header
```

---

## 15.2 Testing Checklist

```bash
# ════════════════════════════════════════════════════════════
# COMANDI BASE
# ════════════════════════════════════════════════════════════
echo hello
ls
pwd
cd /tmp
pwd

# ════════════════════════════════════════════════════════════
# QUOTES
# ════════════════════════════════════════════════════════════
echo 'hello $USER'      # → hello $USER
echo "hello $USER"      # → hello giusmery

# ════════════════════════════════════════════════════════════
# PIPES
# ════════════════════════════════════════════════════════════
echo hello | cat
ls | grep test | wc -l

# ════════════════════════════════════════════════════════════
# REDIRECTIONS
# ════════════════════════════════════════════════════════════
echo hello > out.txt
cat < in.txt
cat << EOF
test
EOF

# ════════════════════════════════════════════════════════════
# BUILTINS
# ════════════════════════════════════════════════════════════
export TEST=hello
echo $TEST
unset TEST
env
exit

# ════════════════════════════════════════════════════════════
# EDGE CASES
# ════════════════════════════════════════════════════════════
echo
echo ""
echo    hello     world
$NOTEXIST
```

---

# 14. DOMANDE FREQUENTI VALUTAZIONE

## 14.1 Generali sul Progetto

### Q1: "Quante variabili globali hai?"

**Risposta:** 

"Una sola: `g_signal` di tipo `volatile sig_atomic_t`. È necessaria perché i signal handler devono poter comunicare con il main loop della shell. Il subject permette esplicitamente una variabile globale per questo motivo. È volatile perché può essere modificata dal signal handler in modo asincrono, e sig_atomic_t garantisce che le operazioni su di essa siano atomiche."

---

### Q2: "Perché hai diviso il progetto in così tanti file?"

**Risposta:**

"Ho seguito la norminette che impone massimo 5 funzioni per file e massimo 25 linee per funzione. Ho organizzato i file per moduli logici:
- **Lexer**: 3 file (main, utils, handlers)
- **Parser**: 3 file (main, utils, handlers)
- **Executor**: gestione esecuzione comandi
- **Pipes**: 3 file per gestione pipeline
- **Redirections**: 2 file per I/O redirection
- **Builtins**: 1 file per builtin (echo, cd, pwd, ecc.)
- **Environment**: 2 file per gestione variabili

Questa organizzazione rende il codice modulare e manutenibile."

---

## 14.2 Lexer e Parser

### Q3: "Spiega come funziona il tuo lexer"

**Risposta:**

"Il lexer trasforma una stringa in una lista di token. Funziona così:

1. **Scansiona** carattere per carattere
2. **Salta** spazi e tab
3. **Riconosce** operatori: `|`, `<`, `>`, `>>`, `<<`
4. **Gestisce quotes**:
   - Single quotes `'...'`: tutto letterale, nessuna espansione
   - Double quotes `"..."`: espande variabili ma preserva spazi
5. **Estrae parole**: fino a spazio o operatore
6. **Espande variabili**: nelle double quotes e fuori quotes

Output: lista collegata di token (WORD, PIPE, REDIR_IN, ecc.)

Esempio: `echo "hello $USER" | cat` diventa:
- [WORD:"echo"]
- [WORD:"hello giusmery"] ← espanso
- [PIPE:"|"]
- [WORD:"cat"]"

---

### Q4: "Come distingui tra comandi e argomenti nel parser?"

**Risposta:**

"Il parser processa i token in ordine:
- Il **primo WORD** dopo un PIPE (o all'inizio) è il **comando** → va in `args[0]`
- I **WORD successivi** sono **argomenti** → vanno in `args[1]`, `args[2]`, ecc.
- I **token REDIR** creano redirections (non argomenti)
- Un **PIPE** chiude il comando corrente e ne inizia uno nuovo

Esempio: `cat file.txt > out.txt`
- `args[0]` = "cat" (comando)
- `args[1]` = "file.txt" (argomento)
- `redirs` = [REDIR_OUT:"out.txt"]"

---

## 14.3 Esecuzione

### Q5: "Come gestisci le pipe?"

**Risposta:**

"Per ogni pipeline:

1. **Conto** i comandi (per allocare array di PIDs)
2. **Per ogni comando** (tranne l'ultimo):
   - Creo una pipe con `pipe()` → `[read_fd, write_fd]`
3. **Fork** un processo per comando
4. **Nel figlio**:
   - Se NON è il primo: `dup2(prev_pipe[0], STDIN)` → legge da pipe precedente
   - Se NON è l'ultimo: `dup2(curr_pipe[1], STDOUT)` → scrive su pipe corrente
   - Chiudo tutti i FD delle pipe
   - `execve()`
5. **Nel padre**:
   - Chiudo prev_pipe
   - Salvo curr_pipe come prev_pipe
6. **Aspetto** tutti i processi con `waitpid()`

Ogni comando vede solo stdin/stdout, non sa delle pipe."

---

### Q6: "Cosa succede quando esegui un comando esterno?"

**Risposta:**

"1. **Cerco il path**:
   - Se contiene `/` → uso path diretto
   - Altrimenti → cerco in $PATH directory per directory
   - Uso `stat()` e `access(X_OK)` per verificare eseguibilità

2. **Fork** un processo figlio

3. **Nel figlio**:
   - Applico redirections con `dup2()` se presenti
   - `execve(path, args, envp)` → sostituisce processo

4. **Nel padre**:
   - `waitpid()` aspetta terminazione
   - Estraggo exit status con `WEXITSTATUS()`
   - Se terminato da segnale: return `128 + signal_number`"

---

## 14.4 Builtins

### Q7: "Perché cd è un built-in?"

**Risposta:**

"Perché `cd` deve cambiare la **directory della shell stessa**. 

Se fosse un comando esterno:
1. Shell fa `fork()` → crea processo figlio
2. Figlio esegue `cd /tmp` → cambia directory del FIGLIO
3. Figlio termina
4. Shell padre rimane nella directory originale ❌

Essendo builtin:
1. Shell esegue `chdir("/tmp")` direttamente (no fork)
2. Directory della shell cambia ✓
3. Aggiorna `PWD` e `OLDPWD` nell'environment

Stesso motivo per `export`, `unset`, `exit`: devono modificare lo stato della shell."

---

### Q8: "Come funziona export?"

**Risposta:**

"Export gestisce variabili d'ambiente:

**Senza argomenti**: stampa tutte le variabili in formato `declare -x VAR="value"`

**Con argomenti** `export KEY=value`:
1. **Parso** argomento (separo KEY e value con `=`)
2. **Valido** KEY (solo lettere, numeri, underscore)
3. **Cerco** se KEY esiste già nell'environment:
   - Se esiste → sostituisco valore
   - Se non esiste → aggiungo nuova variabile (realloc environment)
4. Update `data->envp`

Esempio:
```bash
export TEST=hello
echo $TEST  # → hello
```"

---

## 14.5 Redirections

### Q9: "Come funzionano le redirections?"

**Risposta:**

"Le redirections modificano stdin/stdout con `dup2()`:

**Input `<`**:
1. `fd = open(file, O_RDONLY)`
2. `dup2(fd, STDIN_FILENO)` → stdin ora legge da file
3. `close(fd)`

**Output `>`**:
1. `fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)`
2. `dup2(fd, STDOUT_FILENO)` → stdout ora scrive su file
3. `close(fd)`

**Append `>>`**:
- Come `>` ma con `O_APPEND` invece di `O_TRUNC`

**Heredoc `<<`**:
1. Creo pipe
2. Leggo linee con `readline()` fino a delimiter
3. Scrivo linee nella pipe
4. Uso read end della pipe come stdin"

---

### Q10: "Cosa succede con multiple redirections?"

**Risposta:**

"Le applico **in ordine**. L'ultima vince.

Esempio: `cat < file1.txt < file2.txt`
1. Applico `< file1.txt` → stdin = file1.txt
2. Applico `< file2.txt` → stdin = file2.txt (sovrascrive!)
3. `cat` legge da file2.txt ✓

Esempio: `echo hello > a.txt > b.txt`
- a.txt → creato ma vuoto
- b.txt → contiene 'hello'

È il comportamento standard di bash."

---

## 14.6 Variabili e Espansione

### Q11: "Come funziona l'espansione delle variabili?"

**Risposta:**

"L'espansione avviene nel **lexer**, quando estraggo le parole:

1. **Trovo `$`** in una stringa
2. **Estraggo nome** variabile (fino a carattere non valido)
3. **Caso speciale `$?`**: ritorno exit status
4. **Caso normale `$VAR`**:
   - Cerco VAR in `data->envp`
   - Se trovata → sostituisco con valore
   - Se non trovata → sostituisco con stringa vuota
5. **Costruisco** stringa finale con variabili espanse

**Regole quotes**:
- Single `'$USER'` → NON espande (letterale: `$USER`)
- Double `"$USER"` → espande (risultato: `giusmery`)
- Senza quotes `$USER` → espande e splitta su spazi"

---

### Q12: "Cosa fa $?"

**Risposta:**

"`$?` contiene l'**exit status** dell'ultimo comando eseguito.

- **0** → successo
- **1-125** → errori vari
- **126** → comando non eseguibile
- **127** → comando non trovato
- **128+N** → terminato da segnale N (es: 130 = Ctrl-C)

Esempio:
```bash
/bin/true
echo $?  # → 0

/bin/false
echo $?  # → 1

commandnotfound
echo $?  # → 127
```

Salvo in `data->last_exit_status` dopo ogni comando."

---

## 14.7 Segnali

### Q13: "Come gestisci Ctrl-C?"

**Risposta:**

"Con un signal handler custom:

1. **Setup**: `sigaction(SIGINT, &sa, NULL)` con handler `handle_sigint`
2. **Nel handler**:
   - Salvo segnale in `g_signal` (variabile globale)
   - Stampo newline
   - Uso funzioni readline per mostrare nuovo prompt:
     - `rl_on_new_line()` → nuova linea
     - `rl_replace_line("", 0)` → cancella input
     - `rl_redisplay()` → ridisegna prompt

**Durante esecuzione comando**:
- Segnale va al processo figlio (non alla shell)
- Figlio termina
- Shell rileva terminazione con `WIFSIGNALED()`
- Return exit code `128 + SIGINT (2)` = 130"

---

### Q14: "Perché usi volatile sig_atomic_t per g_signal?"

**Risposta:**

"**`volatile`**: Dice al compilatore di non ottimizzare accessi alla variabile, perché può essere modificata in modo asincrono dal signal handler.

**`sig_atomic_t`**: Tipo garantito dal sistema per operazioni atomiche (non interrompibili). Importante perché il signal handler può interrompere il main in qualsiasi momento.

Senza questi, potrebbero esserci race condition tra main e handler."

---

## 14.8 Memory Management

### Q15: "Come gestisci i memory leaks?"

**Risposta:**

"Ho funzioni di cleanup per ogni struttura allocata:

- **`free_tokens()`**: libera lista token (value + struct)
- **`free_cmd_list()`**: libera comandi (args, redirections, struct)
- **`free_array()`**: libera array di stringhe
- **`free_envp()`**: libera environment
- **`cleanup_data()`**: chiamata all'uscita, libera tutto

**Regole**:
- Ogni `malloc()` ha il suo `free()`
- Free in ordine inverso all'allocazione
- NULL-check prima di free
- Testato con valgrind: 0 leaks"

---

### Q16: "Come testi i memory leaks?"

**Risposta:**

"Su Linux uso **valgrind**:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./minishell
```

Su macOS uso **leaks** (tool nativo):
```bash
./minishell &
leaks minishell
```

Oppure compilo con **fsanitize**:
```bash
make CFLAGS="-fsanitize=address -g"
./minishell
```

Testo vari scenari:
- Comandi semplici
- Pipeline
- Redirections
- Export/unset
- Exit

Risultato atteso: 0 bytes leaked"

---

## 14.9 Errori e Edge Cases

### Q17: "Come gestisci gli errori?"

**Risposta:**

"**Strategia**:
1. Ogni funzione ritorna status (SUCCESS/ERROR)
2. NULL-check dopo ogni `malloc()`
3. Check return value delle syscall (`fork`, `pipe`, `open`)
4. Stampo errori con `print_error()` su STDERR

**Exit codes**:
- 0: successo
- 1: errore generico
- 2: uso scorretto builtin
- 126: comando non eseguibile
- 127: comando non trovato

**Errori critici** (malloc, fork):
- Cleanup parziale
- Exit con ERROR"

---

### Q18: "Quali edge cases gestisci?"

**Risposta:**

"**Input**:
- Empty input (solo ENTER) → ignora
- Solo spazi/tab → ignora
- Quotes vuote `""` o `''` → argomento vuoto

**Variabili**:
- `$NOTEXIST` → stringa vuota
- `$` da solo → letterale `$`
- `$?` → sempre definito

**Redirections**:
- Multiple redirections → ultima vince
- File inesistente `<` → errore
- Permessi negati → errore

**Pipe**:
- Pipe vuota `|` → syntax error
- Pipe all'inizio `| cat` → syntax error

**Comandi**:
- Comando non trovato → exit 127
- Directory come comando → exit 126"

---

## 14.10 Norminette e Stile

### Q19: "Come hai rispettato la norminette?"

**Risposta:**

"**Limiti rispettati**:
- Max 25 linee per funzione → spezzato in helper
- Max 5 funzioni per file → creato file _utils, _handlers
- Max 4 parametri → usato struct (es: `t_pipe_data`)
- Max 5 variabili per funzione → ottimizzato scope
- 1 variabile globale → solo `g_signal`

**Tecniche**:
- Helper functions statiche
- Struct per raggruppare parametri correlati
- Divisione logica in moduli

Risultato: `norminette` → 0 errori (solo Notice per `g_signal`)"

---

### Q20: "Perché alcune funzioni sono static?"

**Risposta:**

"Le funzioni **static** sono private al file:
- **Helper** usate solo internamente
- **Utility** che non servono fuori dal modulo
- Migliora **encapsulation**
- Riduce **namespace pollution**

Solo funzioni API pubbliche sono nel `.h`

Esempio:
```c
// lexer.c
static void skip_spaces(char **input);  // Private
t_token *lexer(char *input);            // Public (in .h)
```"

---

## 14.11 Testing

### Q21: "Come hai testato il progetto?"

**Risposta:**

"**Test funzionali**:
- Comandi base: `echo`, `ls`, `pwd`, `cd`
- Builtins: tutti i 7 (echo, cd, pwd, export, unset, env, exit)
- Pipes: semplici e multiple (`cmd1 | cmd2 | cmd3`)
- Redirections: tutte e 4 (`<`, `>`, `>>`, `<<`)
- Quotes: singole, doppie, miste
- Variabili: `$VAR`, `$?`, espansione
- Segnali: Ctrl-C, Ctrl-D, Ctrl-\

**Test edge cases**:
- Input vuoto
- Solo spazi
- Variabili inesistenti
- Comandi non trovati
- Redirections multiple

**Test memoria**:
- Valgrind/leaks → 0 leaks
- Fsanitize → no errors

**Comparazione con bash**: stesso comportamento per tutti i casi mandatory"

---

## 14.12 Confronto con Bash

### Q22: "Quali differenze ci sono con bash?"

**Risposta:**

"**NON implementato** (come da subject):
- Operatori logici `&&`, `||`
- Separatore `;`
- Wildcards `*`
- Backslash escaping `\`
- Subshells `$(...)` o `` `...` ``
- Job control (`&`, `bg`, `fg`)
- Variables assignment `VAR=value` (senza export)

**Implementato**:
- Tutto il mandatory del subject
- Comportamento identico a bash per:
  - Espansione variabili
  - Quotes
  - Redirections
  - Pipes
  - Builtins
  - Segnali
  - Exit codes"

---

# 15. CHECKLIST FINALE PRE-VALUTAZIONE

## 15.1 Mandatory Part

### Esecuzione Comandi
- [x] Prompt interattivo `minishell$ `
- [x] History (funzioni readline)
- [x] Comandi con path assoluto (`/bin/ls`)
- [x] Comandi con path relativo (`./minishell`)
- [x] Comandi da $PATH (`ls`, `cat`, ecc.)
- [x] Comando non trovato → exit 127

### Quotes
- [x] Single quotes `'...'` → nessuna espansione
- [x] Double quotes `"..."` → espande variabili
- [x] Quote vuote `""` e `''`

### Redirections
- [x] Input `<` da file
- [x] Output `>` su file (truncate)
- [x] Append `>>` su file
- [x] Heredoc `<<` con delimiter
- [x] Multiple redirections

### Pipes
- [x] Pipe singola `cmd1 | cmd2`
- [x] Multiple pipes `cmd1 | cmd2 | cmd3`
- [x] Pipes + redirections

### Variables
- [x] Espansione `$VAR`
- [x] Exit status `$?`
- [x] Variabili inesistenti → stringa vuota

### Signals
- [x] Ctrl-C → nuovo prompt (non esce)
- [x] Ctrl-D → esce dalla shell
- [x] Ctrl-\ → ignorato
- [x] Ctrl-C durante comando → interrompe comando

### Built-ins
- [x] **echo** con opzione `-n`
- [x] **cd** con path relativo/assoluto/solo/~
- [x] **cd** senza args → va a HOME
- [x] **cd** aggiorna PWD e OLDPWD
- [x] **pwd** senza opzioni
- [x] **export** senza args → stampa variabili
- [x] **export** con args → crea/modifica variabili
- [x] **unset** rimuove variabili
- [x] **env** stampa environment
- [x] **exit** senza args → esce con last exit status
- [x] **exit** con numero → esce con quel codice
- [x] **exit** con non-numero → errore
- [x] **exit** troppi args → errore, non esce

---

## 15.2 Norminette

- [x] Norme 42 rispettate
- [x] Max 25 linee per funzione
- [x] Max 5 funzioni per file
- [x] Max 4 parametri per funzione
- [x] Max 5 variabili per funzione
- [x] Max 1 variabile globale (`g_signal`)
- [x] Header guards
- [x] No trailing whitespace
- [x] Indentazione con TAB
- [x] Nomi funzioni/variabili snake_case

---

## 15.3 Makefile

- [x] Rules: `all`, `clean`, `fclean`, `re`
- [x] Non relink
- [x] Compila con `-Wall -Wextra -Werror`
- [x] `$(NAME)` come target principale
- [x] No file object in root

---

## 15.4 Memory & Errors

- [x] No memory leaks (testato con valgrind/leaks)
- [x] No segmentation fault
- [x] No bus error
- [x] No double free
- [x] Ogni malloc ha free
- [x] Gestione errori malloc/fork/pipe
- [x] Free anche in caso di errori

---

## 15.5 Compilation

- [x] Compila senza warnings
- [x] Compila senza errori
- [x] `make` produce eseguibile `minishell`
- [x] `make re` ricompila tutto
- [x] Linked con `-lreadline`

---

# 16. TIPS PER LA VALUTAZIONE

## 16.1 Prima della Valutazione

### Prepara l'Ambiente
```bash
# Assicurati che compili
make fclean
make

# Test rapido
echo "echo hello" | ./minishell
```

### Prepara Spiegazioni
- Avere chiaro flusso: Lexer → Parser → Executor
- Sapere perché ogni builtin è builtin
- Conoscere differenze con bash
- Sapere gestione memoria

---

## 16.2 Durante la Valutazione

### Mostra Sicurezza
- Spiega con calma
- Usa diagrammi se aiuta

### Test Comuni che Faranno
```bash
# Builtins
echo hello
echo -n hello
cd /tmp
pwd
export TEST=hello
echo $TEST
unset TEST
env
exit

# Pipes e redirections
echo hello | cat
cat < file.txt
echo test > out.txt
cat << EOF

# Segnali
(premi Ctrl-C)
(premi Ctrl-D)

# Edge cases
echo $NOTEXIST
| cat  # syntax error
cat |   # syntax error
```

---

## 16.3 Domande Trabocchetto

### "Perché non usi system()?"
**Risposta:** "Il subject vieta esplicitamente `system()`. Implemento tutto con `fork()` + `execve()` per avere controllo completo sull'esecuzione."

### "Come gestisci le memory leaks di readline?"
**Risposta:** "Readline ha alcuni 'still reachable' che sono normali e accettati. Quello che conta è che il MIO codice non abbia leaks. Ho testato con suppressions per readline e il mio codice è pulito."

### "Cosa succede con Ctrl-C durante cat?"
**Risposta:** "Il segnale va al processo cat (figlio), non alla shell. Cat termina, la shell rileva terminazione con waitpid, vede WIFSIGNALED, e ritorna exit code 130 (128+2). La shell stessa non è interrotta."

---

# 17. FIX RECENTI E MIGLIORAMENTI

## 17.1 Export Marks System

**Problema risolto:** `export VAR` senza valore non appariva in `export`

**Implementazione:**
- Aggiunto campo `export_marks` a `t_data`
- Variabili marcate per export ma senza valore vengono salvate separatamente
- `export` senza args mostra sia `envp` che `export_marks`
- `env` mostra solo `envp` (variabili con valore)

**File modificati:**
- `includes/minishell.h` - aggiunto `export_marks` a `t_data`
- `src/export_marks_utils.c` - NUOVO FILE con funzioni helper
- `src/builtin_export.c` - integrazione export marks
- `src/init.c` - inizializzazione e cleanup

**Test:**
```bash
export VAR          # Marca VAR per export
export | grep VAR   # Appare: declare -x VAR
env | grep VAR      # Non appare
export VAR=hello    # Ora ha valore
env | grep VAR      # Appare: VAR=hello
```

---

## 17.2 Word Splitting After Expansion

**Problema risolto:** `export a="cho ciao"; e$a` cercava comando `"echo ciao"` invece di splittare

**Implementazione:**
- Dopo espansione variabili, se il primo argomento contiene spazi, viene splittato
- Usa `ft_split(value, ' ')` per dividere in argomenti separati
- SOLO per args[0] (comando), non per tutti gli argomenti

**File modificati:**
- `src/parser_handlers.c` - aggiunta funzione `contains_space()` e logica split

**Test:**
```bash
export a="cho ciao"
e$a                 # Espande → echo ciao → stampa: ciao
```

---

## 17.3 Edge Case: echo $"VAR"

**Problema risolto:** `echo $"PATH"` stampava `$PATH` invece di `PATH`

**Implementazione:**
- Nel lexer, quando vede `$` seguito immediatamente da `"`, ignora il `$`
- Ritorna stringa vuota per `$`, poi le doppie quote gestiscono il contenuto

**File modificati:**
- `src/lexer_helpers.c` - check speciale in `extract_word_part()`

**Test:**
```bash
echo $"PATH"   # Stampa: PATH (non $PATH)
echo $"USER"   # Stampa: USER
```

---

## 17.4 Memory Leak Fix

**Problema risolto:** Memory leak nei processi figli quando `execve` falliva

**Implementazione:**
- Creata funzione `cleanup_child()` che libera memoria senza chiamare `clear_history()`
- Chiamata prima di ogni `exit()` nei processi figli
- Libera: `envp`, `export_marks`, `cmd_list`, chiude `stdin_backup` e `stdout_backup`

**File modificati:**
- `src/init.c` - aggiunta funzione `cleanup_child()`
- `src/executor.c` - chiamata `cleanup_child()` prima di exit
- `src/pipes_utils.c` - chiamata `cleanup_child()` prima di exit

**Test con valgrind:**
```bash
valgrind --leak-check=full --suppressions=readline.supp ./minishell
sudo ls    # Comando che fallisce per permessi
exit       # No leak!
```

---

## 17.5 Export Validation

**Problema risolto:** Export accettava identificatori invalidi (`1TEST`, `TEST-VAR`, `=value`)

**Implementazione:**
- Validazione identificatori: primo char deve essere lettera o `_`
- Caratteri successivi: alfanumerici o `_`
- Reject: numeri all'inizio, trattini, solo `=`

**File modificati:**
- `src/builtin_export_utils.c` - funzione `is_valid_identifier()`
- `src/builtin_export.c` - validazione in `process_export_arg()`

**Test:**
```bash
export 1TEST=value      # Errore: not a valid identifier
export TEST-VAR=value   # Errore: not a valid identifier  
export =value           # Errore: not a valid identifier
export TEST=value       # OK
```

---

## 17.6 Readline Suppressions

**Aggiunto:** File `readline.supp` per valgrind

**Contenuto:** Suppression patterns per leak "still reachable" di readline/history

**Uso:**
```bash
valgrind --leak-check=full --suppressions=readline.supp ./minishell
```

---

# 18. CONCLUSIONE

## 18.1 Architettura Finale

```
┌─────────────────────────────────────────────────────────────┐
│                    MINISHELL ARCHITECTURE                   │
└─────────────────────────────────────────────────────────────┘

INPUT → LEXER → PARSER → EXPANDER → EXECUTOR → OUTPUT
         ↓        ↓          ↓          ↓
      Tokens  Commands  Variables   Execution
                                   (fork/exec)
```

## 18.2 Moduli Implementati

| Modulo | Responsabilità | Files |
|--------|----------------|-------|
| **Main** | Loop principale, readline | `main.c`, `init.c` |
| **Lexer** | Tokenizzazione input | `lexer*.c` (3 files) |
| **Parser** | Costruzione AST | `parser*.c` (3 files) |
| **Expander** | Espansione variabili | `expand*.c` (3 files) |
| **Executor** | Esecuzione comandi | `executor.c`, `path_utils.c` |
| **Pipes** | Gestione pipeline | `pipes*.c` (3 files) |
| **Redirections** | Gestione I/O | `redirections*.c` (2 files) |
| **Builtins** | Comandi integrati | `builtin_*.c` (7 files) |
| **Environment** | Variabili ambiente | `env_utils*.c` (2 files) |
| **Signals** | Gestione segnali | `signals.c` |
| **Utils** | Utility generiche | `utils*.c`, `error.c` |

---

## 18.3 Features Implementate

### ✅ Mandatory
- Prompt interattivo
- History
- Comandi esterni (con ricerca PATH)
- Builtins (7 totali)
- Redirections (4 tipi)
- Pipes (multiple)
- Variabili d'ambiente
- Quotes (singole e doppie)
- Segnali (Ctrl-C, Ctrl-D, Ctrl-\)
- Exit status


---

## 18.4 Statistiche Progetto

```
Files:           ~35 .c files + 1 .h
Lines of code:   ~3000-4000 LOC
Functions:       ~150-200 functions
Structs:         5 main structs
Max func length: 25 lines (norminette)
Global vars:     1 (g_signal)
Compilation:     0 warnings, 0 errors
```

---

**Created with ❤️** 🌙

---

**Fine della guida completa**