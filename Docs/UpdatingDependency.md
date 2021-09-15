# Updating a Dependency in iMSTK

To update a dependency in imstk one needs to:

 - Swap the url in ` /CMake/External/<DependencyLibraryName>.cmake` to a different url.
    -  Ex: `https://gitlab.kitware.com/iMSTK/assimp/-/archive/fixCompilationError/assimp-fixCompilationError.zip`. Pulling zips prevents git history with it which can be sizeable.
    - Alternatively git shallow would be used to avoid large git histories. But it fails when the tree gets too large, thus if the HEAD of the branch pulled from moves too far, it will fail to pull. (used to cause old version of iMSTK to fail to build until we switched to zips)
 - Update the md5 hashsum in that same file (if using zips).
    - The hashsum is a nearly unique identifer generated from the files of the dependency. This mostly is a security measure that ensures you are getting the files you expect to be getting.
    - To acquire the md5 hashsum one can build iMSTK with the newly updated url. It will fail on the md5 check and report both the actual and current md5.

## Updating a Remote Fork

Most dependencies in iMSTK are forked. This way we don't depend on the remote repository as it could change (rebased/amended/etc). A few forks also contain our own diffs in the rare case something like a cmake fix is introduced. To update a fork:
 - Clone the fork locally: `git clone <git url of fork>` (all forks found in iMSTK group here: https://gitlab.kitware.com/iMSTK)
 - Add upstream `git remote add upstream <url of actual repo/repo forked off>` (the description of the fork normally provides what it was forked from)
    - Tip: `git remote -v` to list all remotes
 - Merge upstream (or rebase): `git merge upstream/<branch to update from>`
 - Push your changes

After updating your fork you can proceed with the beginning of this guide on how to pull a different source.