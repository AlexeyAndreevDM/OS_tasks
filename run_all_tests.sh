#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

TEST_DIR="$ROOT_DIR/test_data"
REPORT="$TEST_DIR/REPORT.txt"

pass_count=0
fail_count=0

timestamp() {
  date '+%Y-%m-%d %H:%M:%S'
}

pass() {
  echo "[PASS] $1"
  pass_count=$((pass_count + 1))
}

fail() {
  echo "[FAIL] $1"
  fail_count=$((fail_count + 1))
}

check_contains() {
  local file="$1"
  local pattern="$2"
  local label="$3"
  if grep -q "$pattern" "$file"; then
    pass "$label"
  else
    fail "$label"
  fi
}

echo "=== secure_copy: полный прогон тестов (часть 4) ==="
echo "Старт: $(timestamp)"

mkdir -p "$TEST_DIR"
rm -f "$TEST_DIR"/*

# 1) Генерация тестовых входов (10 файлов)
for i in $(seq 1 8); do
  file_path="$TEST_DIR/in_text_$i.txt"
  : > "$file_path"
  lines=$((2000 + i * 200))
  for _ in $(seq 1 "$lines"); do
    echo "File $i line" >> "$file_path"
  done
done

dd if=/dev/urandom of="$TEST_DIR/in_bin_9.bin" bs=1024 count=1024 status=none
dd if=/dev/urandom of="$TEST_DIR/in_bin_10.bin" bs=1024 count=2048 status=none

inputs_count=$(ls "$TEST_DIR"/in_* | wc -l | tr -d ' ')
if [[ "$inputs_count" == "10" ]]; then
  pass "Создано 10 входных файлов"
else
  fail "Создано 10 входных файлов"
fi

# 2) Сборка
if make clean >/dev/null 2>&1 && make secure_copy >/dev/null 2>&1; then
  pass "Сборка secure_copy успешна"
else
  fail "Сборка secure_copy успешна"
fi

# 3) Sequential
./secure_copy --mode=sequential X \
  "$TEST_DIR/in_text_1.txt" "$TEST_DIR/out_seq_1.enc" \
  "$TEST_DIR/in_text_2.txt" "$TEST_DIR/out_seq_2.enc" \
  > "$TEST_DIR/run_seq.log"

check_contains "$TEST_DIR/run_seq.log" "Режим: sequential" "Режим sequential выводится"
check_contains "$TEST_DIR/run_seq.log" "Успешно: 2" "Sequential обработал 2/2"
check_contains "$TEST_DIR/run_seq.log" "Общее время:" "Sequential выводит общее время"
check_contains "$TEST_DIR/run_seq.log" "Среднее время на файл:" "Sequential выводит среднее время"

# 4) Parallel
./secure_copy --mode=parallel X \
  "$TEST_DIR/in_text_1.txt" "$TEST_DIR/out_par_1.enc" \
  "$TEST_DIR/in_text_2.txt" "$TEST_DIR/out_par_2.enc" \
  > "$TEST_DIR/run_par.log"

check_contains "$TEST_DIR/run_par.log" "Режим: parallel" "Режим parallel выводится"
check_contains "$TEST_DIR/run_par.log" "Успешно: 2" "Parallel обработал 2/2"
check_contains "$TEST_DIR/run_par.log" "Общее время:" "Parallel выводит общее время"
check_contains "$TEST_DIR/run_par.log" "Среднее время на файл:" "Parallel выводит среднее время"

# 5) Auto (малый набор < 5, должен выбрать sequential)
./secure_copy --mode=auto X \
  "$TEST_DIR/in_text_1.txt" "$TEST_DIR/out_auto_small_1.enc" \
  "$TEST_DIR/in_text_2.txt" "$TEST_DIR/out_auto_small_2.enc" \
  "$TEST_DIR/in_text_3.txt" "$TEST_DIR/out_auto_small_3.enc" \
  "$TEST_DIR/in_text_4.txt" "$TEST_DIR/out_auto_small_4.enc" \
  > "$TEST_DIR/run_auto_small.log"

check_contains "$TEST_DIR/run_auto_small.log" "Режим: sequential" "Auto(<5) выбрал sequential"
check_contains "$TEST_DIR/run_auto_small.log" "Сравнение режимов:" "Auto(<5) выводит сравнение режимов"

# 6) Auto (большой набор >= 5, должен выбрать parallel)
./secure_copy --mode=auto X \
  "$TEST_DIR/in_text_1.txt" "$TEST_DIR/out_auto_1.enc" \
  "$TEST_DIR/in_text_2.txt" "$TEST_DIR/out_auto_2.enc" \
  "$TEST_DIR/in_text_3.txt" "$TEST_DIR/out_auto_3.enc" \
  "$TEST_DIR/in_text_4.txt" "$TEST_DIR/out_auto_4.enc" \
  "$TEST_DIR/in_text_5.txt" "$TEST_DIR/out_auto_5.enc" \
  "$TEST_DIR/in_text_6.txt" "$TEST_DIR/out_auto_6.enc" \
  "$TEST_DIR/in_text_7.txt" "$TEST_DIR/out_auto_7.enc" \
  "$TEST_DIR/in_text_8.txt" "$TEST_DIR/out_auto_8.enc" \
  "$TEST_DIR/in_bin_9.bin" "$TEST_DIR/out_auto_9.enc" \
  "$TEST_DIR/in_bin_10.bin" "$TEST_DIR/out_auto_10.enc" \
  > "$TEST_DIR/run_auto.log"

check_contains "$TEST_DIR/run_auto.log" "Режим: parallel" "Auto(>=5) выбрал parallel"
check_contains "$TEST_DIR/run_auto.log" "Файлов всего: 10" "Auto(>=5) обработал 10 файлов"
check_contains "$TEST_DIR/run_auto.log" "Сравнение режимов:" "Auto(>=5) выводит сравнение режимов"

# 7) XOR обратимость для бинарных файлов
./secure_copy --mode=sequential X \
  "$TEST_DIR/out_auto_9.enc" "$TEST_DIR/dec_auto_9.bin" \
  "$TEST_DIR/out_auto_10.enc" "$TEST_DIR/dec_auto_10.bin" \
  > /dev/null

if cmp -s "$TEST_DIR/in_bin_9.bin" "$TEST_DIR/dec_auto_9.bin"; then
  pass "Бинарный файл #9 восстановлен корректно"
else
  fail "Бинарный файл #9 восстановлен корректно"
fi

if cmp -s "$TEST_DIR/in_bin_10.bin" "$TEST_DIR/dec_auto_10.bin"; then
  pass "Бинарный файл #10 восстановлен корректно"
else
  fail "Бинарный файл #10 восстановлен корректно"
fi

# 8) Проверка лог-файла
if [[ -f "$ROOT_DIR/secure_copy.log" ]]; then
  pass "Файл secure_copy.log создается"
else
  fail "Файл secure_copy.log создается"
fi

{
  echo "=== REPORT: secure_copy part4 ==="
  echo "Дата: $(timestamp)"
  echo "PASS: $pass_count"
  echo "FAIL: $fail_count"
  echo
  echo "--- run_seq.log ---"
  cat "$TEST_DIR/run_seq.log"
  echo
  echo "--- run_par.log ---"
  cat "$TEST_DIR/run_par.log"
  echo
  echo "--- run_auto_small.log ---"
  cat "$TEST_DIR/run_auto_small.log"
  echo
  echo "--- run_auto.log ---"
  cat "$TEST_DIR/run_auto.log"
  echo
  echo "--- tail secure_copy.log ---"
  tail -n 20 "$ROOT_DIR/secure_copy.log" || true
} > "$REPORT"

echo
echo "Итог: PASS=$pass_count FAIL=$fail_count"
echo "Отчет: $REPORT"

if [[ "$fail_count" -eq 0 ]]; then
  echo "РЕЗУЛЬТАТ: УСПЕШНО"
  exit 0
fi

echo "РЕЗУЛЬТАТ: ЕСТЬ ПРОБЛЕМЫ"
exit 1
