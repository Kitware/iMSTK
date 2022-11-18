# Release Process

The following notes describe how to perform an iMSTK release. Whilst it's not law and doesn't have to be followed precisely, it may provide some useful trip/tricks. Ammend the document as appropriate.

## 1.) Prep Release

1. All tests pass
2. Check configurations
    - C# Build
    - Norender/renderless Build (for backend usage)
    - With/without OpenHaptics, Haply, VRPN
3. Ensure wrapper generation up to date.
    - Test failure is not meant to happen if classes aren't wrapped.
4. Ensure Gitlab Pages are generated from the mirror.
    - Gitlab Pages (for doxygen code and user documentation) are built on a mirror as `gitlab.kitware.com` does not support Gitlab Pages.
    - Ensure Gitlab pipeline passing here (pages job): https://gitlab.com/imstk/imstk.gitlab.io
5. Optional:
    - Review/Run examples manually.
    - Clear warnings as possible.

## 2.) Release Notes

Begin the release notes in the `.md` format for Gitlab and mkdocs.

1. Refer to [previous releases](https://gitlab.kitware.com/iMSTK/iMSTK/-/releases) for formatting.
2. Write intro/overview paragraph with focus of improvements.
3. Write the shortlog. See separated section below for shortlog generation.
4. Write the API changes to the best of your ability. (Easier after creating/reviewing the shortlog.)
    - If something was renamed, give the new name.
    - If something was refactored, give new usage.
    - If something was replaced, give the alternative.
5. Write a thank you & list contributors.
    - Get list using `git shortlog --after="12-1-2018" --summary`.
    - Include those who worked on the test machine & non MR items that won't show up through git.

## 3.) Final Commit

1. Add release notes to the user documentation [here](??)
2. Iterate version number at the top of root CMakeLists.txt [here](https://gitlab.kitware.com/iMSTK/iMSTK/-/blob/master/CMakeLists.txt#L2)
3. Create the release tag in `gitlab.kitware.com` on that commit with accompanying release notes `.md`.

## 4.) Release Blog

1. Link the full release notes.
    2. Every major feature should have a longer description with pictures when needed.
        - Tip: Link to gifs/videos in documentation for better size in blog. Autoplay preferred to capture attention.
    3. Remove minor changes as necessary.
    4. Add funding acknowledgements.

## Git Shortlog

Note: Do not use a shallow clone. In some cases this may work depending on how far back your history is. Verify you have the full history in git locally.

Note: Grep on git shortlog only works on unix systems. Regex is an alternative, or notepad++ on windows. Or use WSL on windows.

1. Find the date of the last release in [previous releases](https://gitlab.kitware.com/iMSTK/iMSTK/-/releases). Ex: 12-1-2018
2. Use the command:

```
`git shortlog --after="4-20-2022" --no-merges --grep='STYLE' --invert-grep --format="([%h](<https://gitlab.kitware.com/iMSTK/iMSTK/-/commit/%H>)) %s"`
```

3. Command
    - This gives `.md` formatted hyperlinks for each commit.
    - Shortlog is a git feature intended for release notes. It separates commits by author and only displays the commit message. In order by date.
    - The command given above is meant to:
        - Remove commits before that release date.
        - Remove merge commits that just say what branch was merged.
        - Remove STYLE commits.
        - Format with hyperlink.
    - This requires developers to write good commit messages & separate by type with tags.
    
For example:
```
E:\iMSTKProper\iMSTK>git shortlog --after="4-18-2022"
Andrew Wilson (14):
    REFAC: Remove extra test stage that generates documentation twice
    Merge branch 'VariousFixes' into 'master'
    BUG: Fix issue with PbdObjectCollision not allowing other handlers to be set
    BUG: Fix fetch issue with template test
    Merge branch 'FixTemplateTest' into 'master'
    Merge branch 'PbdCollisionInteractionHandlerSwap' into 'master'
    Merge branch 'fix/coverage' into 'master'
    BUG: Update install project test for renderless mode testing
    Merge branch 'UpdateProjectInstallTreeSha' into 'master'
    STYLE: Uncrustify
    Merge branch 'fix/visual-testing' into 'master'
    ENH: Binary Msh reader & ascii refactor
    REFAC: Improve comments, modern c++, couple other minor improvements, add hexahedral mesh test, tests validate contents

Harald Scheirich (3):
    COMP: Remove 'typename'
    COMP: Fix coverage target
    DOC: Add documentation for coverage calc

harald.scheirich@kitware.com (2):
    BUG: fix logger handling in tests
    BUG: fix spurious failure when closing a window in visual tests on linux
```

4. Perform a final trim (the full git log can be lengthy). Remove commits from this log related to the following:
    - Commits related to minor bugs/refactors
    - (auto removed) Commits related to nonfunctional things such as style
    - (auto removed) Commits for merges
5. Cluster and group commits to the best of your ability to reduce size and create a concise list of changes. Keep authors separate to recognize contributions.
    - If there are 20 commits for one feature, group them under the one head commit. Alter description if needed.
6. Bugs are tricky, use best judgement on what bugs to report & trim.
    - For instance a bug introduced in someones MR, shows up as a commit, but is resolved before that branch is even merged. It's not really a bug fix for the release. It was never an issue on imstk-main.