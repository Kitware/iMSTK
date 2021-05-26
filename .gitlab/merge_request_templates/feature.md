<!-- The text within this markup is a comment, and is intended to provide
guidelines to open a Pull Request for the iMSTK repository. This text will not
be part of the Pull Request. -->

<!-- See the [Contributing guidelines](https://gitlab.kitware.com/iMSTK/iMSTK/-/blob/master/CONTRIBUTING.md). Specifically:

Start commit messages with a standard prefix (and a space):

 * BUG: fix for runtime crash or incorrect result
 * COMP: compiler error or warning fix
 * DOC: documentation change
 * ENH: new functionality
 * PERF: performance improvement
 * STYLE: no logic impact (indentation, comments)
 * WIP: Work In Progress not ready for merge
 * REFAC: Refactoring the code

Provide a short, meaningful message that describes the change you made.

When the PR is based on a single commit, the commit message is usually left as
the PR message.
-->

## PR Checklist
- [ ] No API changes were made (or the changes have been approved)
- [ ] No major design changes were made (or the changes have been approved)
- [ ] No new dependecies were added (or the new dependencies have been approved)
- [ ] Added test (or behavior not changed)
- [ ] Updated API documentation (or API not changed)
- [ ] Ran the `CodeFormatEnforcer` task and verified the header is in all new files 
- [ ] Added [iMSTK examples](https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Examples) for all new major features (if any)

<!-- **Thanks for contributing to iMSTK!** -->