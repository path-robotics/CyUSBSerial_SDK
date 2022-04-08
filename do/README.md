# Testing and Building using Docker and Buildkite

The scripts in this directory serve several purposes.  They help provide consistency
between your build/test results locally and the results of CI.  They also provide a quick
and simple way to build and test your code without needing to be concerned about your
local development environment.

## WARNING

Running these scripts will modify the contents of your directory.
Because the contents of this repository will be volume mounted into a Docker container and your
code will be built and tested, any changes that you wish to preserve in your artifact directories
(e.g. `build`, `logs`, `build-output`, `test-output`, etc) should be copied elsewhere prior to
running these scripts.

## Using the scripts

To test your code changes using Docker, simply run the `do/test` script, which will
build for test, with coverage, and any test results will be printed to the command line.
The `do/build` script will compile the code for Release only.  No Debian packages will be created.
`do/package` will produce a Debian package and must be executed after a successful `do/build`.
For more info on these scripts and the others in this directory please refer to the
[do/API](https://pathrobotics.atlassian.net/wiki/spaces/INFRA/pages/2253094917/do+API) Confluence Page.

## Testing on Buildkite

You can also run the CyUSBSerial tests on [Buildkite](https://buildkite.com/path-robotics/cyusbserial-sdk).
If you do not have a Buildkite account please submit an IT ticket to have your account
created.

## Making changes to Docker

If you would like to change the environment inside the Docker containers, there are a few
places that you will want to look.

1. `do/thirdparty.Dockerfile` - This is the foundational Docker image that contains most of the project's
dependencies.  If you need to change dependencies for the project this is most likely where you will make
those changes. `do/thirdparty.Dockerfile` is the default convention.  If you need a context for your
Dockerfile, use `do/thirdparty.docker.d/Dockerfile` and place the necessary files in that directory.
`do/thirdparty.Dockerfile` must be deleted in that situation.
1. `do/testdeps.Dockerfile` - This Docker image layer installs the tools necessary for testing including
SonarCloud so the code can be scanned. `do/testdeps.Dockerfile` is the default convention.  If you need a
context for your Dockerfile, use `do/testdeps.docker.d/Dockerfile` and place the necessary files in that
directory.  `do/testdeps.Dockerfile` must be deleted in that situation.
1. `do/test.Dockerfile` - This Docker image is used by the `do/test` script.  This Docker layer configures
the Docker image with your user so that build artifacts are owned by your local user.
`do/test.Dockerfile` is the default convention.  If you need a context for your Dockerfile, use
`do/test.docker.d/Dockerfile` and place the necessary files in that directory.
`do/test.Dockerfile` must be deleted in that situation.
1. `do/scan.Dockerfile` - This Docker image is used by the `do/scan` script. This Docker layer adds a few
environment variables needed to support the scan process.
1. `do/build.Dockerfile` - This Docker image is used by the `do/build` script.  This Docker layer configures
the Docker image with your user so that build artifacts are owned by your local user.
`do/build.Dockerfile` is the default convention.  If you need a context for your Dockerfile, use
`do/build.docker.d/Dockerfile` and place the necessary files in that directory.
`do/build.Dockerfile` must be deleted in that situation.
1. `do/package.Dockerfile` - This Docker image is used by the `do/package` script. This Docker layer configures
the Docker image with your user so that build artifacts are owned by your local user.
`do/package.Dockerfile` is the default convention.  If you need a context for your Dockerfile, use
`do/package.docker.d/Dockerfile` and place the necessary files in that directory.
`do/package.Dockerfile` must be deleted in that situation.
1. `do/test` - Creates the Docker images needed for testing, includes functions that contain the
bash commands to run inside the Docker container to compile for test and test the code.
1. `do/scan` - Creates the Docker images needed for scanning, includes functions that contain the
bash commands to run inside the Docker container to scan the code.  Typically this script is executed
as part of the CI pipeline on Buildkite.  You can run this script locally if you have the `SONAR_TOKEN` environment
variable set with a token from [sonarcloud.io](https://sonarcloud.io).
1. `do/build` - Creates the Docker images needed to compile the code, includes the function that contains
the bash commands to run inside the Docker container to compile the code for release.  This script
will perform an incremental compile.
1. `do/package` - Create the Docker images needed to compile the code for release and create a Debian
package. This script includes the functions that contains the bash commands to run inside the Docker
container to compile and package the Debian.  This script runs after `do/build`.
1. `do/private/docker_utils.sh` - This is where the `docker build` and `docker run` commands are defined.  If you
would like to make changes to the `docker build` or `docker run` commands, pass new options to those commands
(e.g. environment variables, build arguments, etc.) by adding them to the do_all function in your script.

## Other do/ scripts

1. `do/private/script_utils.sh` - Handy utility functions that are commonly used.  You should not need to change anything
in this script.
1. `do/private/parse_options_utils.sh` - This script handles all the options parsing for the do scripts.  You should only
need to update this script when a new do script is added to this code repository.
1. `do/lint` - a linting utility to lint bash scripts.  Globally disable warnings by adding them to
`.shellcheckrc` in the repo's root folder or disable them line by line in the bash script with the directive
`# shellcheck disable=code` in a line above the command causing the warning. Please include a comment explaining
the need to disable a warning.  See [ShellCheck](https://github.com/koalaman/shellcheck/wiki) for more information.
1. `do/clean` - this script removes the artifacts resulting from compiling the code and testing the code.  For this repo
it will remove the "build-output" and "test-output" directories.
1. `do/distclean` - this script sets the repo back to a clean state, e.g. just like when it was first cloned from GitHub.
All uncommitted changes wil be lost!
1. `do/publish` - this script will push a Debian package to the AWS S3 apt repository.  Since this is a template repo, a function
that publishes the docker images to the AWS ECR is also available as an example.  It is not actively called for this repo.
Typically a repo will publish either its docker images or a Debian, not both.  Use the workflow that is appropriate for your
repo.
1. `do/ci` - this script is meant to run locally and it will mimic the CI pipeline on Buildkite (less the scanning and publishing).
In order to accomplish this, it will run the following commands:
   - `do/clean -f`
   - `do/lint`
   - `do/test`
   - `do/build`
   - `do/package`
1. `do/auto` - launch this script when you want to automatically execute a do/ script when a file is saved.  For example, if you
want to compile the code right after you make a change and save the file, run `do/auto build` in a terminal and make your change
in another application.  Once you save the file, you'll see the build script automatically run in that terminal window.  A
`do/bash_completion.sh` script is available to assist in the tab complete of `do/auto`.  To take advantage of that helper script,
source it before the `do/auto` script like so:
```
terminal_prompt$ source do/bash_completion.sh
terminal_prompt$ do/auto build
```
