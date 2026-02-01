# Fastlane Setup Guide for Tanpura Project

## Overview

Fastlane is an open-source platform that automates beta deployments and releases for iOS and Android apps. In this project, we use Fastlane to automate the Flutter APK build process for different flavors (dev, staging, prod) and modes (debug, release).

## Prerequisites

Before setting up Fastlane, ensure you have:

- **Ruby** (version 2.5.0 or higher) installed
- **Ruby Gems** (package manager for Ruby)
- **Flutter** installed and configured
- **Android SDK** installed and configured
- **Gradle** (comes with Android SDK)
- **Git** with submodules initialized

## Installation Steps

### 1. Install Ruby

If you don't have Ruby installed:

```bash
# macOS (using Homebrew)
brew install ruby

# Or using rbenv for version management
brew install rbenv
rbenv install 2.7.0
rbenv global 2.7.0
```

### 2. Install Fastlane

Navigate to your Android project directory and install Fastlane:

```bash
cd apps/tanpura/android

# Install Fastlane via Gem
sudo gem install fastlane -NV

# Or use Bundler (recommended)
gem install bundler
bundle install
```

### 3. Initialize Fastlane (if not already done)

```bash
fastlane init
```

This creates the `fastlane/` directory with configuration files:

- `Fastfile` - Contains lane definitions
- `Appfile` - Contains app configuration

## Project Configuration

### Gemfile

The `Gemfile` in `apps/tanpura/android/` specifies dependencies:

```ruby
source "https://rubygems.org"
gem "fastlane"
```

To install gems specified in Gemfile:

```bash
bundle install
```

### Fastfile

Our `Fastfile` contains a custom lane for building Flutter APKs:

```ruby
default_platform(:android)

platform :android do
  desc "Build Flutter APK with flavor and mode"
  lane :flutter_build do |options|
    flavor = options[:flavor] || "dev"
    mode   = options[:mode]   || "debug"
    target = "lib/main_#{flavor}.dart"

    UI.message("🚀 Building flavor=#{flavor}, mode=#{mode}")

    sh("flutter --version")
    sh("flutter pub get")
    sh("flutter build apk --#{mode} --flavor #{flavor} -t #{target}")
  end
end
```

### Appfile

The `Appfile` contains package configuration:

```ruby
json_key_file("") # Path to JSON secret file for Play Store
package_name("com.tanpurax.tanpura")
```

## Running Fastlane Builds

### Build with Flutter

From the `apps/tanpura/android/` directory:

```bash
# Build dev flavor in debug mode
fastlane flutter_build flavor:dev mode:debug

# Build staging flavor in release mode
fastlane flutter_build flavor:staging mode:release

# Build prod flavor in release mode
fastlane flutter_build flavor:prod mode:release
```

### Alternative: Using Bundle Exec

For better gem isolation:

```bash
bundle exec fastlane flutter_build flavor:dev mode:debug
```

## Common Errors and Solutions

### Error 1: Ruby Not Found

**Error Message:**

```
command not found: ruby
```

**Solution:**

```bash
# Install Ruby using Homebrew
brew install ruby

# Add Ruby to PATH (add to ~/.zshrc or ~/.bash_profile)
export PATH="/usr/local/opt/ruby/bin:$PATH"

# Reload shell configuration
source ~/.zshrc
```

### Error 2: Gem Command Not Found

**Error Message:**

```
command not found: gem
```

**Solution:**

```bash
# Ruby should come with Gems, but if missing:
brew install ruby

# Or update Ruby
brew upgrade ruby
```

### Error 3: Bundle Command Not Found

**Error Message:**

```
command not found: bundle
```

**Solution:**

```bash
# Install Bundler
gem install bundler

# If still not found, use full path
/usr/local/opt/ruby/bin/bundle install
```

### Error 4: Fastlane Gem Not Installed

**Error Message:**

```
command not found: fastlane
```

**Solution:**

```bash
# Install Fastlane globally
sudo gem install fastlane -NV

# Or install via Bundler (recommended)
cd apps/tanpura/android
bundle install

# Run via bundle
bundle exec fastlane flutter_build flavor:dev mode:debug
```

### Error 5: Permission Denied During Gem Installation

**Error Message:**

```
ERROR:  While executing gem ... (Gem::FilePermissionError)
  You don't have write permissions for the /usr/local/lib/ruby/gems/...
```

**Solution:**

```bash
# Option 1: Use Bundler (recommended)
bundle install

# Option 2: Use sudo (not recommended)
sudo gem install fastlane -NV

# Option 3: Configure Gem user installation
gem install --user-install bundler fastlane
export PATH="$PATH:$HOME/.gem/ruby/*/bin"
```

### Error 6: Flutter Not Found

