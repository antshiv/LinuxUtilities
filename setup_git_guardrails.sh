#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$ROOT_DIR"

if ! command -v git >/dev/null 2>&1; then
    echo "git is required."
    exit 1
fi

mkdir -p "$ROOT_DIR/.githooks"
mkdir -p "$ROOT_DIR/tests"
chmod +x "$ROOT_DIR/.githooks/commit-msg"
chmod +x "$ROOT_DIR/.githooks/pre-commit"
chmod +x "$ROOT_DIR/tests/run_fast.sh"

git config commit.template "$ROOT_DIR/.gitmessage.txt"
git config core.hooksPath "$ROOT_DIR/.githooks"

echo "Git guardrails enabled for this repo:"
echo "  commit.template = $ROOT_DIR/.gitmessage.txt"
echo "  core.hooksPath  = $ROOT_DIR/.githooks"
echo "  pre-commit test = $ROOT_DIR/tests/run_fast.sh"
