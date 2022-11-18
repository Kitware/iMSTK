# iMSTK contributing guide

* [Forking workflow](#forking-workflow)
* [Coding guidelines](#coding-guidelines)
  * [Coding style](#coding-style)
  * [Documenting your code](#documenting-your-code)
* [Testing code](#testing-code)
* [Commit guidelines](#commit-guidelines)
  * [Commit 101](#commit-101)
  * [Commit messages](#commit-messages)
  * [Editing previous commits](#editing-previous-commits)
  * [Amending changes to last commit](#amending-changes-to-last-commit)
  * [Removing previous commit changes](#removing-previous-commit-changes)
  * [Editing older commits](#editing-older-commits)
  * [Syncing repositories](#syncing-repositories)
* [Data Management](#data-management)
  * [Add Data](#add-data)
  * [Fetch Data](#fetch-data)
  * [Data Usage](#data-usage)
* [Dashboards](#dashboards)


## Forking workflow
To contribute your changes to iMSTK, you will need to follow the [Gitlab forking workflow].

1. [Fork] the iMSTK project to your own namespace.

2. Add the forked remote repository to your git source directory.
```sh
git remote add myremote git@gitlab.kitware.com:<mynamespace>/iMSTK.git
```

3. Create a new branch where you will commit your changes, and name it based on the feature or fix you are implementing.
```sh
git checkout -b new-feature
```

4. Implement your changes: please follow the [coding guidelines](#coding-guidelines) below.

5. Commit your changes: please follow the [commit guidelines](#commit-guidelines) below.

6. Push your development branch to your namespace.
```sh
git push myremote new-feature
```

7. [Create a merge request] once your branch is ready for merging, or earlier as [WIP] if this is a long merge request and need other developers to discuss it.

8. Stay available for the review process: you might need to address some issues and push new changes.

9. Once your changes have been reviewed and the associated builds and tests pass on [iMSTK dashboards](#dashboards), your branch will be merged into `master`.

10. At that point, you may decide to get rid of your development branch and pull the latest commits from the master branch, then repeat from step 3 for further development
```sh
git branch -d new-feature
git checkout master
git pull origin master
```


## Coding guidelines

See the [Imstk Coding Guide](https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Docs/CodingGuide.md)

#### Code style check
You should use the `uncrustify` tool to check and/or adjust code style of your files.

* Visual Studios: Build the uncrustify target in the Innerbuild.sln.
* Linux: Run `./install/bin/uncrustify --replace --no-backup -c ../iMSTK/Utilities/Uncrustify/iMSTKUncrustify.cfg -F Innerbuild/uncrustify.list`. Ensure your uncrustify.list is up to date by running cmake.

\*INDENT-ON\* and \*INDENT-OFF\* keywords in comments can be used to disable uncrustify's style check for a portion of the code, this is currently used to avoid some issues uncrustify has with macros.

#### Testing Code
A developer with `Developer` role in giltab may run tests on MRs submitted. Assign or ping one for review and to run tests on our machines. A developer may run:
 * `do: test`: To run an incremental build
 * `do: test --clean`: To run a clean build. May be useful for dependency, build, or data changes.
 * `do: test --stop`: Will stop issuing build commands, does not currently stop mid build.
 * `do: test --help`: For information on more commands.

Test results are repoted to our [CDash](https://open.cdash.org/index.php?project=iMSTK). Here you will find a Merge Requests section with cmake configure, build, & test sections.

## Commit guidelines

### Commit 101

1. **Commit often**

    You do not need to have a lot of changes to create a commit. On the contrary, you should try to split your changes into small logical steps and commit each of them. They should be consistent, self-contained (work independently of later commits), and pass the same test which previously did (or more) to ensure you do not introduce any regression. On the other hand, do not push it to the extreme: if your changes are tightly related and stay clear as a whole, there is no need to create multiple commits, even if they are applied to different files, methods, etc.
    >TIP: `git gui` will allow you to stage changes by lines or by hunk instead of the whole file. It is extremely useful when your recent changes could be divided in multiple commits.

2. **Style only rhymes with style**

    Style changes can introduce a big number of diffs in your whole project (take a variable name change, tabulation in a whole file...). If commited as part of a commit for enhancement or bug fixes, it becomes very difficult for a reviewer to track down the changes specific to your work, and the ones specific to style changes. For that reason, prefer grouping style changes in a separate commit.

3. **Push often**

    A good reason to push often is to have your work somewhere else than just on your computer in the event of something were to happen to local computer. Another reason is to facilitate code sharing: even if your branch is not at maturity, creating a pull request as [WIP] will allow you to discuss your progress, ask questions, and get feedbacks during the development process and not just at the end.

4. **Merge often...**

    The whole idea behind continuous integration (or CI) is to integrate your own code with the main repository as often as you can. If you make small changes and put them into single commits you may integrate them often (and probably should). Doing so minimizes merge conflicts with your team members. You can read more about CI in a Git context in Martin Fowler’s excellent [FeatureBranch article].

5. **... but never the other way around!**

    If your branch and the master branch diverge and are in conflicts, never resolve those by merging master into your branch. Instead, rebase your branch against master. This will allow you to adress the conflicts where they were introduced instead of commiting a patch complicated to review, and will keep the commit history clean.

Some of the content above was directly or partially taken from Crealytics blog : [5 reasons for keeping your git commits small](https://crealytics.com/blog/2010/07/09/5-reasons-keeping-git-commits-small/).

### Commit messages

Proper commit messages are important as they allow to speed up the review process. They are also crucial for development down the road to be able to come back to that commit and understand the logic behind the changes.

1. Describe thoroughly

    Assume the reviewers do not know your motivation behind your work: every change you make has a reason you need to explicitly write down in the commit description, even the little things (ex: why changing a default value). This will accelerate the review process by allowing reviewers to rely on your work description instead of having to contact you for answers.

2. Document accordingly

    A great addition is to offer links to additional information like your source (reference to the algorithm you are using, online discussion supporting your changes...), hash of the commit that introduced the regression being fixed, etc.

3. Be specific

    When making changes to multiple classes or refering to other classes than the one being modified, be specific about class functions and member variables by properly specifying the namespace: `Class:function()` or `Class::m_member`.

4. List changes

    If your commit is composed of multiple changes in order to stay self-contained, list your changes instead of appending them in long paragraphs for clarity.

5. Use a clear and short commit title

    Describe the topic in a couple words while being as specific as possible. "Improve rendering" or "Fix controllers" would be considered too broad.

6. Use the appropriate title prefix to help quickly understand broad context
   * `BUG`: Fix for runtime crash or incorrect result
   * `COMP`: Compiler error or warning fix
   * `DOC`: Documentation change
   * `ENH`: New functionality
   * `REFAC`: Rework of existing functionality
   * `PERF`: Performance improvement
   * `TEST`: Test change
   * `STYLE`: No logic impact (indentation, comments)
   * `WIP`: Work In Progress not ready for merge

### Editing previous commits

While implementing your topic that would be holding multiple commits, you might end up making changes which should be coupled with or which revert changes from previous commits. The same can happen when your merge request has been reviewed and you need to make changes that should be applied in an existing commit.

It is possible to change previous commits in order to keep your topic clean and make the reviewing process easier, a couple options are listed below.

This process consists of rewriting the branch history, therefore re-syncing it with other remote and local repositories will be necessary ([see below](#syncing-repositories)).

* **Amending changes to last commit**

    All you have to do is stage the extra changes like you would for a normal commit, then commit with the --amend argument.
    ```
    git commit --amend
    ```
    > TIP: You can also use `git gui` and click on the amend commit radio button.*

* **Removing previous commit changes**

    To go to your previous commit and make changes, you can use reset:
    ```
    git reset HEAD~
    ```
    This will undo your commit, but keep your changes as unstaged. Once your have made your changes, you can then create a new commit:
    ```
    git commit -m "message"
    ```
    >TIP:
    >* You can also use `git gui` and click on the amend commit radio button, then unstage or remove changes from the previous commit.
    >* To keep the changes staged, you can use the `--soft` flag. If you want to get rid of the commit changes alltogether, you can use the `--force`.
    >* To reset more than one commit, you can call `git reset HEAD~X` with X the number of commits to reset. Works with the flags referenced above.

* **Editing older commits**

    If you need to change commits that precede your last commit, you can use the interactive rebase tool.

    First, run git log to see how much back you need to go from your current state:
    ```
    git log --pretty=oneline
    ```
    If you want to make changes up to 3 commits back, you would then call:
    ```
    git rebase -i HEAD~3
    ```
    Running this command gives you a list of commits in your text editor that looks like this *(note that the order is inversed from git log, the more recent commits being at the bottom of the list)*:
    ```
    pick ecd29d0 ENH: Add custom event handlers for vtk interaction
    pick 529ca92 COMP: Fix function is not a member of std (Win)
    pick 8d0bc67 COMP: Fix return error in MeshIO::write
    ```
    From here, you can do multiple things:
    * Rearrange the order of commits by swapping the lines (in vim: `d+d` to copy, `p` to paste)
    * Delete a commit by removing the commit line
    * Replace the `pick` command (which does nothing) by other commands, like:
      * `r` or `reword` to edit the commit message
      * `e` or `edit` to stop for amending
      * `s` or `squash` to meld into previous commit (line above) and stop to edit the commit message
      * `f` or `fixup` to meld into previous commit (line above) and ignore that commit message
      * `d` or `drop` to remove the commit (same as removing the line

    When you save and exit the editor, Git rewinds you back to the last commit in that list and starts running the commands per commit, from the oldest commit to the more recent. If you stop at a commit to amend it with `edit`, follow the [instructions above to amend/edit the latest commit](#amend-changes-to-last-commit) (you can even create multiple commits). Once you are ready, continue the rebase with the command:

    ```
    git rebase --continue
    ```

    * ***How do I amend some code I already wrote to a previous comment?***

        You have two possibilities:
        * If you have already commited that new code, you can place its line under the commit to amend in the interactive rebase, and change its command to `s` or `f` if you want to edit the initial commit message or not.
        * If you haven't commited your changes, you can either commit them and follow the instructions above, or stash them:

            ```
            git stash
            ```

            Then run the interactive rebase with `e` on the commit to amend. Once you stop at that commit, you can pop the changes from the stash, commit them, then continue:

            ```
            git stash pop
            git add .
            git commit --amend
            git rebase --continue
            ```

    * ***I have conflicts when rebasing or calling stash pop, what do I do?***

        When you try to amend changes to a previous commit, it is necessary that no changes were made to that same chunk of code in between to avoid conflicts. If you end up having conflicts, there are two solutions:
        * Try fixing the conflicts by hand: if you have made small and concise commits, the conflicts should be easy to tackle. `git status` or `git gui` will show your were the conflicts are. You can use tools like the ones listed [here](https://developer.atlassian.com/blog/2015/12/tips-tools-to-solve-git-conflicts/) to help you resolve those.

            Once you have resolved the conflicts, stage the final changes and continue the rebase. MAKE SURE NOT TO COMMIT!

            ```
            git add .
            git rebase --continue
            ```

        * If the conflicts are too complex to solve, you can abort the rebase at any time:

            ```
            git rebase --abort
            ```

            At that point, you can rethink your amending strategy based on the conflicts you discovered, and divide your recent changes in multiple commits to squash properly in a new interactive rebase attempt.

    * ***I messed up the rebase really bad without aborting, what do I do?***

        * ONLY if everything you need is on your remote (you did not lose anything locally which would not be on the web), the easiest way is to do sync up from your remote repository:

            ```
            git fetch myremote
            git reset --hard myremote/branch-name
            ```

        * If not, reflog is a mechanism which records when the tip of branches are updated. It can be used to revert/access changes still in Git's storage.

            ```
            git reflog --all
            ```

            What this shows is a list of commits that Git still has in its storage. The first 7 characters are the beginning of the commit’s SHA1 hash, followed by the commit’s pointer, action, and message. You can see that not just commits are listed: switching branches, merges, rebases, and more are shown. The important part is the commit pointer and the SHA1: once you have that, you can do a `git show` to see the diff, or perhaps a `checkout`, `cherry-pick`, or `merge` depending on how you want to get the change back into your history. - Read the full article on [gitready.com] for more information.

            > TIP: You can also use the `--all` option to get more detailed information about different branches and even the stash


* **Syncing repositories**

    If your code was never pushed before being changed then you don't have to worry about it, but if you push often (which you should be doing), then the history of your branch on your remote will be different than the one you changed locally.
    Pushing to your remote will not work for that reason, you will need to do a push force. Before you do this, make sure that:
    * you did not forget anything locally as the remote history will be ignored and replaced by your local one
    * noone else pulled your code from your remote repository and is using it for another reason that merge request testing

    The command to push for to your remote repository is:
    ```
    git push myremote feature-branch -f
    ```

    If you or someone else pulled your branch to test the merge-request on another computer, you won't be able to pull from the remote repository since the difference in history will start a merge. Instead, you can resync the local branch based on the remote branch this way:
    ```
    git fetch myremote
    git reset --hard myremote/branch-name
    ```


## Data Management

iMSTKs data sits in a separate repository https://gitlab.kitware.com/iMSTK/imstk-data managed with git LFS. It is downloaded by iMSTK's superbuild when either the testing or the examples is enabled.

### Add Data:

1. Checkout imstk-data repository. [imstk-data](https://gitlab.kitware.com/iMSTK/imstk-data).

2. Commit & push changes.

3. Copy the sha of the desired commit to pull in iMSTK's `CMake/External/External_iMSTKData.cmake` file.

4. Clean build or rebuild the iMSTKData target.

### Fetch Data:
A clean build will always acquire the latest changes. If incrementally building, go up to the superbuild and rebuild the iMSTKData target.

### Data Usage:
The path to the data directory is defined by iMSTK_DATA_ROOT which can be used in your C++ code as shown below :
```c++
imstk::MeshIO::read<TetrahedralMesh>(iMSTK\_DATA\_ROOT"/relative/path/to/mesh.vtk");
```

## Dashboards

Dashboards - powered by [CDash] - helps visualizing the latest state of iMSTK builds on Windows, MacOSx, and Linux, to avoid introducing regressions when offering changes to the toolkit:
>http://open.cdash.org/index.php?project=iMSTK

**Name** | CDash Build Group Description
--- | ---
**Nightly** | Builds running every night
**Continuous** | Builds running every time there is a change to the master branch
**Experimental** | Builds running after calling `do: test` in the comment section of a Merge Request

---
[Gitlab forking workflow]: <https://docs.gitlab.com/ce/workflow/forking_workflow.html>
[Fork]: <https://docs.gitlab.com/ce/workflow/forking_workflow.html#creating-a-fork>
[Create a merge request]: <https://docs.gitlab.com/ce/workflow/forking_workflow.html#merging-upstream>
[WIP]: <https://docs.gitlab.com/ee/user/project/merge_requests/work_in_progress_merge_requests.html>
[FeatureBranch article]: <https://martinfowler.com/bliki/FeatureBranch.html>
[gitready.com]: <http://gitready.com/advanced/2009/01/17/restoring-lost-commits.html>
[CDash]: <http://www.cdash.org/>
