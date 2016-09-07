#!/usr/bin/env bash

#-----------------------------------------------------------------------------
# Define Error and Exit
#-----------------------------------------------------------------------------
printErrorAndExit() {
  echo 'Failure during git hook setup' 1>&2
  echo '------------------------------------' 1>&2
  echo '' 1>&2
  echo "$@" 1>&2
  exit 1
}

AssertFileExists() {
  FILE=$1
  if [ ! -f $FILE ]; then
   echo "Error: the file '$FILE' is missing."
   printErrorAndExit
  fi
}

#-----------------------------------------------------------------------------
# Go to GitHooks directory
#-----------------------------------------------------------------------------
cd "${BASH_SOURCE%/*}/../GitHooks"

#-----------------------------------------------------------------------------
# Populate ".git/hooks"
#-----------------------------------------------------------------------------
git_dir=$(git rev-parse --git-dir)
cp * $git_dir/hooks

echo "Done."
