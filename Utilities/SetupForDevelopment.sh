#!/usr/bin/env bash

#-----------------------------------------------------------------------------
# Define Error and Exit
#-----------------------------------------------------------------------------
printErrorAndExit() {
  echo 'Failure during git development setup' 1>&2
  echo '------------------------------------' 1>&2
  echo '' 1>&2
  echo "$@" 1>&2
  exit 1
}

#-----------------------------------------------------------------------------
# Go to the repository root
#-----------------------------------------------------------------------------
cd "$(echo "$0" | sed 's/[^/]*$//')"/..

if test -d .git/.git; then
  printErrorAndExit "The directory '.git/.git' exists, indicating a
  configuration error. Please 'rm -rf' this directory."
fi

#-----------------------------------------------------------------------------
# Make 'git pull' on master always use rebase
#-----------------------------------------------------------------------------
git config branch.master.rebase true

#-----------------------------------------------------------------------------
# Make automatic and manual rebase operations to display a summary and stat
# display of changes merged in the fast-forward operation.
#-----------------------------------------------------------------------------
git config rebase.stat true

#-----------------------------------------------------------------------------
# Run Setup Scripts
#-----------------------------------------------------------------------------
cd Utilities/SetupScripts

echo "Checking basic user information..."
./SetupGitUser.sh || exit 1
echo

echo "Setting up git hooks..."
./SetupGitHooks.sh || exit 1
echo

echo "Suggesting git tips..."
./SuggestGitTips.sh || exit 1
echo

#-----------------------------------------------------------------------------
# Record the version of this setup so the developer can be notified that
# this script and/or hooks have been modified.
#-----------------------------------------------------------------------------
SetupForDevelopment_VERSION=1
git config hooks.SetupForDevelopment ${SetupForDevelopment_VERSION}
