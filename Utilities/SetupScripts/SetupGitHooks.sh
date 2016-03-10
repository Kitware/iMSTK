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
echo "Populating git hooks..."
git_dir=$(git rev-parse --git-dir)
cp * $git_dir/hooks

#-----------------------------------------------------------------------------
# Go to the project root directory
#-----------------------------------------------------------------------------
#cd "../.."

#-----------------------------------------------------------------------------
# Set up KWStyle hook
#-----------------------------------------------------------------------------
#echo "Setting up the KWStyle hook..."
#KWStyleConf="Utilities/KWStyle/iMSTK.kws.xml"
#KWStyleOverwriteConf="Utilities/KWStyle/iMSTKKWSOverwrite.txt"
#AssertFileExists $KWStyleConf
#AssertFileExists $KWStyleOverwriteConf
#git config hooks.KWStyle.conf $KWStyleConf
#git config hooks.KWStyle.overwriteRulesConf $KWStyleOverwriteConf
#git config hooks.KWStyle true

echo "Done."
