#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

GREP="./s21_grep"
ORIGINAL="grep"
PATTERN="hello"
TEST_FILE="test_data.txt"

MY_OUT=$(mktemp)
MY_ERR=$(mktemp)
ORIG_OUT=$(mktemp)
ORIG_ERR=$(mktemp)

cleanup() {
    rm -f "$MY_OUT" "$MY_ERR" "$ORIG_OUT" "$ORIG_ERR" "$TEST_FILE"
}
trap cleanup EXIT

# Подготовка тестового файла
cat > "$TEST_FILE" << 'EOF'
Hello world
HELLO WORLD
hello world
goodbye world
Hello everyone
123 hello 456
This is a test
Hello hello hello
World of warcraft

HELLO
xyz
EOF

# Сравнение с системным grep
# Нормализуем имя программы в stderr, чтобы различались только настоящие ошибки
run_test() {
    local name="$1"
    shift
    local args=("$@")

    ((TOTAL_TESTS++))

    "$GREP" "${args[@]}" > "$MY_OUT" 2> "$MY_ERR"
    local my_code=$?
    "$ORIGINAL" "${args[@]}" > "$ORIG_OUT" 2> "$ORIG_ERR"
    local orig_code=$?

    sed -i 's/^s21_grep:/PROG:/' "$MY_ERR"
    sed -i 's/^grep:/PROG:/'     "$ORIG_ERR"

    if diff -q "$MY_OUT" "$ORIG_OUT" > /dev/null && \
       diff -q "$MY_ERR" "$ORIG_ERR" > /dev/null && \
       [ "$my_code" -eq "$orig_code" ]; then
        echo -e "${GREEN}✓ PASSED${NC}  [$name] (exit=$my_code)"
        ((PASSED_TESTS++))
    else
        echo -e "${RED}✗ FAILED${NC}  [$name]"
        echo -e "  ${CYAN}Args:${NC} ${args[*]}"
        echo -e "  ${CYAN}Exit:${NC} my=$my_code orig=$orig_code"
        if ! diff -q "$MY_OUT" "$ORIG_OUT" > /dev/null; then
            echo -e "  ${CYAN}stdout diff:${NC}"
            diff "$MY_OUT" "$ORIG_OUT" | sed 's/^/    /'
        fi
        if ! diff -q "$MY_ERR" "$ORIG_ERR" > /dev/null; then
            echo -e "  ${CYAN}stderr diff:${NC}"
            diff "$MY_ERR" "$ORIG_ERR" | sed 's/^/    /'
        fi
        ((FAILED_TESTS++))
    fi
}

# Проверка наличия необходимых файлов
if [ ! -f "$GREP" ]; then
    echo -e "${RED}Ошибка: $GREP не найден. Соберите проект (make).${NC}"
    exit 1
fi

echo "========================================"
echo "  ПОЛНОЕ ТЕСТИРОВАНИЕ S21_GREP"
echo "========================================"
echo ""

# 1. Без флагов
echo -e "${MAGENTA}═══ 1. БАЗОВЫЕ ТЕСТЫ (без флагов) ═══${NC}"
run_test "Без флагов"                        "$PATTERN"  "$TEST_FILE"
run_test "Нет совпадений"                    "xyz"       "$TEST_FILE"
run_test "Регекс ^"                          "^hello"    "$TEST_FILE"
run_test "Регекс $"                          "world$"    "$TEST_FILE"
run_test "Регекс класс"                      "[Hh]ello"  "$TEST_FILE"
echo ""

# 2. Одиночные флаги
echo -e "${MAGENTA}═══ 2. ОДИНОЧНЫЕ ФЛАГИ ═══${NC}"
for f in i v c l n; do
    run_test "Флаг -$f"  "-$f" "$PATTERN" "$TEST_FILE"
done
run_test "Флаг -e"  -e "$PATTERN" "$TEST_FILE"
echo ""

