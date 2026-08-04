# Git Sign-off Rules

Every commit MUST carry a DCO sign-off that **exactly matches the commit author's
name and email** — the DCO check on PRs rejects any mismatch.

- Use `git commit -s` so git fills the trailer from the configured identity, or write
  it explicitly with the author's real details:
  ```
  Signed-off-by: <author name> <author email>
  ```
- Never copy a sign-off template literally (no `Your Name <your-email@example.com>`
  placeholders) and never sign off with an identity different from `git config
  user.name` / `user.email` of the commit's author.
- Before pushing, verify with:
  ```bash
  git log origin/main..HEAD --format='%h|%an <%ae>|%(trailers:key=Signed-off-by,valueonly)' | awk -F'|' '$2!=$3'
  ```
  Any output means a commit will fail the DCO check.
