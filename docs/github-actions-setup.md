# GitHub Actions Setup Guide for Tanpura Project

## Overview

GitHub Actions is a continuous integration and continuous deployment (CI/CD) platform that allows you to automate your build, test, and deployment pipeline. In this project, we use GitHub Actions to automatically build Flutter APKs for different flavors and modes whenever triggered.

## What is GitHub Actions?

GitHub Actions enables you to:

- Automate tests on every push and pull request
- Automatically build and deploy applications
- Run custom scripts and actions on specific events
- Create workflow automation without third-party services
- Run workflows on GitHub-hosted runners or self-hosted runners

## Prerequisites

Before setting up GitHub Actions, ensure you have:

- A GitHub repository (public or private)
- Push access to the repository
- Administrator access to enable/manage GitHub Actions (for private repos, Actions should be enabled)
- Flutter SDK compatible with your project
- Android build environment configured locally (tested and working)
- Git submodules properly initialized (for projects using submodules like Oboe)

## Project Setup

### Repository Structure

Our GitHub Actions workflows are located in:

```
.github/
└── workflows/
    └── flutter_ci.yml
```

### Existing Workflow: flutter_ci.yml

Our main workflow file is configured for manual triggering with customizable parameters:

```yaml
name: Flutter CI (Manual Trigger)

on:
  workflow_dispatch:
    inputs:
      branch-name:
        description: "Enter branch name"
        required: true
        default: main
      app-name:
        description: "Enter app name"
        required: true
        default: tanpura
      flavor:
        description: "Select build flavor"
        required: true
        default: dev
        options:
          - dev
          - staging
          - prod
      build-mode:
        description: "Select build mode"
        required: true
        default: debug
        options:
          - debug
          - release
      flutter-version:
        description: "Select flutter version"
        required: true
        default: "3.32.0"
```

## How GitHub Actions Works

### 1. Triggers

Workflows are triggered by events:

- **`workflow_dispatch`** - Manual trigger from GitHub UI (our setup)
- **`push`** - When code is pushed to a branch
- **`pull_request`** - When a PR is opened or updated
- **`schedule`** - Cron-based scheduled runs
- **`release`** - When a release is created

### 2. Jobs

Jobs are sequences of steps that run on runners. Each job runs on a fresh instance of the specified runner.

```yaml
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      # Steps execute sequentially
```

### 3. Runners

Runners are machines that execute the jobs. GitHub provides:

- **GitHub-hosted runners**: ubuntu-latest, windows-latest, macos-latest
- **Self-hosted runners**: Your own machines

### 4. Steps

Steps are individual commands or actions that run within a job.

```yaml
- name: Step name
  run: command to run
  # or
  uses: action/from/marketplace
```

## Our Workflow Breakdown

### Step 0: Set Build Variables

```yaml
- name: Set build variables
  run: |
    echo "FLAVOR=${{ github.event.inputs.flavor }}" >> $GITHUB_ENV
    echo "MODE=${{ github.event.inputs.build-mode }}" >> $GITHUB_ENV
    echo "TARGET=lib/main_${{ github.event.inputs.flavor }}.dart" >> $GITHUB_ENV
```

**Purpose:** Converts workflow inputs into environment variables for use in subsequent steps.

**Variables:**

- `FLAVOR` - Selected app flavor (dev/staging/prod)
- `MODE` - Build mode (debug/release)
- `TARGET` - Entry point file path

### Step 1: Checkout Code with Submodules

```yaml
- name: Checkout code
  uses: actions/checkout@v4
  with:
    ref: ${{ github.event.inputs.branch-name }}
    submodules: "recursive"
```

**Purpose:** Clones the repository and initializes all Git submodules (critical for projects with Oboe).

**Key Attribute:** `submodules: "recursive"` - Ensures oboe and other submodules are properly initialized.

### Step 2: Set up Java

```yaml
- name: Set up Java
  uses: actions/setup-java@v4
  with:
    distribution: temurin
    java-version: 17
```

**Purpose:** Installs Java Runtime Environment needed for Android build tools.

**Distribution:** Temurin (AdoptOpenJDK) - Recommended for CI environments.

### Step 3: Set up Flutter

