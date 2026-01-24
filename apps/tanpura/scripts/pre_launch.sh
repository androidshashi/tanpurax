#!/bin/bash

echo "=========================================="
echo "🚀 Running pre-launch script..."
echo "=========================================="

# Delete .cxx build cache
echo "🗑️  Deleting .cxx cache..."
rm -rf packages/tanpura_engine/android/.cxx


echo "Refresing pub dependencies..."
cd apps/tanpura && fvm flutter clean && fvm flutter pub get

echo "✅ Pre-launch cleanup completed"
echo "=========================================="
