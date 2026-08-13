#!/bin/bash
# Launches Jubin Mitra's 8085 Simulator.
# Double-click this file in Finder, or run it from a terminal.
# The simulator's own File > Save Assembly language code dialog decides where
# .asm files land, so start it from the repo root to get a sensible default.

cd "$(dirname "$0")/.." || exit 1

if ! command -v java >/dev/null 2>&1; then
  echo "Java not found. Install a JDK (e.g. 'brew install --cask temurin') and retry."
  read -r -p "Press Return to close."
  exit 1
fi

exec java -jar "tools/8085Compiler.jar"
