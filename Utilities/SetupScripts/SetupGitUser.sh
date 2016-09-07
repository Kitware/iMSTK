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
# Define Setup User
#-----------------------------------------------------------------------------
setup_user() {
  read -ep "Please enter your full name, such as 'John Doe': " name
  echo "Setting name to '$name'"
  git config user.name "$name"
  read -ep "Please enter your email address, such as 'john@email.com': " email
  echo "Setting email address to '$email'"
  git config user.email "$email"
}

#-----------------------------------------------------------------------------
# Setup user if empty credentials
#-----------------------------------------------------------------------------
gitName=$(git config user.name)
gitEmail=$(git config user.email)
if [ "$gitName" == "" ] || [ "$gitEmail" == "" ]; then
  setup_user
fi

#-----------------------------------------------------------------------------
# Loop until the user is happy with the authorship information
#-----------------------------------------------------------------------------
for (( ; ; ))
do
  # Display the final user information
  gitName=$(git config user.name)
  gitEmail=$(git config user.email)
  echo "Your commits will have the following author:
  $gitName <$gitEmail>
  "
  
  # Ask for user confirmation
  read -ep "Is the author name and email address above correct? [Y/n] " correct
  if [ "$correct" == "n" ] || [ "$correct" == "N" ]; then
    setup_user
  else
    break
  fi
done

echo "Done."
