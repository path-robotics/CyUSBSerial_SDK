#!/usr/bin/env bash
# shellcheck disable=SC2034
# Disable unused variable check for configure_environment function

DEFAULT_AWS_REGION="us-east-2"

function show_stack() {
  local frame=0
  while caller "${frame}"; do
    ((frame++));
  done
  echo "$*"
}

function exit_on_fail() {
  if [ "${1}" -ne 0 ]; then
    show_stack "Exiting due to error code '${1}'" 1>&2
    exit "${1}"
  fi
}

function exit_on_fail_with_action() {
  if [ "${1}" -ne 0 ]; then
    show_stack "Exiting due to error code '${1}'" 1>&2
    "${2}"
    exit "${1}"
  fi
}

function aws_login() {
  local cli_version
  cli_version=$(aws --version | cut -d'/' -f2 | cut -d'.' -f1)
  local aws_region=${1:-${DEFAULT_AWS_REGION}}

  if [[ "${cli_version}" == "2" ]]; then
    aws ecr get-login-password --region "${aws_region}" | docker login --username AWS --password-stdin "335509591822.dkr.ecr.${aws_region}.amazonaws.com"
  else
    eval "$(aws ecr get-login --no-include-email --region "${aws_region}")"
  fi
}

function check_env_var_with_exit() {
  local cache_set_flags=$-
  set +e

  ENV_VAR=$(printenv "${1}")
  if [[ -z "${ENV_VAR}" ]]; then
    echo "Environment variable '${1}' is not set.  Stopping build." >&2
    exit 1
  fi

  if [[ ${cache_set_flags} =~ e ]]; then
    # reset the -e flag since it was set previously
    set -e
  fi
}

function write_s3_auth_file() {
  echo "Writing s3auth.conf file..."
  S3_AUTH_FILE=/etc/apt/s3auth.conf

  if [[ ! -f "${S3_AUTH_FILE}" ]]; then
    check_env_var_with_exit AWS_ACCESS_KEY
    check_env_var_with_exit AWS_SECRET_ACCESS_KEY

    echo "creating s3auth.conf file"

    cat <<EOF > s3auth.conf
AccessKeyId=${AWS_ACCESS_KEY}
SecretAccessKey=${AWS_SECRET_ACCESS_KEY}
Region=us-east-2
Token=''
EOF

    S3_AUTH_FILE="s3auth.conf"
  fi
}

function remove_s3_auth_file() {
  # strip off any path since we only want to remove the locally created file and
  # not the file in /etc/apt/
  local file_name
  file_name=$( basename "${S3_AUTH_FILE-}" )
  if [[ -f "${file_name}" ]]; then
    echo "Removing ${file_name} file."
    rm -f "${file_name}"
  fi
}

function configure_environment() {
  AWS_REGION=${1:-${DEFAULT_AWS_REGION}}

  PROJECT_REPO=$(basename "$(git config --get remote.origin.url)" .git | tr '[:upper:]' '[:lower:]')
  PROJECT_REPO_URL=$(git config --get remote.origin.url)
  PROJECT_VERSION=$(git describe --tags "$(git rev-list --tags --max-count=1)" > /dev/null 2>&1 || echo 0.0)
  DEFAULT_BRANCH=$(git remote show "${PROJECT_REPO_URL}" | grep 'HEAD branch' | cut -d' ' -f5)

  GIT_COMMIT=$(git rev-parse --short HEAD)
  GIT_HASH_TAG=git-${GIT_COMMIT}
  GIT_VER_TAG=$(git tag --points-at)
  GIT_BRANCH=${BUILDKITE_BRANCH-}
  GIT_BRANCH=${GIT_BRANCH:=$(git rev-parse --abbrev-ref HEAD)}

  USER_ID=$(id -u)
  GROUP_ID=$(id -g)
}

function create_aws_ecr_repo() {
  aws --region "${AWS_REGION}" ecr create-repository \
      --repository-name "${1}" \
      --image-scanning-configuration scanOnPush=true \
      > /dev/null

  aws --region "${AWS_REGION}" ecr put-lifecycle-policy \
      --repository-name "${1}" \
      --lifecycle-policy-text "file://${SCRIPT_DIR}/private/ecr_life_cycle_policy.json" \
      > /dev/null
}
