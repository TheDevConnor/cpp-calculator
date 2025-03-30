#!/usr/bin/env bash

# Directories
DEBUG_DIR="debug"
RELEASE_DIR="release"

# Global variables
BUILD_TYPE=""
DEBUG_TYPE=""

die() {
  exit 1
}

# Build functions (combined)
build() {
  type="$1"
  BUILD_TYPE="$type"
  cmake -S . -B "$type" -DCMAKE_BUILD_TYPE="$type" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 || die
  cmake --build "$type"
}

# Clean function
clean() {
  rm -rf "$DEBUG_DIR" "$RELEASE_DIR" "valgrind-out.txt" || die
}

run() {
  executable="./$BUILD_TYPE/calculator"
  echo "Executable: $executable" || die
  if [ ! -f "$executable" ]; then
    echo "Executable not found in $BUILD_TYPE build." || die
    return 1
  fi
  # "$executable" build zura_files/main.zu -save -debug -name main|| die
  echo "Running..." || die
  if [ "$DEBUG_TYPE" == "debug" ]; then
    "$executable" || die
  else
    "$executable" || die
  fi
}

# make the commands be able to be sequenced
for cmd in "$@"; do
  case "$cmd" in
    debug|release)
      build "$cmd" || die
      if [ "$cmd" == "debug" ]; then
        DEBUG_TYPE="debug"
      fi
      ;;
    val)
      valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./"$BUILD_TYPE"/calculator || die
      ;;
    clean)
      clean
      ;;
    run)
      run "$2" || die
      ;;
    *)
      echo "Usage: $0 {debug|release|val|clean|run}" || die
      ;;
  esac
done