# 3. Пары флагов (без -e)
echo -e "${MAGENTA}═══ 3. ПАРЫ ФЛАГОВ ═══${NC}"
FLAGS=(i v c l n)
for ((a=0; a<${#FLAGS[@]}; a++)); do
    for ((b=a+1; b<${#FLAGS[@]}; b++)); do
        f1=${FLAGS[$a]}
        f2=${FLAGS[$b]}
        run_test "-$f1 -$f2"  "-$f1" "-$f2" "$PATTERN" "$TEST_FILE"
    done
done
echo ""

# 4. Тройки флагов
echo -e "${MAGENTA}═══ 4. ТРОЙКИ ФЛАГОВ ═══${NC}"
for ((a=0; a<${#FLAGS[@]}; a++)); do
    for ((b=a+1; b<${#FLAGS[@]}; b++)); do
        for ((c=b+1; c<${#FLAGS[@]}; c++)); do
            f1=${FLAGS[$a]}
            f2=${FLAGS[$b]}
            f3=${FLAGS[$c]}
            run_test "-$f1 -$f2 -$f3"  "-$f1" "-$f2" "-$f3" "$PATTERN" "$TEST_FILE"
        done
    done
done
echo ""

# 5. Четвёрки флагов
echo -e "${MAGENTA}═══ 5. ЧЕТВЁРКИ ФЛАГОВ ═══${NC}"
for ((a=0; a<${#FLAGS[@]}; a++)); do
    for ((b=a+1; b<${#FLAGS[@]}; b++)); do
        for ((c=b+1; c<${#FLAGS[@]}; c++)); do
            for ((d=c+1; d<${#FLAGS[@]}; d++)); do
                f1=${FLAGS[$a]}
                f2=${FLAGS[$b]}
                f3=${FLAGS[$c]}
                f4=${FLAGS[$d]}
                run_test "-$f1 -$f2 -$f3 -$f4"  \
                    "-$f1" "-$f2" "-$f3" "-$f4" "$PATTERN" "$TEST_FILE"
            done
        done
    done
done
echo ""

# 6. Все пять флагов
echo -e "${MAGENTA}═══ 6. ВСЕ ФЛАГИ ═══${NC}"
run_test "Все флаги"  -i -v -c -l -n "$PATTERN" "$TEST_FILE"
echo ""

# 7. Комбинации с -e
echo -e "${MAGENTA}═══ 7. КОМБИНАЦИИ С -e ═══${NC}"
run_test "-e + -i"      -e "$PATTERN" -i "$TEST_FILE"
run_test "-e + -v"      -e "$PATTERN" -v "$TEST_FILE"
run_test "-e + -c"      -e "$PATTERN" -c "$TEST_FILE"
run_test "-e + -l"      -e "$PATTERN" -l "$TEST_FILE"
run_test "-e + -n"      -e "$PATTERN" -n "$TEST_FILE"
run_test "-e + -i -c"   -e "$PATTERN" -i -c "$TEST_FILE"
run_test "-e + -i -n"   -e "$PATTERN" -i -n "$TEST_FILE"
run_test "-e + -v -n"   -e "$PATTERN" -v -n "$TEST_FILE"
run_test "-e + -i -v"   -e "$PATTERN" -i -v "$TEST_FILE"
run_test "-e + -c -n"   -e "$PATTERN" -c -n "$TEST_FILE"
run_test "-e + -l -n"   -e "$PATTERN" -l -n "$TEST_FILE"
run_test "-e + -i -c -n" -e "$PATTERN" -i -c -n "$TEST_FILE"
run_test "-e + -i -v -n" -e "$PATTERN" -i -v -n "$TEST_FILE"

# Две -e (несколько шаблонов)
run_test "Два -e"        -e "$PATTERN" -e "world" "$TEST_FILE"
run_test "Два -e + -i"   -e "HELLO"    -e "WORLD" -i "$TEST_FILE"
run_test "Два -e + -c"   -e "hello"    -e "xyz"   -c "$TEST_FILE"
run_test "Два -e + -n"   -e "hello"    -e "test"  -n "$TEST_FILE"
run_test "Два -e + -v"   -e "hello"    -e "world" -v "$TEST_FILE"
run_test "Два -e + -v -n" -e "hello"   -e "world" -v -n "$TEST_FILE"
echo ""

# 8. Несколько файлов
echo -e "${MAGENTA}═══ 8. НЕСКОЛЬКО ФАЙЛОВ ═══${NC}"
cp "$TEST_FILE" test_data2.txt
run_test "Два файла"            "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -n"       -n "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -c"       -c "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -l"       -l "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -i"       -i "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -i -n"    -i -n "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла + -v"       -v "$PATTERN" "$TEST_FILE" test_data2.txt
run_test "Два файла без совпадений"  "xyz" "$TEST_FILE" test_data2.txt
rm -f test_data2.txt
echo ""

# 9. Повторяющиеся флаги
echo -e "${MAGENTA}═══ 9. ПОВТОРЯЮЩИЕСЯ ФЛАГИ ═══${NC}"
run_test "-i -i"     -i -i "$PATTERN" "$TEST_FILE"
run_test "-v -v"     -v -v "$PATTERN" "$TEST_FILE"
run_test "-c -c"     -c -c "$PATTERN" "$TEST_FILE"
run_test "-n -n"     -n -n "$PATTERN" "$TEST_FILE"
run_test "-i -n -i"  -i -n -i "$PATTERN" "$TEST_FILE"
run_test "-c -i -c"  -c -i -c "$PATTERN" "$TEST_FILE"
echo ""

# 10. Смешанные формы записи
echo -e "${MAGENTA}═══ 10. РАЗНЫЕ ФОРМЫ ЗАПИСИ ═══${NC}"
run_test "Раздельные флаги"  -i -n "$PATTERN" "$TEST_FILE"
run_test "Слитные флаги"     -in "$PATTERN"  "$TEST_FILE"
run_test "Слитные -ivn"      -ivn "$PATTERN" "$TEST_FILE"
run_test "Слитные -cv"       -cv "$PATTERN"  "$TEST_FILE"
run_test "Слитные -ln"       -ln "$PATTERN"  "$TEST_FILE"
echo ""

# 11. Крайние случаи
echo -e "${MAGENTA}═══ 11. КРАЙНИЕ СЛУЧАИ ═══${NC}"
run_test "Пустой паттерн"      ""           "$TEST_FILE"
run_test "Паттерн-точка"       "."          "$TEST_FILE"
run_test "Регекс с +"          "[0-9]+"     "$TEST_FILE"
run_test "Регекс со *"         "hel*"       "$TEST_FILE"
run_test "Два -e + -i -v -n"   -e "hello" -e "xyz" -i -v -n "$TEST_FILE"
run_test "Всё вместе"          -e "$PATTERN" -e "world" -i -v -c -n "$TEST_FILE"
echo ""

echo "========================================"
echo "  РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ"
echo "========================================"
echo -e "Всего тестов: ${YELLOW}$TOTAL_TESTS${NC}"
echo -e "Успешно:      ${GREEN}$PASSED_TESTS${NC}"
echo -e "Провалено:    ${RED}$FAILED_TESTS${NC}"
echo "========================================"

if [ $TOTAL_TESTS -gt 0 ]; then
    PERCENT=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    echo -e "Процент успешности: ${YELLOW}${PERCENT}%${NC}"
fi
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}✅ ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!${NC}"
    exit 0
else
    echo -e "${RED}❌ ЕСТЬ ПРОВАЛЕННЫЕ ТЕСТЫ!${NC}"
    exit 1
fi