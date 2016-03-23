#!/usr/bin/env bash

#-----------------------------------------------------------------------------
# Define egrep-q
#-----------------------------------------------------------------------------
egrep-q() {
  egrep "$@" >/dev/null 2>/dev/null
}

#-----------------------------------------------------------------------------
# Git color output
#-----------------------------------------------------------------------------
if test "$(git config color.ui)" != "auto"; then
  echo '
You may want to enable color output from Git commands with

  git config --global color.ui auto

'
fi

#-----------------------------------------------------------------------------
# Git bash-completion
#-----------------------------------------------------------------------------
if ! bash -i -c 'echo $PS1' | egrep-q '__git_ps1'; then
  echo '
A dynamic, informative Git shell prompt can be obtained by sourcing the git
bash-completion script in your ~/.bashrc.  Set the PS1 environmental variable as
suggested in the comments at the top of the bash-completion script.  You may
need to install the bash-completion package from your distribution to obtain the
script.

'
fi

#-----------------------------------------------------------------------------
# Git Merge tool configuration
#-----------------------------------------------------------------------------
if ! git config merge.tool >/dev/null; then
  echo '
A merge tool can be configured with

  git config merge.tool <toolname>

For more information, see

  git help mergetool

'
fi

echo "Done."
