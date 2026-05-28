#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="$ROOT_DIR/test_data/image_tests"
IMAGE="$TEST_DIR/disk.img"
KEY="secret"

cleanup() {
  rm -rf "$TEST_DIR"
}
trap cleanup EXIT

mkdir -p "$TEST_DIR"

if ! make -s -C "$ROOT_DIR" secure_copy; then
  echo "[FAIL] Build failed"
  exit 1
fi

# Create nested directory structure (depth 4)
mkdir -p "$TEST_DIR/dir1/dir2/dir3/dir4"

printf "Hello root file\n" > "$TEST_DIR/file_root.txt"
printf "Another root file\n" > "$TEST_DIR/file_root_2.txt"
printf "Nested text file\n" > "$TEST_DIR/dir1/dir2/dir3/dir4/nested.txt"

# Binary file
if command -v dd >/dev/null 2>&1; then
  dd if=/dev/urandom of="$TEST_DIR/bin_root.bin" bs=1024 count=4 status=none
else
  head -c 4096 /dev/urandom > "$TEST_DIR/bin_root.bin"
fi

# Add files and directory to image
"$ROOT_DIR/secure_copy" -add -key "$KEY" -image "$IMAGE" \
  "$TEST_DIR/file_root.txt" \
  "$TEST_DIR/file_root_2.txt" \
  "$TEST_DIR/bin_root.bin" \
  "$TEST_DIR/dir1" \
  > "$TEST_DIR/add.log"

if [[ ! -f "$IMAGE" ]]; then
  echo "[FAIL] Image was not created"
  exit 1
fi

# Build expected list (sorted)
name_root_1="$TEST_DIR/file_root.txt"
name_root_2="$TEST_DIR/file_root_2.txt"
name_bin="$TEST_DIR/bin_root.bin"
name_nested="dir1/dir2/dir3/dir4/nested.txt"

size_root_1=$(wc -c < "$TEST_DIR/file_root.txt" | tr -d ' ')
size_root_2=$(wc -c < "$TEST_DIR/file_root_2.txt" | tr -d ' ')
size_bin=$(wc -c < "$TEST_DIR/bin_root.bin" | tr -d ' ')
size_nested=$(wc -c < "$TEST_DIR/dir1/dir2/dir3/dir4/nested.txt" | tr -d ' ')

expected_list="$TEST_DIR/expected_list.txt"
cat > "$expected_list" <<EOF
${name_bin} ${size_bin}
${name_root_1} ${size_root_1}
${name_root_2} ${size_root_2}
${name_nested} ${size_nested}
EOF

# The output must be sorted by name
sort "$expected_list" > "$expected_list.sorted"

"$ROOT_DIR/secure_copy" -list -image "$IMAGE" > "$TEST_DIR/list.log"

sort "$TEST_DIR/list.log" > "$TEST_DIR/list.sorted"

if ! cmp -s "$expected_list.sorted" "$TEST_DIR/list.sorted"; then
  echo "[FAIL] List output does not match expected"
  exit 1
fi

# Extract and verify two files
"$ROOT_DIR/secure_copy" -get -image "$IMAGE" -key "$KEY" \
  -out "$TEST_DIR/out_root.txt" "$name_root_1" > /dev/null

"$ROOT_DIR/secure_copy" -get -image "$IMAGE" -key "$KEY" \
  -out "$TEST_DIR/out_nested.txt" "$name_nested" > /dev/null

"$ROOT_DIR/secure_copy" -get -image "$IMAGE" -key "$KEY" \
  -out "$TEST_DIR/out_bin.bin" "$name_bin" > /dev/null

if ! cmp -s "$TEST_DIR/file_root.txt" "$TEST_DIR/out_root.txt"; then
  echo "[FAIL] Extracted root file mismatch"
  exit 1
fi

if ! cmp -s "$TEST_DIR/dir1/dir2/dir3/dir4/nested.txt" "$TEST_DIR/out_nested.txt"; then
  echo "[FAIL] Extracted nested file mismatch"
  exit 1
fi

if ! cmp -s "$TEST_DIR/bin_root.bin" "$TEST_DIR/out_bin.bin"; then
  echo "[FAIL] Extracted binary file mismatch"
  exit 1
fi

echo "[PASS] Image add/list/get tests passed"