**Error Message:**

```
sh: flutter: command not found
```

**Solution:**

```bash
# Add Flutter to PATH
export PATH="$PATH:/path/to/flutter/bin"

# Verify Flutter installation
flutter --version

# Add to ~/.zshrc or ~/.bash_profile for persistence
echo 'export PATH="$PATH:/path/to/flutter/bin"' >> ~/.zshrc
source ~/.zshrc
```

### Error 7: Gradle Wrapper Not Found

**Error Message:**

```
Could not find gradle wrapper
```

**Solution:**

```bash
# Ensure you're in the correct directory
cd apps/tanpura/android

# Make gradlew executable
chmod +x gradlew

# Or regenerate gradle wrapper
./gradlew wrapper --gradle-version=<version>
```

### Error 8: Git Submodules Not Initialized

**Error Message:**

```
CMake Error: The source directory does not contain a CMakeLists.txt file
```

**Solution:**

```bash
# Initialize Git submodules
git submodule update --init --recursive

# Or clone with submodules
git clone --recurse-submodules <repo-url>
```

### Error 9: Java Not Found

**Error Message:**

```
Error: JAVA_HOME not found
```

**Solution:**

```bash
# Install Java
brew install openjdk

# Set JAVA_HOME
export JAVA_HOME=/usr/libexec/java_home

# Verify Java installation
java -version

# Add to ~/.zshrc for persistence
echo 'export JAVA_HOME=/usr/libexec/java_home' >> ~/.zshrc
```

### Error 10: Fastfile Syntax Error

**Error Message:**

```
SyntaxError: unexpected token
```

**Solution:**

```bash
# Validate Fastfile syntax
fastlane validate_syntax

# Or manually check for:
# - Missing colons
# - Incorrect quote matching
# - Missing parentheses
# - Incorrect method calls

# Get help on Fastlane syntax
fastlane docs
```

### Error 11: Out of Memory During Build

**Error Message:**

```
java.lang.OutOfMemoryError
```

**Solution:**

```bash
# Increase Gradle memory in gradle.properties
echo "org.gradle.jvmargs=-Xmx4096m" >> apps/tanpura/android/gradle.properties

# Or set environment variable
export _JAVA_OPTIONS="-Xmx4g"
```

### Error 12: Build Cache Issues

**Error Message:**

```
Build failed - cache corruption detected
```

**Solution:**

```bash
# Clean build cache
cd apps/tanpura/android
./gradlew clean

# Clear Flutter build
flutter clean

# Rebuild
fastlane flutter_build flavor:dev mode:debug
```

## Troubleshooting Tips

1. **Check Gem Versions**

   ```bash
   bundle exec fastlane --version
   gem list | grep fastlane
   ```

2. **Enable Fastlane Verbose Output**

   ```bash
   FASTLANE_DEBUG=true fastlane flutter_build flavor:dev mode:debug
   ```

3. **Validate Configuration**

   ```bash
   fastlane validate_syntax
   fastlane env
   ```

4. **Check Flutter Installation**

   ```bash
   flutter doctor
   ```

5. **View Fastlane Documentation**
   ```bash
   fastlane docs
   fastlane action flutter_build
   ```

## Best Practices

1. **Always use Bundler** - Use `bundle exec` to ensure gem versions are consistent
2. **Keep Gemfile Updated** - Run `bundle update` periodically
3. **Commit Gemfile.lock** - Ensures reproducible builds across environments
4. **Use Environment Variables** - Store sensitive data in environment variables, not in code
5. **Document Custom Lanes** - Add descriptions to all Fastlane lanes
6. **Test Locally First** - Test Fastlane configurations locally before committing
7. **Version Control** - Keep Fastlane configuration in version control

## Resources

- [Fastlane Official Documentation](https://docs.fastlane.tools)
- [Fastlane Android Documentation](https://docs.fastlane.tools/getting-started/android/setup/)
- [Flutter Build Documentation](https://flutter.dev/docs/deployment/android)
- [Ruby Installation Guide](https://www.ruby-lang.org/en/documentation/installation/)
- [Bundler Documentation](https://bundler.io/)

## Next Steps

After successful setup:

1. Test the build locally with `fastlane flutter_build flavor:dev mode:debug`
2. Configure CI/CD pipeline to use Fastlane
3. Set up automated deployments to Play Store (requires Play Store credentials)
4. Monitor build times and optimize as needed
5. Document any custom lanes or configurations specific to your team

## Support

For issues or questions:

1. Check the [Fastlane Issues](https://github.com/fastlane/fastlane/issues)
2. Run `fastlane docs` for action-specific help
3. Review the project's CI/CD logs for build failures
4. Check `fastlane/report.xml` for build reports
