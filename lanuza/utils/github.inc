#!/usr/bin/env bash

# Gets the files modified by a pull request
function github_get_pr_files() {
  local pr=$1
  # TODO: no more than 3000 files can be identified using GitHub api
  # XXX disable pager (in case the user has exported this env)
  PAGER= gh.sh api graphql --paginate -F owner='{owner}' -F repo='{repo}' -F pr="${pr}" -f query='
  query($owner: String!, $repo: String!, $pr: Int!, $endCursor: String) {
    repository(owner: $owner, name: $repo) {
      pullRequest(number: $pr) {
        files(first: 100, after: $endCursor) {
          pageInfo{ hasNextPage, endCursor }
          nodes {
            path
          }
        }
      }
    }
  }' --jq '.data.repository.pullRequest.files.nodes.[].path'
}

# Gets the files modified between two refs (branches, tags, commits...)
# Usage: 
#   github_get_branches_files main feature1 
function github_get_branches_files() {
  local BASE=$1
  local HEAD=$2
  PAGER= gh.sh api --paginate "/repos/{owner}/{repo}/compare/${BASE}...${HEAD}" --jq '.files.[].filename'
}