```yaml
- name: Set up Flutter
  uses: subosito/flutter-action@v2
  with:
    flutter-version: ${{ github.event.inputs.flutter-version }}
    cache: true
```

**Purpose:** Installs Flutter SDK at specified version.

**Caching:** `cache: true` speeds up workflow runs by caching Flutter dependencies.

### Step 4: Get Dependencies

```yaml
- name: Flutter pub get
  working-directory: apps/${{ github.event.inputs.app-name }}
  run: flutter pub get
```

**Purpose:** Fetches Flutter and Dart dependencies.

### Step 5: Build APK via Fastlane

```yaml
- name: Build APK via Fastlane
  working-directory: apps/${{ github.event.inputs.app-name }}/android
  run: |
    fastlane flutter_build \
      flavor:${FLAVOR} \
      mode:${MODE}
```

**Purpose:** Builds the APK using Fastlane with specified flavor and mode.

### Step 6: Upload APK Artifact

```yaml
- name: Upload APK artifact
  uses: actions/upload-artifact@v4
  with:
    name: app-${{ github.event.inputs.flavor }}-${{ github.event.inputs.build-mode }}-apk
    path: apps/${{ github.event.inputs.app-name }}/build/app/outputs/flutter-apk/*.apk
```

**Purpose:** Uploads built APK as workflow artifact for download.

**Retention:** Artifacts are kept for 90 days by default.

## Running the Workflow

### Method 1: GitHub Web Interface

1. Go to your repository on GitHub
2. Click **Actions** tab
3. Select **Flutter CI (Manual Trigger)** workflow
4. Click **Run workflow** button
5. Fill in the input parameters:
   - Branch name (e.g., `main`)
   - App name (e.g., `tanpura`)
   - Flavor (dev/staging/prod)
   - Build mode (debug/release)
   - Flutter version (e.g., `3.32.0`)
6. Click **Run workflow**

### Method 2: GitHub CLI

```bash
# Install GitHub CLI
brew install gh

# Authenticate
gh auth login

# Run workflow
gh workflow run flutter_ci.yml \
  -f branch-name=main \
  -f app-name=tanpura \
  -f flavor=dev \
  -f build-mode=debug \
  -f flutter-version=3.32.0
```

### Method 3: API

```bash
curl -X POST \
  -H "Authorization: token YOUR_GITHUB_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/YOUR_USERNAME/tanpurax/actions/workflows/flutter_ci.yml/dispatches \
  -d '{"ref":"main","inputs":{"branch-name":"main","app-name":"tanpura","flavor":"dev","build-mode":"debug","flutter-version":"3.32.0"}}'
```

## Monitoring Workflow Runs

### View Results

1. Go to **Actions** tab in your repository
2. Click on the workflow run name
3. View job status and logs in real-time or after completion
4. Click on individual steps to see detailed output

### Download Artifacts

1. After workflow completion, scroll to **Artifacts** section
2. Download the APK artifact

### Check Workflow Status Badge

Add to README.md:

```markdown
![Flutter CI](https://github.com/YOUR_USERNAME/tanpurax/actions/workflows/flutter_ci.yml/badge.svg)
```

## Common Errors and Solutions

### Error 1: "You need to be an administrator or have the 'manage Actions' permission"

**Error Message:**

```
You do not have permission to enable/disable GitHub Actions
```

**Solution:**

- Ask repository owner/administrator to enable GitHub Actions
- Navigate to Settings → Actions → General
- Ensure "Allow all actions and reusable workflows" is selected

### Error 2: "Submodule Not Found"

**Error Message:**

```
CMake Error at CMakeLists.txt:8 (add_subdirectory):
The source directory does not contain a CMakeLists.txt file
```

**Solution:**

```yaml
# Ensure checkout uses recursive submodules
- name: Checkout code
  uses: actions/checkout@v4
  with:
    ref: ${{ github.event.inputs.branch-name }}
    submodules: "recursive" # This is critical
```

### Error 3: "Java Not Found"

**Error Message:**

```
Error: Unable to locate Java compiler
```

**Solution:**

