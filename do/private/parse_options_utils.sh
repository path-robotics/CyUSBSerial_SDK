#!/usr/bin/env bash

function usage_default()
{
  echo " "
  echo "Usage:"
  echo " do/${SCRIPT_NAME}"
  echo " do/${SCRIPT_NAME} --docker-only"
  echo " do/${SCRIPT_NAME} --no-docker"

  for STEP in "${DOCKER_STEPS[@]}"; do
    echo " do/${SCRIPT_NAME} --${STEP}-only"
    echo " do/${SCRIPT_NAME} [--no-docker] --no-${STEP}"
  done

  echo " "
  echo " do/${SCRIPT_NAME} --help|-h"
  echo " do/${SCRIPT_NAME} --usage"
  echo " "
}

function help_default()
{
  usage

  if [[ "${DOCKER_STEPS[*]}" = "build test" && ${#DOCKER_STEPS[@]} -eq 2 ]]; then
    cat <<EOF

Compile code and run unit tests.

Options:
     --docker-only    Create the thirdparty docker, testdeps docker, and build
                      docker images.  Do not compile code for test or run unit
                      tests.  Cannot be combined with other options.
     --no-docker      Do not create any docker images.  If only option then
                      compile code for test and run unit tests.
                      Can be combined with --no-build and --no-test.

     --build-only     Compiles code for test only.  No docker builds, no
                      running unit tests.
     --no-build       Runs unit tests only.  No compile code for test. Default
                      behavior will create docker images.  If combined with
                      --no-docker, then docker images are not created.

     --test-only      Runs unit tests only.  No docker builds, no compile code
                      for test.
     --no-test        Compiles code for test.  No running unit tests.  Default
                      behavior will create docker images.  If combined with
                      --no-docker, then docker images are not created.
EOF
  elif [[ "${DOCKER_STEPS[*]}" = "scan" && ${#DOCKER_STEPS[@]} -eq 1 ]]; then
    cat <<EOF

Scan code.

Options:
     --docker-only    Create the scan docker.  Do not scan the code.
                      Cannot be combined with other options.
     --no-docker      Do not create any docker images.  If only option then
                      scan the code. Can be combined with --no-scan.

     --scan-only      Scans code only.  No docker builds.
     --no-scan        No scanning code.  Default behavior will create docker
                      images.  If combined with --no-docker then no actions
                      are taken.
EOF
  elif [[ "${DOCKER_STEPS[*]}" = "build" && ${#DOCKER_STEPS[@]} -eq 1 ]]; then
    cat <<EOF

Compile code.

Options:
     --docker-only    Create the thirdparty docker and build docker images.
                      Does not compile code. Cannot be combined with other
                      options.
     --no-docker      Do not create any docker images.  If only option then
                      compile code.  Can be combined with --no-build.

     --build-only     Compiles code only - no docker builds.
     --no-build       Does not compile code. Default behavior will create
                      docker images.  If combined with --no-docker then no
                      actions are taken (no compile, no docker build).
EOF
  elif [[ "${DOCKER_STEPS[*]}" = "package" && ${#DOCKER_STEPS[@]} -eq 1 ]]; then
    cat <<EOF

Compile code for release and package Debian file.

Options:
     --docker-only    Create the thirdparty docker and build docker images.
                      Do not compile code for release or package Debian file.
                      Cannot be combined with other options.
     --no-docker      Do not create any docker images.  If only option then
                      compile code for release and package the Debian file.
                      Can be combined with --no-package.

     --package-only   Compiles code for release only and packages Debian file.
                      No docker builds.
     --no-package     Does not compile code or package Debian file. Default
                      behavior will create docker images.  If combined with
                      --no-docker then no actions are taken (no compile,
                      no package, no docker build).
EOF
  else
    cat <<EOF

Add your specific help message here for your custom step.

Options:
     --docker-only    Create the docker images only. No step executed.
     --no-docker      Do not create the docker images.  Only execute the step.

     --<step>-only    Only execute the step. No docker builds.
     --no-step        Do not execute the step.  Default implementation will
                      build the docker images.
EOF
  fi

  cat <<EOF

 -h, --help           Display this help.
     --usage          Display only the Usage section of this help.

EOF
}

function invalid_option_combo_echo_with_exit()
{
  echo "${1} is an invalid option combination!" >&2
  help
  exit 1
}

function invalid_option_echo_with_exit()
{
  echo "${1} is an invalid option!" >&2
  help
  exit 1
}

function do_all()
{
  if [ "${BASH_VERSINFO:-0}" -lt 4 ]; then
    echo "This script requires Bash version >= 4";
    exit 1;
  fi

  local args=("$@")
  DOCKER_IMAGE="${args[-1]}"

  if [[ "${DOCKER_IMAGE}" != "--" ]]; then
    unset 'args[-1]'
  fi

  local idx=0
  for arg in "${args[@]}"; do
    if [[ "${arg}" == "--" ]]; then
      break
    else
      ((++idx))
    fi
  done

  DOCKER_STEPS=("${args[@]::${idx}}")
  DOCKER_OPTIONS=("${args[@]:${idx}+1}")

  parse_command_line_options
  execute_steps
}

function parse_command_line_options()
{
  # create an associative array for the possible options
  declare -A APPROVED_OPTIONS
  APPROVED_OPTIONS[--docker-only]=0
  APPROVED_OPTIONS[--no-docker]=0
  APPROVED_OPTIONS[--help]=0
  APPROVED_OPTIONS[-h]=0
  APPROVED_OPTIONS[-?]=0
  APPROVED_OPTIONS[--usage]=0
  for STEP in "${DOCKER_STEPS[@]}"; do
    APPROVED_OPTIONS["--${STEP}-only"]=0
    APPROVED_OPTIONS["--no-${STEP}"]=0
  done

  # set the approved options based on the options passed into the script, and
  # if there is an option that is not approved print error message, print help,
  # then exit
  for OP in "${COMMAND_LINE_OPTIONS[@]}"; do
    if [[ ${APPROVED_OPTIONS["${OP}"]-} ]]; then
      APPROVED_OPTIONS["${OP}"]=1
    else
      invalid_option_echo_with_exit "${OP}"
    fi
  done

  # if help or usage should be shown, do that then exit
  if [[ "${APPROVED_OPTIONS[--help]}" -eq 1 || "${APPROVED_OPTIONS[-h]}" -eq 1 || "${APPROVED_OPTIONS[-?]}" -eq 1 ]]; then
    help
    exit 0
  elif [[ "${APPROVED_OPTIONS[--usage]}" -eq 1 ]]; then
    usage
    exit 0
  fi

  # now look for bad combinations
  if [[ "${APPROVED_OPTIONS[--docker-only]}" -eq 1 && "${APPROVED_OPTIONS[--no-docker]}" -eq 1 ]]; then
    invalid_option_combo_echo_with_exit "--docker-only and --no-docker"
  fi

  for STEP in "${DOCKER_STEPS[@]}"; do
    if [[ "${APPROVED_OPTIONS["--${STEP}-only"]}" -eq 1 && "${APPROVED_OPTIONS["--no-${STEP}"]}" -eq 1 ]]; then
      invalid_option_combo_echo_with_exit "--${STEP}-only and --no-${STEP}"
    elif [[ "${APPROVED_OPTIONS["--${STEP}-only"]}" -eq 1 && "${APPROVED_OPTIONS["--docker-only"]}" -eq 1 ]]; then
      invalid_option_combo_echo_with_exit "--${STEP}-only and --docker-only"
    fi
  done

  local num_step_only_options=0
  for STEP in "${DOCKER_STEPS[@]}"; do
    if [[ "${APPROVED_OPTIONS["--${STEP}-only"]}" -eq 1 ]]; then
      ((++num_step_only_options))
    fi
  done
  if [[ "${num_step_only_options}" -gt 1 ]]; then
    invalid_option_combo_echo_with_exit "--<step_1>-only and --<step_2>-only"
  fi

  # after filtering out all the invalid options, bad combinations, and showing
  # the help/usage, filter the steps to execute
  BUILD_DOCKER=1
  EXE_DOCKER_STEPS=("${DOCKER_STEPS[@]}")
  if [[ "${APPROVED_OPTIONS[--docker-only]}" -eq 1 ]]; then
    EXE_DOCKER_STEPS=()
  elif [[ "${APPROVED_OPTIONS[--no-docker]}" -eq 1 ]]; then
    BUILD_DOCKER=0
  fi

  for STEP in "${DOCKER_STEPS[@]}"; do
    if [[ "${APPROVED_OPTIONS["--${STEP}-only"]}" -eq 1 ]]; then
      EXE_DOCKER_STEPS=("${STEP}")
      BUILD_DOCKER=0
    fi
  done

  for STEP in "${DOCKER_STEPS[@]}"; do
    if [[ "${APPROVED_OPTIONS["--no-${STEP}"]}" -eq 1 ]]; then
      for IDX in "${!EXE_DOCKER_STEPS[@]}"; do
        if [[ "${EXE_DOCKER_STEPS[${IDX}]}" == "${STEP}" ]]; then
          unset "EXE_DOCKER_STEPS[${IDX}]"
          break
        fi
      done
    fi
  done
}

function execute_steps()
{
  # execute pre-docker commands
  pre_docker

  # build the docker image(s)
  if [[ "${BUILD_DOCKER}" -eq 1 ]]; then
    echo "--- :docker: Building the ${DOCKER_IMAGE} docker image for '${PROJECT_REPO-}' - branch '${GIT_BRANCH-}', commit '${GIT_COMMIT-}'"
    build_docker "${DOCKER_IMAGE}"
  fi

  # execute each step inside the docker
  for STEP in "${EXE_DOCKER_STEPS[@]}"; do
    run_docker "${DOCKER_OPTIONS[@]}" "do_${STEP}_in_docker" "${DOCKER_IMAGE}"
  done

  # execute the post-docker commands
  post_docker
}


# shellcheck source=do/private/script_utils.sh
source "${SCRIPT_DIR}/private/script_utils.sh"
# shellcheck source=do/private/docker_utils.sh
source "${SCRIPT_DIR}/private/docker_utils.sh"

# cache the options passed into the calling do script
COMMAND_LINE_OPTIONS=("$@")

if [[ -z "${INSIDE_DOCKER_CMD-}" ]]; then
  # NOT running inside docker
  configure_environment
else
  # Running inside docker
  # run the command given as the environment variable on the `docker run` command
  "${INSIDE_DOCKER_CMD}"
  exit "$?"
fi
