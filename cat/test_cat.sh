#!/bin/bash

# Цвета для вывода результатов
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0;0m'

SUCCESS_COUNT=0
FAIL_COUNT=0
TEST_NUMBER=0

# Создание тестовых файлов, если они не существуют
echo -e "Line 1\n\nLine 3 with tab:\t[tab]\n\n\nLine 6 with non-printable:\x01\x02\x03\nLine 7" > test_1.txt
echo -e "Another file\nWith multiple blank lines\n\n\n\nEnd of file" > test_2.txt
echo -e "" > test_empty.txt

# Массив тестируемых файлов
FILES="test_1.txt test_2.txt test_empty.txt"

# Функция для запуска одного теста
run_test() {
    local flags="$1"
    TEST_NUMBER=$((TEST_NUMBER + 1))

    # Запуск оригинального cat и нашей версии s21_cat
    cat $flags $FILES > res_orig.log 2> err_orig.log
    ./s21_cat $flags $FILES > res_s21.log 2> err_s21.log

    # Сравнение стандартного вывода (stdout) и потока ошибок (stderr)
    diff res_orig.log res_s21.log > /dev/null
    local diff_out=$?

    diff err_orig.log err_s21.log > /dev/null
    local diff_err=$?

    if [ $diff_out -eq 0 ] && [ $diff_err -eq 0 ]; then
        echo -e "Test #${TEST_NUMBER} [cat ${flags}]: ${GREEN}SUCCESS${NC}"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "Test #${TEST_NUMBER} [cat ${flags}]: ${RED}FAIL${NC}"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        # Раскомментируйте строку ниже, если хотите видеть разницу при падении теста:
        # diff -u res_orig.log res_s21.log
    fi
}

# 1. Тест без флагов
run_test ""

# 2. Одиночные короткие флаги
for flag in "-b" "-e" "-E" "-n" "-s" "-t" "-T" "-v"; do
    run_test "$flag"
done

# 3. Длинные GNU-флаги
for flag in "--number-nonblank" "--number" "--squeeze-blank"; do
    run_test "$flag"
done

# 4. Парные комбинации коротких флагов
for f1 in "-b" "-e" "-n" "-s" "-t" "-v"; do
    for f2 in "-b" "-e" "-n" "-s" "-t" "-v"; do
        if [ "$f1" != "$f2" ]; then
            run_test "$f1 $f2"
        fi
    done
done

# 5. Тройные комбинации флагов
run_test "-b -e -s"
run_test "-n -e -t"
run_test "-s -t -v"
run_test "-b -t -s"

# 6. Тест на несуществующий файл (проверка stderr)
TEST_NUMBER=$((TEST_NUMBER + 1))
cat non_existent_file.txt > res_orig.log 2> err_orig.log
./s21_cat non_existent_file.txt > res_s21.log 2> err_s21.log
# Проверяем только факт наличия ошибки в stderr (так как текст может незначительно отличаться)
if [ -s err_orig.log ] && [ -s err_s21.log ]; then
    echo -e "Test #${TEST_NUMBER} [Non-existent file]: ${GREEN}SUCCESS${NC}"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "Test #${TEST_NUMBER} [Non-existent file]: ${RED}FAIL${NC}"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi

# Очистка временных файлов
rm -f test_1.txt test_2.txt test_empty.txt res_orig.log res_s21.log err_orig.log err_s21.log
