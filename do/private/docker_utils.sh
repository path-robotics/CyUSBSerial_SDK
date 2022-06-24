#!/usr/bin/env bash

# arguments to this function: docker build arguments followed by the Dockerfile
function docker_build()
{
  local args=("$@")
  local docker_file="${args[-1]}"
  unset 'args[-1]'

  DOCKER_BUILDKIT=1 \
  docker build "${args[@]}" - < "${docker_file}"
}

# arguments to this function: docker build arguments followed by the docker directory
function docker_build_with_context()
{
  local args=("$@")
  local docker_dir="${args[-1]}"
  unset 'args[-1]'

  DOCKER_BUILDKIT=1 \
  docker build "${args[@]}" "${docker_dir}"
}

# arguments to this function: docker build arguments followed by the docker image
function build_and_tag_docker_image()
{
  local args=("$@")
  local docker_image="${args[-1]}"
  unset 'args[-1]'
  local docker_image_with_tag="${docker_image}-${GIT_HASH_TAG}"
  local docker_image_with_ver_tag="${docker_image}-${GIT_VER_TAG}"
  local docker_type
  docker_type="${docker_image##*:}"
  local docker_file="${SCRIPT_DIR}/${docker_type}.Dockerfile"
  args+=(--tag "${docker_image_with_tag}")
  args+=(--build-arg BUILDKIT_INLINE_CACHE=1)

  if [[ -f "${docker_file}" ]]; then
    docker_build "${args[@]}" "${docker_file}"
  else
    docker_build_with_context "${args[@]}" "${SCRIPT_DIR}/${docker_type}.docker.d/"
  fi


  if [[ -n "${GIT_VER_TAG}" ]]; then
    docker tag "${docker_image_with_tag}" "${docker_image_with_ver_tag}"
  fi

}

# must be called after calling configure_environment in script_utils file
function build_docker_thirdparty()
{
  local docker_image="${PROJECT_REPO}:thirdparty"
  # note: order is important for --cache-from.  use the current git branch tag first
  #       then fall back to the default branch tag.  this allows release-* branches to use
  #       the cache associated with the release instead of using the default/develop branch
  local docker_args=(--cache-from "${DOCKER_REGISTRY_HOST}/${docker_image}-${GIT_BRANCH}" \
                     --cache-from "${DOCKER_REGISTRY_HOST}/${docker_image}-${DEFAULT_BRANCH}" \
                     --progress plain \
                     "$@")

  build_and_tag_docker_image "${docker_args[@]}" "${docker_image}"
}


# must be called after calling configure_environment in script_utils file
# must be called after calling build_docker_third_party() above
function build_docker_testdeps()
{
  local docker_image="${PROJECT_REPO}:testdeps"
  local docker_args=(--build-arg DOCKER_VER_IMG="${GIT_HASH_TAG}" \
                     --build-arg SONAR_TOKEN="${SONAR_TOKEN-}" \
                     --build-arg GIT_BRANCH="${GIT_BRANCH}" \
                     --build-arg PROJECT_VERSION="${PROJECT_VERSION}" \
                     --build-arg PROJECT_REPO="${PROJECT_REPO}" \
                     --progress plain \
                     "$@")

  build_and_tag_docker_image "${docker_args[@]}" "${docker_image}"
}


# must be called after calling configure_environment in script_utils file
function build_docker()
{
  local docker_image="${1}"
  local docker_args=(--build-arg PROJECT_REPO="${PROJECT_REPO}" \
                     --build-arg DOCKER_VER_IMG="${GIT_HASH_TAG}" \
                     --build-arg GIT_BRANCH="${GIT_BRANCH}" \
                     --build-arg GIT_COMMIT="${GIT_COMMIT}" \
                     --progress plain)

  build_and_tag_docker_image "${docker_args[@]}" "${docker_image}"


  local docker_type
  docker_type="${docker_image##*:}"
  local docker_user_image_with_tag="${docker_image}-user-${GIT_HASH_TAG}"
  local docker_user_image_with_ver_tag="${docker_image}-user-${GIT_VER_TAG}"
  local docker_user_args=(--build-arg PROJECT_REPO="${PROJECT_REPO}" \
                          --build-arg DOCKER_VER_IMG="${GIT_HASH_TAG}" \
                          --build-arg DOCKER_TYPE="${docker_type}" \
                          --build-arg USER_ID="${USER_ID}" \
                          --build-arg GROUP_ID="${GROUP_ID}" \
                          --progress plain \
                          --tag "${docker_user_image_with_tag}")

  docker_build "${docker_user_args[@]}" "${SCRIPT_DIR}/private/user.Dockerfile"

  if [[ -n "${GIT_VER_TAG}" ]]; then
    docker tag "${docker_user_image_with_tag}" "${docker_user_image_with_ver_tag}"
  fi
}


function run_docker()
{
  local args=("$@")
  local docker_image="${args[-1]}"
  unset 'args[-1]'
  local docker_func_cmd="${args[-1]}"
  unset 'args[-1]'
  local docker_user_image_with_tag="${docker_image}-user-${GIT_HASH_TAG}"
  [[ ! ${BUILDKITE-} ]] && args+=(--tty)

  docker run \
        "${args[@]}" \
        --rm \
        --env INSIDE_DOCKER_CMD="${docker_func_cmd}" \
        "${docker_user_image_with_tag}" \
        "${SCRIPT_SHORT_DIR}/${SCRIPT_NAME}"
}