```yaml
# Ensure Java setup step is included and runs before Gradle
- name: Set up Java
  uses: actions/setup-java@v4
  with:
    distribution: temurin
    java-version: 17
```

### Error 4: "Flutter Not Found"

**Error Message:**

```
bash: flutter: command not found
```

**Solution:**

```yaml
# Ensure Flutter setup runs before using flutter command
- name: Set up Flutter
  uses: subosito/flutter-action@v2
  with:
    flutter-version: ${{ github.event.inputs.flutter-version }}
    cache: true
```

### Error 5: "Fastlane Command Not Found"

**Error Message:**

```
fastlane: command not found
```

**Solution:**
Option 1 - Install Fastlane in workflow:

```yaml
- name: Install Fastlane
  run: |
    gem install fastlane -NV
```

Option 2 - Use Bundler (recommended):

```yaml
- name: Install dependencies
  working-directory: apps/${{ github.event.inputs.app-name }}/android
  run: bundle install

- name: Build APK via Fastlane
  working-directory: apps/${{ github.event.inputs.app-name }}/android
  run: bundle exec fastlane flutter_build flavor:${FLAVOR} mode:${MODE}
```

### Error 6: "APK Build Failed - Out of Memory"

**Error Message:**

```
java.lang.OutOfMemoryError: Java heap space
```

**Solution:**
Add to workflow before build step:

```yaml
- name: Build APK
  env:
    _JAVA_OPTIONS: "-Xmx4g"
    GRADLE_OPTS: "-Xmx4g"
  run: # build command
```

### Error 7: "Artifact Not Found After Build"

**Error Message:**

```
Artifact upload failed - path not found
```

**Solution:**
Verify the APK output path is correct:

```bash
# Build locally and check where APK is generated
find build -name "*.apk" -type f

# Update workflow with correct path
- name: Upload APK artifact
  uses: actions/upload-artifact@v4
  with:
    name: app-apk
    path: apps/tanpura/build/app/outputs/flutter-apk/*.apk
```

### Error 8: "Workflow File Syntax Error"

**Error Message:**

```
Invalid workflow syntax
```

**Solution:**

- Validate YAML syntax using online tools (yamllint.com)
- Check for correct indentation (use 2 spaces, not tabs)
- Verify quotes are matched properly
- Check for missing colons in key-value pairs

Example of common YAML error:

```yaml
# ❌ Wrong - missing colon
run echo "test"

# ✅ Correct
run: echo "test"
```

### Error 9: "Timeout - Workflow Exceeded Time Limit"

**Error Message:**

```
The operation timed out
```

**Solution:**

- Default timeout is 360 minutes (6 hours)
- For faster execution, optimize steps:
  - Use caching for Flutter/Gradle
  - Skip unnecessary steps
  - Parallel jobs (if possible)

```yaml
- name: Set up Flutter
  uses: subosito/flutter-action@v2
  with:
    flutter-version: ${{ github.event.inputs.flutter-version }}
    cache: true # Enable caching
```

### Error 10: "Insufficient Disk Space"

**Error Message:**

```
No space left on device
```

**Solution:**

```yaml
# Free up space before build
- name: Free up space
  run: |
    rm -rf /usr/share/dotnet
    rm -rf /usr/local/lib/android
    df -h
```

### Error 11: "Input Parameter Not Recognized"

**Error Message:**

```
Workflow input "branch-name" is not defined
```

**Solution:**
Ensure parameter names in `inputs` match usage in steps:

```yaml
on:
  workflow_dispatch:
    inputs:
      branch-name: # Note: hyphenated name
        description: "Enter branch name"

steps:
  - uses: actions/checkout@v4
    with:
      ref: ${{ github.event.inputs.branch-name }} # Use same name
```

### Error 12: "Permission Denied - Cannot Write to Artifact"

**Error Message:**

```
Permission denied writing to artifact path
```

**Solution:**

```yaml
# Ensure build step runs with correct permissions
- name: Build APK
  run: |
    chmod +x apps/tanpura/android/gradlew
    # Then run build
```

## Troubleshooting Tips

### 1. Check Workflow Logs

Click on a workflow run and view detailed logs for each step.

### 2. Enable Debug Logging

