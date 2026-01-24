#!/bin/bash

echo "=========================================="
echo "🚀 Running pre-launch script..."
echo "=========================================="

# Delete .cxx build cache
echo "🗑️  Deleting .cxx cache..."
rm -rf packages/tanpura_engine/android/.cxx

echo "✅ Pre-launch cleanup completed"
echo "=========================================="
