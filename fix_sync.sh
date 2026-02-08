#!/bin/bash

# Gemini Wallet - Manual Sync Fix Script
# Run this inside the Rebble Cloud terminal

echo "--- GEMINI SYNC FIX STARTING ---"

# 1. Clean the environment
rm -rf build
rm -rf resources

# 2. Re-link the GitHub repository
git remote remove origin 2>/dev/null
git remote add origin https://github.com/MitoKafander/PebbleWallet_Gemini.git

# 3. Force download and overwrite EVERYTHING
git fetch origin
git reset --hard origin/main

# 4. Final verification
echo ""
echo "VERIFYING package.json version..."
cat package.json | grep version

echo ""
echo "--- SYNC COMPLETE ---"
echo "If you still see old files in the sidebar, press CMD+R (or F5) to refresh the browser page."