```yaml
env:
  RUNNER_DEBUG: 1 # Enable runner debug logging
```

### 3. Test Locally First

Always test your build locally before pushing to GitHub:

```bash
# Test the exact commands that will run in workflow
flutter pub get
flutter build apk --dev --flavor dev -t lib/main_dev.dart
```

### 4. Validate Workflow Syntax

```bash
# Using official action validator
gh workflow validate .github/workflows/flutter_ci.yml
```

### 5. Check Runner Specifications

View available runners and their specifications:

```bash
gh api repos/YOUR_USERNAME/tanpurax/actions/runners
```

### 6. Monitor Job Duration

Track how long each step takes:

- Click workflow run
- View "Summary" tab
- Check timing for each step

## Advanced Configuration

### Creating Multiple Workflows

Create separate workflows for different purposes:

```
.github/workflows/
├── flutter_ci.yml          # Manual builds
├── flutter_test.yml        # Run tests
├── deploy_play_store.yml   # Deploy to Play Store
└── scheduled_build.yml     # Nightly builds
```

### Scheduled Builds

Add automatic nightly builds:

```yaml
name: Nightly Build

on:
  schedule:
    # Every day at 2 AM UTC
    - cron: "0 2 * * *"

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      # Same steps as flutter_ci.yml
```

### Push/Pull Request Triggers

Automatically build on every push:

```yaml
on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main, develop]
```

### Environment Secrets

Store sensitive data securely:

1. Go to **Settings** → **Secrets and variables** → **Actions**
2. Add secrets (e.g., API keys, credentials)
3. Use in workflow:

```yaml
env:
  PLAY_STORE_KEY: ${{ secrets.PLAY_STORE_KEY }}
```

## Best Practices

1. **Always Use Latest Action Versions** - Regularly update action versions (`@v4`, `@v2`)
2. **Cache Dependencies** - Use caching to speed up builds
3. **Validate Inputs** - Ensure required inputs are provided
4. **Fail Fast** - Stop on first error instead of continuing
5. **Clear Naming** - Use descriptive step names
6. **Document Secrets** - Keep track of which secrets are used
7. **Monitor Costs** - GitHub Actions has usage limits for private repos
8. **Test Changes** - Test workflow changes on a feature branch first
9. **Use Concurrency** - Prevent multiple simultaneous runs:

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

10. **Archive Logs** - Keep logs for debugging:

```yaml
- name: Upload build logs
  if: always()
  uses: actions/upload-artifact@v4
  with:
    name: build-logs
    path: build/logs/
```

## Resources

- [GitHub Actions Official Documentation](https://docs.github.com/en/actions)
- [GitHub Actions Marketplace](https://github.com/marketplace?type=actions)
- [Workflow Syntax Reference](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Events that trigger workflows](https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows)
- [Flutter Action](https://github.com/subosito/flutter-action)
- [Setup Java Action](https://github.com/actions/setup-java)
- [Upload Artifact Action](https://github.com/actions/upload-artifact)

## Security Considerations

### 1. Protect Secrets

Never commit secrets to version control:

```yaml
# ❌ Wrong
password: "abc123def456"

# ✅ Correct
password: ${{ secrets.MY_PASSWORD }}
```

### 2. Use Minimal Permissions

Limit what each job can do:

```yaml
permissions:
  contents: read
  packages: write
```

### 3. Review Workflow Changes

Always review workflow changes in pull requests before merging.

### 4. Audit Workflow Runs

Regularly check who triggered workflows and what they did.

## Next Steps

1. Test the current `flutter_ci.yml` workflow manually
2. Monitor build times and optimize if needed
3. Set up additional workflows for testing (flutter test)
4. Configure Play Store deployment when ready
5. Set up notifications for build failures
6. Create runbooks for common issues

## Support

For issues or questions:

1. Check [GitHub Actions Issues](https://github.com/actions/toolkit/issues)
2. Visit [GitHub Community Discussions](https://github.com/orgs/community/discussions)
3. Review [Stack Overflow - github-actions tag](https://stackoverflow.com/questions/tagged/github-actions)
4. Check project CI/CD logs and error messages
5. Test workflow syntax locally with `gh workflow validate`
