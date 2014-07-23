#!/usr/bin/env bash

# Make sure we are inside the repository.
cd "${BASH_SOURCE%/*}/.."

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true

echo "Checking basic user information..."
scripts/gitsetup/setup-user
echo

scripts/gitsetup/setup-hooks
echo

echo "Setting up git aliases..."
scripts/setup-git-aliases
echo

setup_version=1
git config hooks.setup ${setup_version}
