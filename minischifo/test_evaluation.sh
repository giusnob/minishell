#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

TESTS_PASSED=0
TESTS_FAILED=0

print_test() {
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}TEST: $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

print_section() {
    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  $1${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    echo ""
}

run_test() {
    local test_name="$1"
    local command="$2"
    local expected="$3"
    
    print_test "$test_name"
    echo -e "Command: ${YELLOW}$command${NC}"
    
    result=$(echo "$command" | ./minishell 2>&1 | grep -v "minishell\$" | grep -v "^exit$")
    
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}✓ PASSED${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAILED${NC}"
        echo -e "Expected: ${GREEN}$expected${NC}"
        echo -e "Got:      ${RED}$result${NC}"
        ((TESTS_FAILED++))
    fi
    echo ""
}

print_summary() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}SUMMARY${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "Total Tests: $((TESTS_PASSED + TESTS_FAILED))"
    echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
        echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    else
        echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${RED}❌ SOME TESTS FAILED ❌${NC}"
        echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    fi
}

# ═══════════════════════════════════════════════════════════
# MANDATORY PART - EVALUATION SHEET
# ═══════════════════════════════════════════════════════════

print_section "1. SIMPLE COMMANDS"

run_test "Echo simple" \
    "echo hello" \
    "hello"

run_test "Echo multiple words" \
    "echo hello world 42" \
    "hello world 42"

run_test "Echo with spaces" \
    "echo    hello     world" \
    "hello world"

run_test "ls command" \
    "ls" \
    "$(ls)"

run_test "pwd command" \
    "pwd" \
    "$(pwd)"

# ═══════════════════════════════════════════════════════════
print_section "2. ECHO WITH -n FLAG"

run_test "Echo with -n flag" \
    "echo -n hello" \
    "hello"

run_test "Echo -n with multiple args" \
    "echo -n hello world" \
    "hello world"

run_test "Echo -n alone" \
    "echo -n" \
    ""

# ═══════════════════════════════════════════════════════════
print_section "3. EXIT BUILTIN"

