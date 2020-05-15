#!/usr/bin/env bash

BRANCH=$(git branch | grep \* | cut -d ' ' -f2)
DATE_LAST_COMMIT=$(git --no-pager log -1 --format=%cd)
DATE_NO_DST=$(echo $DATE_LAST_COMMIT | awk '{$NF="";sub(/[ \t]+$/,"")}1')
DATE=$(date --date="$DATE_NO_DST" +%Y%m%d)


if [[ "$BRANCH" == "dev" ]]; then
  echo "${DATE}dev"
else
  echo "${DATE}"
fi
