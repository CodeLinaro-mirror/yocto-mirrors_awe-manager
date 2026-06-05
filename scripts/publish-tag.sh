#!/bin/bash

# this script publishes a specific AWE-Manager release tag to our public repo
# 
# Configuration
REMOTE_NAME="public"

# enable and execute these for test driving script:
# rm -rf ~/git-test/public-repo && mkdir -p ~/git-test/public-repo && cd ~/git-test/public-repo && git init --bare
# REMOTE_URL="$HOME/git-test/public-repo" # Using $HOME for absolute path safety
REMOTE_URL="git@bitbucket.org:dspconcepts/awe-manager.git"

TARGET_BRANCH="main"

TAG_NAME=$1

if [ -z "$TAG_NAME" ]; then
    echo "❌ Error: Please provide a tag name."
    exit 1
fi

# 1. Verification
if ! git rev-parse "$TAG_NAME" >/dev/null 2>&1; then
    echo "❌ Error: Tag '$TAG_NAME' not found."
    exit 1
fi

# --- 💡 CRITICAL FIX FOR LOCAL WSL FOLDERS ---
# 1. Allow LFS to skip the "server" check
git config lfs.allowincompletepush true
# 2. Force LFS to use basic file copying for this specific remote
git config "remote.$REMOTE_NAME.lfsurl" "$REMOTE_URL"
# ---------------------------------------------

# 2. Setup Remote
# Check if the remote exists
if git remote | grep -qx "$REMOTE_NAME"; then
    echo "🔄 Remote '$REMOTE_NAME' exists. Updating URL to: $REMOTE_URL"
    git remote set-url "$REMOTE_NAME" "$REMOTE_URL"
else
    echo "🔗 Adding new remote '$REMOTE_NAME': $REMOTE_URL"
    git remote add "$REMOTE_NAME" "$REMOTE_URL"
fi

# 3. Ensure LFS objects are present
echo "📦 Fetching LFS objects for $TAG_NAME..."
git lfs fetch origin "$TAG_NAME"

# 4. Create Orphan Branch
CURRENT_BRANCH=$(git branch --show-current)
TEMP_BRANCH="export-lfs-$(date +%s)"
git checkout --orphan "$TEMP_BRANCH" "$TAG_NAME" >/dev/null 2>&1

# 5. Commit
git add -A
git commit -m "Public Release $TAG_NAME (Clean Slate with LFS)" --quiet

# 6. Push LFS Objects
echo "📤 Uploading LFS assets to $REMOTE_NAME..."
# We use --all to ensure all objects for this commit are moved
git lfs push "$REMOTE_NAME" "$TEMP_BRANCH"

# 7. Push Code
echo "🚀 Pushing code to $REMOTE_NAME/$TARGET_BRANCH..."
if git push "$REMOTE_NAME" "$TEMP_BRANCH":"$TARGET_BRANCH" --force; then
    git push "$REMOTE_NAME" "$TEMP_BRANCH":refs/tags/"$TAG_NAME" --force
    echo "✅ Success!"
else
    echo "❌ Error: Push failed. Check if $REMOTE_URL is a 'git init --bare' repo."
fi

# 8. Cleanup
git checkout "$CURRENT_BRANCH" --quiet
git branch -D "$TEMP_BRANCH" --quiet
git remote remove "$REMOTE_NAME"