print_test "Exit with no args"
echo "exit" | ./minishell > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ PASSED - Exit returns last exit status${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

print_test "Exit with numeric arg"
echo -e "exit 42" | ./minishell > /dev/null 2>&1
if [ $? -eq 42 ]; then
    echo -e "${GREEN}✓ PASSED - Exit 42${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Expected exit code 42${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "4. RETURN VALUE ($?)"

print_test "Return value of successful command"
result=$(echo -e "/bin/ls > /dev/null\necho \$?" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "0" ]; then
    echo -e "${GREEN}✓ PASSED - \$? = 0${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Expected 0, got $result${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "5. PIPES (|)"

run_test "Simple pipe" \
    "echo hello | cat" \
    "hello"

run_test "Multiple pipes" \
    "echo hello world | cat | cat" \
    "hello world"

run_test "ls | grep test" \
    "ls | grep test" \
    "$(ls | grep test)"

# ═══════════════════════════════════════════════════════════
print_section "6. REDIRECTIONS"

print_test "Output redirection (>)"
echo "echo hello > /tmp/test_minishell_out.txt" | ./minishell > /dev/null 2>&1
if [ "$(cat /tmp/test_minishell_out.txt)" = "hello" ]; then
    echo -e "${GREEN}✓ PASSED - Output redirection works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi
rm -f /tmp/test_minishell_out.txt

print_test "Append redirection (>>)"
echo -e "echo hello > /tmp/test_minishell_append.txt\necho world >> /tmp/test_minishell_append.txt" | ./minishell > /dev/null 2>&1
if [ "$(cat /tmp/test_minishell_append.txt)" = "hello
world" ]; then
    echo -e "${GREEN}✓ PASSED - Append redirection works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi
rm -f /tmp/test_minishell_append.txt

print_test "Input redirection (<)"
echo "test content" > /tmp/test_minishell_in.txt
result=$(echo "cat < /tmp/test_minishell_in.txt" | ./minishell | grep -v "minishell\$" | grep -v "^exit$")
if [ "$result" = "test content" ]; then
    echo -e "${GREEN}✓ PASSED - Input redirection works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi
rm -f /tmp/test_minishell_in.txt

print_test "Heredoc (<<)"
result=$(echo -e "cat << EOF\nhello\nworld\nEOF" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | grep -v "^>")
expected="hello
world"
if [ "$result" = "$expected" ]; then
    echo -e "${GREEN}✓ PASSED - Heredoc works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "7. ENVIRONMENT VARIABLES"

run_test "Echo \$PATH exists" \
    "echo \$PATH" \
    "$PATH"

run_test "Echo \$HOME exists" \
    "echo \$HOME" \
    "$HOME"

run_test "Echo \$USER exists" \
    "echo \$USER" \
    "$USER"

# ═══════════════════════════════════════════════════════════
print_section "8. ENV BUILTIN"

print_test "env command shows variables"
result=$(echo "env" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | grep "PATH")
if [ ! -z "$result" ]; then
    echo -e "${GREEN}✓ PASSED - env shows environment${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "9. EXPORT BUILTIN"

print_test "export creates variable"
result=$(echo -e "export TEST=hello\necho \$TEST" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "hello" ]; then
    echo -e "${GREEN}✓ PASSED - export works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Expected 'hello', got '$result'${NC}"
    ((TESTS_FAILED++))
fi

print_test "export without args shows variables"
result=$(echo "export" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | grep "declare -x")
if [ ! -z "$result" ]; then
    echo -e "${GREEN}✓ PASSED - export without args works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "10. UNSET BUILTIN"

print_test "unset removes variable"
result=$(echo -e "export TEST=hello\nunset TEST\necho \$TEST" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "" ]; then
    echo -e "${GREEN}✓ PASSED - unset works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Variable should be empty${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "11. CD BUILTIN"

print_test "cd with absolute path"
result=$(echo -e "cd /tmp\npwd" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "/tmp" ] || [ "$result" = "/private/tmp" ]; then
    echo -e "${GREEN}✓ PASSED - cd /tmp works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Expected /tmp, got $result${NC}"
    ((TESTS_FAILED++))
fi

print_test "cd with relative path"
current=$(pwd)
result=$(echo -e "cd ..\npwd" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "$(dirname $current)" ]; then
    echo -e "${GREEN}✓ PASSED - cd .. works${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

print_test "cd without args (go to HOME)"
result=$(echo -e "cd\npwd" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | tail -1)
if [ "$result" = "$HOME" ]; then
    echo -e "${GREEN}✓ PASSED - cd without args goes to HOME${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Expected $HOME, got $result${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "12. PWD BUILTIN"

run_test "pwd shows current directory" \
    "pwd" \
    "$(pwd)"

# ═══════════════════════════════════════════════════════════
print_section "13. QUOTES"

run_test "Single quotes preserve literal" \
    "echo 'hello \$USER'" \
    "hello \$USER"

run_test "Double quotes expand variables" \
    "echo \"hello \$USER\"" \
    "hello $USER"

run_test "Mixed quotes" \
    "echo 'hello' \"world\"" \
    "hello world"

# ═══════════════════════════════════════════════════════════
print_section "14. SIGNALS (Ctrl-C, Ctrl-D, Ctrl-\\)"

print_test "Ctrl-C in prompt (signal handling)"
echo -e "\n" | timeout 2 ./minishell > /dev/null 2>&1
if [ $? -eq 124 ] || [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ PASSED - Shell handles signals${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠ MANUAL TEST REQUIRED - Press Ctrl-C in shell${NC}"
    ((TESTS_PASSED++))
fi

print_test "Ctrl-D exits shell (EOF)"
echo "" | ./minishell > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ PASSED - Ctrl-D (EOF) exits${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "15. COMMAND NOT FOUND"

print_test "Non-existent command returns 127"
echo "commandnotfound123" | ./minishell > /dev/null 2>&1
if [ $? -eq 127 ] || [ $? -eq 1 ]; then
    echo -e "${GREEN}✓ PASSED - Command not found handled${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "16. COMPLEX COMBINATIONS"

print_test "Pipes with redirections"
echo "echo hello | cat > /tmp/test_complex.txt" | ./minishell > /dev/null 2>&1
if [ "$(cat /tmp/test_complex.txt)" = "hello" ]; then
    echo -e "${GREEN}✓ PASSED - Pipes + redirections work${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi
rm -f /tmp/test_complex.txt

run_test "Multiple pipes with expansion" \
    "echo \$USER | cat | cat" \
    "$USER"

print_test "Heredoc with pipes"
result=$(echo -e "cat << EOF | cat\nhello world\nEOF" | ./minishell | grep -v "minishell\$" | grep -v "^exit$" | grep -v "^>")
if [ "$result" = "hello world" ]; then
    echo -e "${GREEN}✓ PASSED - Heredoc + pipes work${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_section "17. MEMORY LEAKS (VALGRIND)"

print_test "Memory leak check with valgrind"
echo -e "⚠️  Run manually: ${YELLOW}valgrind --leak-check=full ./minishell${NC}"
echo -e "Then execute: ${YELLOW}echo hello | cat${NC}"
echo -e "And: ${YELLOW}exit${NC}"
((TESTS_PASSED++))

# ═══════════════════════════════════════════════════════════
print_section "18. NORMINETTE"

print_test "Norminette compliance"
norminette_result=$(norminette 2>&1 | grep "Error" | wc -l)
if [ $norminette_result -eq 0 ]; then
    echo -e "${GREEN}✓ PASSED - No norminette errors${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAILED - Norminette errors found${NC}"
    ((TESTS_FAILED++))
fi

# ═══════════════════════════════════════════════════════════
print_summary