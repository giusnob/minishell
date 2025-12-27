# 📚 MINISHELL - GUIDA COMPLETA E DETTAGLIATA

**Progetto:** Minishell  
**Autori:** Giusmery & Giulia  

---

## 📑 INDICE

1. [Introduzione](#introduzione)
2. [Architettura Generale](#architettura-generale)
3. [Fase 1: Setup e Inizializzazione](#fase-1-setup-e-inizializzazione)
4. [Fase 2: Lexer (Analisi Lessicale)](#fase-2-lexer-analisi-lessicale)
5. [Fase 3: Parser (Analisi Sintattica)](#fase-3-parser-analisi-sintattica)
6. [Fase 4: Executor (Esecuzione)](#fase-4-executor-esecuzione)
7. [Fase 5: Built-ins](#fase-5-built-ins)
8. [Fase 6: Variabili d'Ambiente](#fase-6-variabili-dambiente)
9. [Fase 7: Redirections](#fase-7-redirections)
10. [Fase 8: Pipe](#fase-8-pipe)
11. [Fase 9: Segnali](#fase-9-segnali)
12. [Gestione della Memoria](#gestione-della-memoria)
13. [Testing e Debugging](#testing-e-debugging)

---

## 1. INTRODUZIONE

### 1.1 Cos'è una Shell?

Una **shell** è un programma che:
- Legge comandi dall'utente
- Interpreta questi comandi
- Li esegue interagendo con il sistema operativo

**Esempi:** bash, zsh, sh

### 1.2 Cosa fa Minishell?

Minishell è una **shell semplificata** che implementa le funzionalità base di bash:

✅ **Prompt interattivo** (legge comandi)  
✅ **History** (frecce su/giù per comandi precedenti)  
✅ **Esecuzione comandi** (cerca ed esegue programmi)  
✅ **Built-ins** (comandi integrati: echo, cd, pwd, export, unset, env, exit)  
✅ **Quote** (singole `'` e doppie `"`)  
✅ **Redirections** (`<`, `>`, `>>`, `<<`)  
✅ **Pipe** (`|`)  
✅ **Variabili** (`$VAR`, `$?`)  
✅ **Segnali** (Ctrl+C, Ctrl+D, Ctrl+\)

---

## 2. ARCHITETTURA GENERALE

### 2.1 Il Ciclo REPL (Read-Eval-Print-Loop)

```
┌─────────────────────────────────────────┐
│  1. READ:    Legge input dall'utente    │
│              (con readline)             │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  2. LEXER:   Tokenizza l'input          │
│              "echo hello" → [echo][hello]│
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  3. PARSER:  Costruisce struttura       │
│              comandi                    │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  4. EXECUTOR: Esegue i comandi          │
│               (fork + execve)           │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  5. LOOP:    Torna al prompt            │
└─────────────────────────────────────────┘
```

### 2.2 Strutture Dati Principali

#### **t_token** (Lexer output)
```c
typedef struct s_token
{
    t_token_type    type;      // Tipo: WORD, PIPE, REDIR_IN, etc.
    char            *value;    // Valore: "ls", "|", "file.txt"
    struct s_token  *next;     // Puntatore al prossimo token
}   t_token;
```

**Esempio:**
```
Input:  "ls -la | grep test"
Tokens: [WORD:"ls"] → [WORD:"-la"] → [PIPE:"|"] → [WORD:"grep"] → [WORD:"test"]
```

#### **t_cmd** (Parser output)
```c
typedef struct s_cmd
{
    char            **args;     // Array di argomenti ["ls", "-la", NULL]
    struct s_redir  *redirs;    // Lista di redirections
    struct s_cmd    *next;      // Prossimo comando (per pipe)
}   t_cmd;
```

**Esempio:**
```
Input:  "cat file.txt > output.txt"
CMD:    args = ["cat", "file.txt", NULL]
        redirs = [REDIR_OUT: "output.txt"]
```

#### **t_data** (Dati globali)
```c
typedef struct s_data
{
    char    **envp;              // Copia dell'environment
    t_cmd   *cmd_list;           // Lista di comandi da eseguire
    int     last_exit_status;    // Exit status ($?)
    int     stdin_backup;        // Backup di stdin
    int     stdout_backup;       // Backup di stdout
}   t_data;
```

---

## 3. FASE 1: SETUP E INIZIALIZZAZIONE

### 3.1 Main Loop

**File:** `src/main.c`

```c
int main(int argc, char **argv, char **envp)
{
    t_data  data;
    char    *input;
    
    // 1. Inizializza i dati
    init_data(&data, envp);
    
    // 2. Setup segnali
    setup_signals();
    
    // 3. Loop principale
    while (1)
    {
        // Legge input
        input = readline("minishell$ ");
        
        if (!input)  // Ctrl+D
            break;
            
        if (*input)  // Se non vuoto
        {
            add_history(input);  // Aggiunge a history
            
            // Processa il comando
            process_input(&data, input);
        }
        
        free(input);
    }
    
    // 4. Cleanup
    cleanup_data(&data);
    return (data.last_exit_status);
}
```

### 3.2 Inizializzazione

**File:** `src/init.c`

```c
int init_data(t_data *data, char **envp)
{
    // Copia l'environment (dobbiamo poterlo modificare)
    data->envp = copy_envp(envp);
    
    // Inizializza
    data->cmd_list = NULL;
    data->last_exit_status = 0;
    
    // Salva stdin/stdout originali (per redirections)
    data->stdin_backup = dup(STDIN_FILENO);
    data->stdout_backup = dup(STDOUT_FILENO);
    
    return (SUCCESS);
}
```

**Perché copiare envp?**  
Perché `export` e `unset` devono modificare l'environment, ma non possiamo modificare l'array originale del sistema.

### 3.3 Readline

**Libreria:** `readline`

```c
char *readline(const char *prompt);
```

- Mostra il prompt
- Legge input dall'utente
- Gestisce automaticamente la history (frecce su/giù)
- Ritorna NULL se l'utente preme Ctrl+D

**Linking:**
```makefile
LDFLAGS = -lreadline
```

---

## 4. FASE 2: LEXER (ANALISI LESSICALE)

### 4.1 Cos'è il Lexer?

Il **lexer** (o scanner) trasforma una stringa di caratteri in una sequenza di **token**.

**Token:** Unità lessicale minima (come una "parola" del linguaggio)

**Esempio:**
```
Input:  "echo hello > file.txt"

Token:  [WORD: "echo"]
        [WORD: "hello"]
        [REDIR_OUT: ">"]
        [WORD: "file.txt"]
```

### 4.2 Tipi di Token

```c
typedef enum e_token_type
{
    TOKEN_WORD,           // Parola normale: "ls", "hello"
    TOKEN_PIPE,           // Pipe: |
    TOKEN_REDIR_IN,       // Input redirect: <
    TOKEN_REDIR_OUT,      // Output redirect: >
    TOKEN_REDIR_APPEND,   // Append: >>
    TOKEN_REDIR_HEREDOC   // Heredoc: <<
}   t_token_type;
```

### 4.3 Algoritmo del Lexer

**File:** `src/lexer.c`

```c
t_token *lexer(char *input, t_data *data)
{
    t_token *tokens = NULL;
    
    while (*input)
    {
        // 1. Salta spazi
        skip_spaces(&input);
        
        if (!*input)
            break;
        
        // 2. Identifica il tipo di token
        if (*input == '|' || *input == '<' || *input == '>')
            new_token = handle_operator(&input);
        else
            new_token = handle_word(&input, data);
        
        // 3. Aggiunge alla lista
        add_token(&tokens, new_token);
    }
    
    return (tokens);
}
```

### 4.4 Gestione delle Quote

#### **Quote Singole** `'...'`
- Tutto è **letterale**
- NO espansione di variabili
- NO interpretazione di caratteri speciali

```c
static char *extract_single_quote(char **input)
{
    (*input)++;  // Salta '
    
    // Trova la chiusura
    int len = 0;
    while ((*input)[len] && (*input)[len] != '\'')
        len++;
    
    if ((*input)[len] != '\'')
        return (error("unclosed quote"));
    
    // Estrae il contenuto
    word = malloc(len + 1);
    strncpy(word, *input, len);
    word[len] = '\0';
    
    *input += len + 1;  // Salta anche la '
    return (word);
}
```

**Esempio:**
```
Input:  echo '$USER'
Output: $USER  (letterale, non espanso)
```

#### **Quote Doppie** `"..."`
- Le variabili SI espandono
- I caratteri speciali sono letterali

```c
static char *extract_double_quote(char **input, t_data *data)
{
    (*input)++;  // Salta "
    
    // Estrae contenuto
    int len = 0;
    while ((*input)[len] && (*input)[len] != '"')
        len++;
    
    word = malloc(len + 1);
    strncpy(word, *input, len);
    word[len] = '\0';
    
    *input += len + 1;
    
    // IMPORTANTE: Espandi variabili!
    expanded = expand_variables(word, data);
    free(word);
    
    return (expanded);
}
```

**Esempio:**
```
Input:  echo "$USER"
Output: giusmerynobile  (espanso!)
```

### 4.5 Gestione Operatori

```c
static t_token *handle_operator(char **input)
{
    if (**input == '|')
    {
        type = TOKEN_PIPE;
        value = "|";
        (*input)++;
    }
    else if (**input == '<')
    {
        if ((*input)[1] == '<')  // <<
        {
            type = TOKEN_REDIR_HEREDOC;
            value = "<<";
            (*input) += 2;
        }
        else  // <
        {
            type = TOKEN_REDIR_IN;
            value = "<";
            (*input)++;
        }
    }
    // ... stesso per > e >>
    
    return (create_token(type, value));
}
```

---

## 5. FASE 3: PARSER (ANALISI SINTATTICA)

### 5.1 Cos'è il Parser?

Il **parser** trasforma i token in una **struttura dati** che rappresenta i comandi da eseguire.

**Input:** Lista di token  
**Output:** Lista di comandi (`t_cmd`)

### 5.2 Grammatica della Shell

```
command_line  ::= command ('|' command)*
command       ::= word* redirection*
redirection   ::= '<' file | '>' file | '>>' file | '<<' delimiter
```

**Esempio:**
```
Input tokens: [cat] [file.txt] [|] [grep] [test] [>] [output.txt]

Parser crea:
    CMD1: args=["cat", "file.txt"]  redirs=NULL
    CMD2: args=["grep", "test"]     redirs=[>output.txt]
```

### 5.3 Algoritmo del Parser

**File:** `src/parser.c`

```c
t_cmd *parser(t_token *tokens)
{
    t_cmd *cmd_list = NULL;
    
    while (tokens)
    {
        // Processa un comando (fino alla prossima pipe)
        t_cmd *new_cmd = process_command(&tokens);
        
        // Aggiunge alla lista
        add_cmd(&cmd_list, new_cmd);
        
        // Se c'è una pipe, salta e continua
        if (tokens && tokens->type == TOKEN_PIPE)
            tokens = tokens->next;
    }
    
    return (cmd_list);
}
```

### 5.4 Process Command

```c
static t_cmd *process_command(t_token **tokens)
{
    t_cmd *cmd = create_cmd();
    
    while (*tokens && (*tokens)->type != TOKEN_PIPE)
    {
        if ((*tokens)->type == TOKEN_WORD)
        {
            // Aggiungi argomento
            add_args_to_cmd(cmd, tokens);
        }
        else if (is_redirection(*tokens))
        {
            // Aggiungi redirection
            handle_redirection(cmd, tokens);
        }
    }
    
    return (cmd);
}
```

### 5.5 Gestione Redirections

```c
static int handle_redirection(t_cmd *cmd, t_token **tokens)
{
    // Prende il tipo di redirection
    t_redir_type type = get_redir_type((*tokens)->type);
    
    // Avanza al prossimo token (il file)
    *tokens = (*tokens)->next;
    
    if (!*tokens || (*tokens)->type != TOKEN_WORD)
        return (error("syntax error"));
    
    // Crea redirection
    t_redir *redir = create_redir(type, (*tokens)->value);
    
    // Aggiunge al comando
    add_redir(&cmd->redirs, redir);
    
    *tokens = (*tokens)->next;
    return (SUCCESS);
}
```

---

## 6. FASE 4: EXECUTOR (ESECUZIONE)

### 6.1 Cos'è l'Executor?

L'**executor** prende la lista di comandi dal parser e li **esegue**.

### 6.2 Built-in vs External Command

```c
int execute_simple_cmd(t_data *data, t_cmd *cmd)
{
    // Controlla se è built-in
    if (is_builtin(cmd->args[0]))
        return (exec_builtin(data, cmd));
    
    // Altrimenti esegue comando esterno
    return (exec_external_cmd(data, cmd));
}
```

### 6.3 Esecuzione Comandi Esterni

#### 6.3.1 Ricerca del Comando

**File:** `src/path_utils.c`

```c
char *find_command_path(char *cmd, char **envp)
{
    // 1. Se contiene '/', è un path assoluto/relativo
    if (has_slash(cmd))
    {
        if (is_executable(cmd))
            return (strdup(cmd));
        return (NULL);
    }
    
    // 2. Altrimenti cerca nel PATH
    char *path_env = get_env_value("PATH", envp);
    char **paths = split(path_env, ':');
    
    // 3. Prova ogni directory
    for (int i = 0; paths[i]; i++)
    {
        char *full_path = join(paths[i], "/", cmd);
        
        if (is_executable(full_path))
            return (full_path);
        
        free(full_path);
    }
    
    return (NULL);  // Non trovato
}
```

**Esempio:**
```
cmd = "ls"
PATH = "/usr/bin:/bin"

Prova:
    /usr/bin/ls  ✗ (non esiste)
    /bin/ls      ✓ (trovato!)
    
Ritorna: "/bin/ls"
```

#### 6.3.2 Fork + Execve

```c
int exec_external_cmd(t_data *data, t_cmd *cmd)
{
    // 1. Trova il path
    char *cmd_path = find_command_path(cmd->args[0], data->envp);
    
    if (!cmd_path)
        return (error("command not found"));
    
    // 2. Fork (crea processo figlio)
    pid_t pid = fork();
    
    if (pid == 0)
    {
        // PROCESSO FIGLIO
        
        // Applica redirections
        apply_redirections(cmd);
        
        // Esegue il comando (sostituisce il processo)
        execve(cmd_path, cmd->args, data->envp);
        
        // Se arriviamo qui, execve è fallito
        exit(127);
    }
    else
    {
        // PROCESSO PADRE
        
        // Aspetta che il figlio finisca
        int status;
        waitpid(pid, &status, 0);
        
        // Estrae exit code
        if (WIFEXITED(status))
            return (WEXITSTATUS(status));
    }
}
```

**Cosa fa fork()?**
- Crea una **copia identica** del processo
- Ritorna **0** nel figlio, **PID** nel padre

**Cosa fa execve()?**
- **Sostituisce** il processo corrente con un nuovo programma
- Se ha successo, **non ritorna mai**
- Se fallisce, ritorna -1

**Perché fork + execve?**
- **fork**: Creiamo un processo separato per il comando
- **execve**: Eseguiamo il comando in quel processo
- Il padre (minishell) continua a esistere!

### 6.4 Exit Status

```c
// Nel figlio dopo execve
exit(exit_code);

// Nel padre
int status;
waitpid(pid, &status, 0);

if (WIFEXITED(status))
    exit_code = WEXITSTATUS(status);  // Codice uscita normale

if (WIFSIGNALED(status))
    exit_code = 128 + WTERMSIG(status);  // Ucciso da segnale

// Salva in $?
data->last_exit_status = exit_code;
```

---

## 7. FASE 5: BUILT-INS

### 7.1 Cosa sono i Built-ins?

I **built-ins** sono comandi **integrati nella shell**, non programmi esterni.

**Perché?**  
Alcuni comandi devono modificare lo **stato della shell** (es. `cd` cambia directory, `export` modifica environment).

### 7.2 Lista Built-ins

| Built-in | Funzione |
|----------|----------|
| `echo` | Stampa testo |
| `cd` | Cambia directory |
| `pwd` | Stampa directory corrente |
| `export` | Crea/modifica variabile |
| `unset` | Rimuove variabile |
| `env` | Mostra environment |
| `exit` | Esce dalla shell |

### 7.3 Implementazione Built-ins

#### **echo**

**File:** `src/builtin_echo.c`

```c
int builtin_echo(char **args)
{
    int newline = 1;  // Di default stampa newline
    int i = 1;
    
    // Controlla flag -n
    while (args[i] && is_flag_n(args[i]))
    {
        newline = 0;
        i++;
    }
    
    // Stampa argomenti
    while (args[i])
    {
        printf("%s", args[i]);
        if (args[i + 1])
            printf(" ");
        i++;
    }
    
    if (newline)
        printf("\n");
    
    return (SUCCESS);
}
```

**Particolarità:**
- `-n`: Non stampa newline finale
- `-nnnn`: Multipli `-n` contano come uno solo

#### **cd**

**File:** `src/builtin_cd.c`

```c
int builtin_cd(char **args, t_data *data)
{
    char *path;
    
    // Salva directory corrente (per OLDPWD)
    char old_pwd[1024];
    getcwd(old_pwd, sizeof(old_pwd));
    
    // Determina dove andare
    if (!args[1])
        path = get_env_value("HOME", data->envp);  // cd senza args
    else if (strcmp(args[1], "-") == 0)
        path = get_env_value("OLDPWD", data->envp);  // cd -
    else if (args[1][0] == '~')
        path = expand_tilde(args[1], data);  // cd ~
    else
        path = args[1];
    
    // Cambia directory
    if (chdir(path) == -1)
        return (error("cd: no such file or directory"));
    
    // Aggiorna PWD e OLDPWD
    update_pwd(data, old_pwd);
    
    return (SUCCESS);
}
```

**Perché cd è built-in?**  
Perché deve cambiare la directory **della shell stessa**, non di un processo figlio!

#### **pwd**

```c
int builtin_pwd(void)
{
    char cwd[1024];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        return (error("pwd: error getting current directory"));
    
    printf("%s\n", cwd);
    return (SUCCESS);
}
```

#### **export**

**File:** `src/builtin_export.c`

```c
int builtin_export(char **args, t_data *data)
{
    // Senza argomenti: mostra tutte le variabili
    if (!args[1])
    {
        print_all_vars(data);
        return (SUCCESS);
    }
    
    // Con argomenti: esporta variabili
    for (int i = 1; args[i]; i++)
    {
        // Parse "KEY=VALUE"
        char *key, *value;
        parse_export_arg(args[i], &key, &value);
        
        // Aggiunge/modifica nell'environment
        set_env_value(key, value, data);
    }
    
    return (SUCCESS);
}
```

#### **unset**

```c
int builtin_unset(char **args, t_data *data)
{
    for (int i = 1; args[i]; i++)
    {
        unset_env_value(args[i], data);
    }
    
    return (SUCCESS);
}
```

#### **env**

```c
int builtin_env(char **envp)
{
    for (int i = 0; envp[i]; i++)
        printf("%s\n", envp[i]);
    
    return (SUCCESS);
}
```

#### **exit**

```c
int builtin_exit(char **args, t_data *data)
{
    printf("exit\n");
    
    // Senza argomenti
    if (!args[1])
        exit(data->last_exit_status);
    
    // Con argomento numerico
    if (is_numeric(args[1]))
    {
        if (args[2])  // Troppi argomenti
        {
            printf("exit: too many arguments\n");
            return (ERROR);
        }
        exit(atoi(args[1]));
    }
    
    // Argomento non numerico
    printf("exit: numeric argument required\n");
    exit(255);
}
```

---

## 8. FASE 6: VARIABILI D'AMBIENTE

### 8.1 Espansione Variabili

Le variabili d'ambiente vengono espanse quando trovate nel formato `$NOME`.

**File:** `src/expand.c`

```c
char *expand_variables(char *str, t_data *data)
{
    char *result = "";
    
    int i = 0;
    while (str[i])
    {
        if (str[i] == '$' && str[i + 1])
        {
            // Estrae nome variabile
            char *var_name = extract_var_name(str + i);
            
            // Ottiene valore
            char *value = get_var_value(var_name, data);
            
            // Aggiunge al risultato
            result = strjoin(result, value);
            
            i += strlen(var_name) + 1;
        }
        else
        {
            // Carattere normale
            result = append_char(result, str[i]);
            i++;
        }
    }
    
    return (result);
}
```

### 8.2 Variabile Speciale $?

`$?` contiene l'**exit status** dell'ultimo comando.

```c
char *get_var_value(char *name, t_data *data)
{
    // Caso speciale: $?
    if (strcmp(name, "?") == 0)
        return (itoa(data->last_exit_status));
    
    // Variabile normale
    return (get_env_value(name, data->envp));
}
```

**Esempio:**
```bash
minishell$ ls
file.txt
minishell$ echo $?
0

minishell$ ls filechenonesiste
ls: cannot access: No such file or directory
minishell$ echo $?
1
```

### 8.3 Gestione Environment

```c
char *get_env_value(char *key, char **envp)
{
    int key_len = strlen(key);
    
    for (int i = 0; envp[i]; i++)
    {
        // Controlla se inizia con "KEY="
        if (strncmp(envp[i], key, key_len) == 0 &&
            envp[i][key_len] == '=')
        {
            // Ritorna valore dopo '='
            return (strdup(envp[i] + key_len + 1));
        }
    }
    
    return (NULL);  // Non trovata
}
```

```c
int set_env_value(char *key, char *value, t_data *data)
{
    // Controlla se esiste già
    for (int i = 0; data->envp[i]; i++)
    {
        if (starts_with(data->envp[i], key))
        {
            // Sostituisci
            free(data->envp[i]);
            data->envp[i] = format("%s=%s", key, value);
            return (SUCCESS);
        }
    }
    
    // Non esiste, aggiungila
    int size = array_size(data->envp);
    data->envp = realloc(data->envp, (size + 2) * sizeof(char *));
    data->envp[size] = format("%s=%s", key, value);
    data->envp[size + 1] = NULL;
    
    return (SUCCESS);
}
```

---

## 9. FASE 7: REDIRECTIONS

### 9.1 Tipi di Redirection

| Operatore | Nome | Descrizione |
|-----------|------|-------------|
| `<` | Input | Legge da file invece che da stdin |
| `>` | Output | Scrive su file invece che su stdout |
| `>>` | Append | Aggiunge a file esistente |
| `<<` | Heredoc | Legge fino a delimiter |

### 9.2 Come Funzionano le Redirections?

Le redirections usano **file descriptors**:
- `0` = stdin (input standard)
- `1` = stdout (output standard)
- `2` = stderr (errori)

**Esempio:**
```bash
cat < input.txt > output.txt
```

```
Normalmente:
    stdin (0) → keyboard
    stdout (1) → screen

Con redirections:
    stdin (0) → input.txt
    stdout (1) → output.txt
```

### 9.3 Implementazione

**File:** `src/redirections.c`

```c
int apply_redirections(t_cmd *cmd)
{
    t_redir *redir = cmd->redirs;
    
    while (redir)
    {
        int fd = open_redir_file(redir);
        
        if (fd == -1)
            return (ERROR);
        
        // Redirigi stdin o stdout
        if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
        {
            dup2(fd, STDIN_FILENO);  // stdin ora legge da fd
        }
        else if (redir->type == REDIR_OUT || redir->type == REDIR_APPEND)
        {
            dup2(fd, STDOUT_FILENO);  // stdout ora scrive su fd
        }
        
        close(fd);
        redir = redir->next;
    }
    
    return (SUCCESS);
}
```

### 9.4 dup2() - Duplicazione File Descriptor

```c
int dup2(int oldfd, int newfd);
```

**Cosa fa:**
- Copia `oldfd` in `newfd`
- Chiude `newfd` se era aperto
- Ora `newfd` punta allo stesso file di `oldfd`

**Esempio:**
```c
int fd = open("file.txt", O_RDONLY);  // fd = 3
dup2(fd, 0);  // stdin (0) ora legge da file.txt
close(fd);    // Possiamo chiudere fd

// Ora read(0, ...) legge da file.txt!
```

### 9.5 Heredoc

**Cos'è:**  
Heredoc legge input fino a trovare un **delimiter**.

**Esempio:**
```bash
cat << EOF
line 1
line 2
EOF
```

**Implementazione:**
```c
int handle_heredoc(char *delimiter)
{
    int pipe_fd[2];
    pipe(pipe_fd);  // Crea pipe
    
    char *line;
    while (1)
    {
        line = readline("> ");
        
        // Controlla delimiter
        if (strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // Scrive nella pipe
        write(pipe_fd[1], line, strlen(line));
        write(pipe_fd[1], "\n", 1);
        free(line);
    }
    
    close(pipe_fd[1]);  // Chiude scrittura
    return (pipe_fd[0]);  // Ritorna lettura
}
```

---

## 10. FASE 8: PIPE

### 10.1 Cos'è una Pipe?

Una **pipe** collega l'output di un comando all'input del successivo.

**Esempio:**
```bash
cat file.txt | grep hello | wc -l
```

```
┌──────┐      ┌──────┐      ┌────┐
│ cat  │ ───> │ grep │ ───> │ wc │
└──────┘      └──────┘      └────┘
```

### 10.2 Come Funziona?

```
        pipe()
    ┌──────────────┐
    │              │
    ▼              ▼
  read(0)      write(1)
  
Comando 1:  stdout → write(1)
Comando 2:  stdin  ← read(0)
```

### 10.3 Implementazione Pipeline

**File:** `src/pipes.c`

```c
int execute_pipeline(t_data *data)
{
    t_cmd *current = data->cmd_list;
    int prev_pipe[2] = {-1, -1};
    int curr_pipe[2];
    
    while (current)
    {
        // Crea pipe se non è l'ultimo comando
        if (current->next)
            pipe(curr_pipe);
        
        pid_t pid = fork();
        
        if (pid == 0)  // Figlio
        {
            // Input dalla pipe precedente
            if (prev_pipe[0] != -1)
            {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            
            // Output alla pipe corrente
            if (current->next)
            {
                dup2(curr_pipe[1], STDOUT_FILENO);
                close(curr_pipe[0]);
                close(curr_pipe[1]);
            }
            
            // Esegui comando
            exec_command(data, current);
        }
        
        // Padre: chiude pipe precedente
        if (prev_pipe[0] != -1)
        {
            close(prev_pipe[0]);
            close(prev_pipe[1]);
        }
        
        // Salva pipe corrente come precedente
        if (current->next)
        {
            prev_pipe[0] = curr_pipe[0];
            prev_pipe[1] = curr_pipe[1];
        }
        
        current = current->next;
    }
    
    // Aspetta tutti i figli
    wait_all_children();
    
    return (SUCCESS);
}
```

### 10.4 Schema Pipeline

```
cat file.txt | grep hello | wc -l

FORK 1:
    stdin: keyboard
    stdout: pipe1[1]  ───┐
    cmd: cat file.txt    │
                         │
FORK 2:                  │
    stdin: pipe1[0]  <───┘
    stdout: pipe2[1] ───┐
    cmd: grep hello      │
                         │
FORK 3:                  │
    stdin: pipe2[0]  <───┘
    stdout: screen
    cmd: wc -l
```

---

## 11. FASE 9: SEGNALI

### 11.1 Cosa sono i Segnali?

I **segnali** sono notifiche asincrone inviate ai processi.

**Esempi:**
- `SIGINT` (Ctrl+C): Interrompe il processo
- `SIGQUIT` (Ctrl+\): Termina con core dump
- `SIGTERM`: Termina gentilmente

### 11.2 Gestione Segnali in Minishell

| Segnale | Tasto | Comportamento |
|---------|-------|---------------|
| SIGINT | Ctrl+C | Nuova linea, nuovo prompt |
| SIGQUIT | Ctrl+\ | Ignorato (non fa niente) |
| EOF | Ctrl+D | Esce dalla shell |

### 11.3 Implementazione

**File:** `src/signals.c`

```c
// UNICA variabile globale permessa!
volatile sig_atomic_t g_signal = 0;

void handle_sigint(int sig)
{
    (void)sig;
    
    g_signal = 1;  // Marca che abbiamo ricevuto il segnale
    
    write(STDOUT_FILENO, "\n", 1);  // Nuova linea
    rl_on_new_line();  // Readline: vai a nuova linea
    rl_redisplay();    // Mostra nuovo prompt
}

void setup_signals(void)
{
    // SIGINT: custom handler
    signal(SIGINT, handle_sigint);
    
    // SIGQUIT: ignora
    signal(SIGQUIT, SIG_IGN);
}
```

### 11.4 Perché g_signal?

**Problema:** I signal handler **non possono accedere a variabili locali**.

**Soluzione:** Una variabile globale `g_signal` che indica se abbiamo ricevuto un segnale.

**Subject 42:** Permette **MASSIMO 1 variabile globale** per i segnali.

### 11.5 Segnali Durante Esecuzione

Quando un comando è in esecuzione, i segnali devono funzionare diversamente:

```c
void setup_execution_signals(void)
{
    // Durante esecuzione: comportamento default
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

void restore_interactive_signals(void)
{
    // Torna a modalità interattiva
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, SIG_IGN);
}
```

---

## 12. GESTIONE DELLA MEMORIA

### 12.1 Principio Fondamentale

**Ogni malloc() DEVE avere il suo free()!**

### 12.2 Memory Leaks Comuni

#### Problema 1: Token non liberati
```c
// ❌ SBAGLIATO
t_token *tokens = lexer(input);
// ... usa tokens
// LEAK! Non liberiamo i token!
```

```c
// ✅ CORRETTO
t_token *tokens = lexer(input);
// ... usa tokens
free_tokens(tokens);  // Libera tutti i token
```

#### Problema 2: Comandi non liberati
```c
// ❌ SBAGLIATO
t_cmd *cmd_list = parser(tokens);
// ... esegue
// LEAK! Non liberiamo i comandi!
```

```c
// ✅ CORRETTO
t_cmd *cmd_list = parser(tokens);
// ... esegue
free_cmd_list(cmd_list);  // Libera tutto
```

### 12.3 Funzioni di Free

```c
void free_tokens(t_token *tokens)
{
    t_token *tmp;
    
    while (tokens)
    {
        tmp = tokens;
        tokens = tokens->next;
        
        free(tmp->value);  // Libera valore
        free(tmp);         // Libera struttura
    }
}

void free_cmd_list(t_cmd *cmd_list)
{
    t_cmd *tmp_cmd;
    
    while (cmd_list)
    {
        tmp_cmd = cmd_list;
        cmd_list = cmd_list->next;
        
        // Libera argomenti
        free_array(tmp_cmd->args);
        
        // Libera redirections
        free_redirs(tmp_cmd->redirs);
        
        // Libera struttura
        free(tmp_cmd);
    }
}
```

### 12.4 Valgrind

**Tool per trovare memory leaks:**

```bash
valgrind --leak-check=full ./minishell
```

**Output ideale:**
```
All heap blocks were freed -- no leaks are possible
```

---

## 13. TESTING E DEBUGGING

### 13.1 Test Manuali

#### Test Base
```bash
./minishell

# Comandi semplici
minishell$ ls
minishell$ echo hello
minishell$ pwd

# Built-ins
minishell$ cd /tmp
minishell$ pwd
minishell$ export TEST=hello
minishell$ echo $TEST
minishell$ unset TEST

# Quote
minishell$ echo "hello world"
minishell$ echo '$USER'
minishell$ echo "$USER"

# Redirections
minishell$ echo hello > file.txt
minishell$ cat < file.txt
minishell$ echo world >> file.txt

# Pipe
minishell$ ls | grep test
minishell$ cat file.txt | wc -l

# Exit status
minishell$ ls
minishell$ echo $?
minishell$ ls fileinesistente
minishell$ echo $?
```

#### Test Segnali
```bash
# Ctrl+C in prompt vuoto
minishell$ ^C
minishell$

# Ctrl+C dopo aver scritto
minishell$ hello^C
minishell$

# Ctrl+C durante comando
minishell$ cat
^C
minishell$

# Ctrl+D (exit)
minishell$ ^D
exit
```

### 13.2 Script di Test

**File:** `test_minishell.sh`

Testa automaticamente:
- Comandi semplici
- Echo con flag
- Exit status
- Quote
- Built-ins
- Redirections
- Pipe
- Variabili

### 13.3 Debugging con GDB

```bash
gdb ./minishell

(gdb) break main
(gdb) run
(gdb) next
(gdb) print data
(gdb) continue
```

### 13.4 Debugging con Printf

```c
// Durante sviluppo
printf("DEBUG: tokens = %p\n", tokens);
printf("DEBUG: cmd->args[0] = %s\n", cmd->args[0]);
```

**IMPORTANTE:** Rimuovi tutti i printf debug prima di consegnare!

---

## 14. DOMANDE FREQUENTI VALUTAZIONE

### Q1: "Quante variabili globali hai?"

**Risposta:** "Una sola: `g_signal` di tipo `volatile sig_atomic_t`. È necessaria perché i signal handler non possono accedere a variabili locali. Il subject permette max 1 variabile globale per questo motivo."

### Q2: "Spiega come funziona il tuo lexer"

**Risposta:** "Il lexer legge l'input carattere per carattere e crea token. Gestisce:
- Spazi (separatori)
- Operatori (|, <, >, <<, >>)
- Quote singole (tutto letterale)
- Quote doppie (espande variabili)
- Parole normali (con espansione variabili)

Ritorna una lista collegata di token."

### Q3: "Come gestisci le pipe?"

**Risposta:** "Per ogni pipe creo una coppia di file descriptor con `pipe()`. Il comando a sinistra scrive su `pipe[1]`, il comando a destra legge da `pipe[0]`. Uso `dup2()` per redirigere stdin/stdout. Ogni comando gira in un processo separato creato con `fork()`."

### Q4: "Perché cd è un built-in?"

**Risposta:** "Perché `cd` deve cambiare la directory della shell stessa. Se fosse un comando esterno, cambierebbe la directory del processo figlio, ma quando il figlio termina, la shell padre rimarrebbe nella directory originale."

### Q5: "Come gestisci i memory leaks?"

**Risposta:** "Ho funzioni di free per ogni struttura allocata:
- `free_tokens()` per i token
- `free_cmd_list()` per i comandi
- `free_envp()` per l'environment
- Ogni `malloc()` ha il suo `free()`
- Testato con valgrind per verificare 0 leaks."

### Q6: "Come funziona l'espansione delle variabili?"

**Risposta:** "Il lexer, quando trova `$`, estrae il nome della variabile e chiama `expand_variables()`. Questa funzione:
1. Cerca la variabile nell'environment
2. Se è `$?`, ritorna l'exit status
3. Altrimenti ritorna il valore dalla variabile d'ambiente
4. Sostituisce `$VAR` con il valore

Nelle quote singole NON espande, nelle doppie SÌ."

---

## 15. CHECKLIST FINALE

### Mandatory

- [x] Prompt funzionante
- [x] History (readline)
- [x] Esecuzione comandi (path relativo/assoluto/da PATH)
- [x] Quote singole (no espansione)
- [x] Quote doppie (con espansione)
- [x] Redirections (<, >, >>, <<)
- [x] Pipe (|)
- [x] Variabili d'ambiente ($VAR)
- [x] Exit status ($?)
- [x] Segnali (Ctrl+C, Ctrl+D, Ctrl+\)
- [x] Built-ins:
  - [x] echo (con -n)
  - [x] cd (relativo/assoluto/HOME/-)
  - [x] pwd
  - [x] export
  - [x] unset
  - [x] env
  - [x] exit

### Norminette

- [ ] Norme 42 rispettate
- [ ] Max 25 linee per funzione
- [ ] Max 5 funzioni per file
- [ ] No variabili globali (tranne g_signal)

### Memory

- [x] No memory leaks
- [x] No segfault
- [x] Ogni malloc ha free

---

## 16. CONCLUSIONE

Questo progetto implementa una **shell funzionante** con:

✅ **Architettura a 4 fasi**: Read → Lexer → Parser → Executor  
✅ **Gestione completa**: Built-ins, variabili, redirections, pipe  

---

**Fine della guida**
