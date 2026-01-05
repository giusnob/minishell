# 🧪 GUIDA TEST MANUALE MINISHELL

## ✅ CHECKLIST RAPIDA (15 minuti)

### 1. COMPILAZIONE
```bash
make re
# Verifica: compila senza warning
# ✅ Se OK, passa al prossimo
```

### 2. BASIC COMMANDS (2 min)
```bash
./minishell
echo test                    # Output: test
echo -n test                 # Output: test (no newline)
echo -n -n test              # Output: test (no newline)
pwd                          # Output: percorso corrente
ls                           # Lista file
exit
# ✅ Se tutto OK, continua
```

### 3. CD COMMAND (2 min)
```bash
./minishell
pwd                          # Nota il percorso
cd /tmp
pwd                          # Output: /tmp
cd -
pwd                          # Torna al percorso iniziale
cd
pwd                          # Output: /home/[user]
cd nonexisto                 # Errore: No such file
exit
# ✅ Se tutto OK, continua
```

### 4. EXPORT/UNSET/ENV (3 min)
```bash
./minishell
export TEST=hello
echo $TEST                   # Output: hello
export TEST=world
echo $TEST                   # Output: world
export VUOTA=
echo "x${VUOTA}y"           # Output: xy
unset TEST
echo $TEST                   # Output: (vuoto)
env | grep USER              # Mostra USER
export | grep USER           # Mostra declare -x USER="..."
exit
# ✅ Se tutto OK, continua
```

### 5. QUOTES & EXPANSION (2 min)
```bash
./minishell
echo 'ciao $USER'           # Output: ciao $USER (no expansion)
echo "ciao $USER"           # Output: ciao [tuo username]
echo $USER$PWD              # Output: [username][percorso]
ls
echo $?                     # Output: 0
lsnonexisto
echo $?                     # Output: 127
exit
# ✅ Se tutto OK, continua
```

### 6. REDIRECTIONS (3 min)
```bash
./minishell
echo test > file.txt
cat file.txt                # Output: test
echo append >> file.txt
cat file.txt                # Output: test\nappend
cat < file.txt              # Output: test\nappend
cat < file.txt > out.txt
cat out.txt                 # Output: test\nappend
rm file.txt out.txt
exit
# ✅ Se tutto OK, continua
```

### 7. PIPES (2 min)
```bash
./minishell
echo ciao | cat             # Output: ciao
ls | grep minishell         # Output: minishell
cat Makefile | grep NAME    # Output: linee con NAME
export | grep USER          # Output: USER
exit
# ✅ Se tutto OK, continua
```

### 8. HEREDOC (1 min)
```bash
./minishell
cat << EOF
line 1
line 2
$USER
EOF
# Output: line 1, line 2, [tuo username]
exit
# ✅ Se tutto OK, continua

```

### 9. SEGNALI (2 min)
```bash
./minishell
# Premi Ctrl+C → nuovo prompt (NON esce)
sleep 5
# Premi Ctrl+C → interrompe sleep, nuovo prompt
# Premi Ctrl+D → stampa "exit" e chiude

NO - errore:
./minishell
minishell$ ^C
minishell$ sleep 5
^C
minishell$ minishell$ 
exit
```

### 10. ERROR HANDLING (2 min)
```bash
./minishell
nonexisto                   # Errore: command not found
echo $?                     # Output: 127
cat /root/noperm           # Errore: Permission denied
| cat                      # Errore: syntax error
cat |                      # Errore: syntax error
cat >                      # Errore: syntax error
exit
```

---

## 🎯 EDGE CASES DA TESTARE

### Exit con Vari Argomenti
```bash
./minishell
exit                        # Esce con status dell'ultimo comando
```

```bash
./minishell
exit 42
echo $?                     # Da bash: dovrebbe essere 42
```

```bash
./minishell
exit ciao                   # Errore: numeric argument required
# Poi esce con status 2
```

```bash
./minishell
exit 1 2                    # Errore: too many arguments
# NON esce, torna al prompt!
echo test                   # Dovrebbe ancora funzionare
exit
```

### Export con Identificatori Non Validi
```bash
./minishell
export 1TEST=value          # Dovrebbe dare errore
export TEST-VAR=value       # Dovrebbe dare errore
export =value               # Dovrebbe dare errore
export TEST=value           # Dovrebbe funzionare
exit

NO - Non funziona
./minishell
minishell$ export 1TEST=value
minishell$ export TEST-VAR=value
minishell$ export =value
minishell(23755,0x1fdaa60c0) malloc: *** error for object 0x16ce1ec58: pointer being freed was not allocated
minishell(23755,0x1fdaa60c0) malloc: *** set a breakpoint in malloc_error_break to debug
zsh: abort      ./minishell
giusmerynobile@Mac minischifo % ./minishell
minishell$ export TEST=value
minishell$ exit
exit
```

### Variabili Vuote
```bash
./minishell
export VUOTA=
echo "x${VUOTA}y"          # Output: xy
echo x${VUOTA}y            # Output: xy
exit
```

### Combinazioni Complesse
```bash
./minishell
cat Makefile | grep minishell | wc -l
cat < Makefile > out.txt
cat < out.txt | grep NAME
rm out.txt
exit
```

---

## 📋 CHECKLIST FINALE

Segna ✅ quando completato:

- [ ] Compilazione senza warning
- [ ] echo (con e senza -n)
- [ ] pwd
- [ ] cd (assoluto, relativo, home, -)
- [ ] export/unset/env
- [ ] Quotes (single e double)
- [ ] Espansione variabili ($VAR, $?)
- [ ] Redirections (>, >>, <)
- [ ] Pipes semplici
- [ ] Pipes multiple
- [ ] Heredoc
- [ ] Ctrl+C (al prompt)
- [ ] Ctrl+C (durante comando)
- [ ] Ctrl+D (chiusura)
- [ ] Error handling
- [ ] exit (vari casi)

---

## 🔍 VALGRIND TEST

```bash
# Compila
make re

# Esegui con valgrind
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./minishell

# Dentro minishell, esegui:
echo test
ls
cat Makefile | grep NAME
export A=1
echo $A
unset A
exit

# Controlla output valgrind:
# ✅ "All heap blocks were freed"
# ✅ "ERROR SUMMARY: 0 errors"
# ⚠️  "still reachable" da readline è OK
# ❌ "definitely lost" è PROBLEMA!
```

---

## 📝 APPUNTI PER L'EVALUATION

Mentre testi, prendi nota di:

**Cosa funziona bene:**
- ...
- ...

**Possibili problemi trovati:**
- ...
- ...

**Edge cases interessanti:**
- ...
- ...

---

🌙 *Test with confidence!* 🌙
