# Testing and Building CyUSBSerial SDK using Docker and Buildkite

The scripts in this directory serve several purposes.  They help provide consistency
between your build/test results locally and the results of CI.  They also provide a quick
and simple way to build and test your code without needing to be concerned about your
local development environment.

## WARNING

Running these scripts will modify the contents of your CyUSBSerial directory.  
Because the contents of this repository will be volume mounted into a Docker container and your
code will be built and tested, any changes that you wish to preserve in your `build`
or `logs` directory should be copied elsewhere prior to running these scripts.

## Using the scripts

To build and test your code changes using Docker, simply run the `do/build` script, which will produce 
a debian package and build for Release.  The `do/test` script will build for test, with coverage, and
any test results will be printed to the command line.

## Testing on Buildkite

You can also run the CyUSBSerial tests on [Buildkite](https://buildkite.com/path-robotics/cyusbserial-sdk).

If you do not have a Buildkite account please submit an IT ticket to have your account
created.

## Making changes to Docker

If you would like to change the environment inside the Docker containers, there are a few
places that you will want to look.

1. `docker/Dockerfile.thirdparty` - This is the foundational Docker image that contains most of
the project's dependencies.  If you need to change dependencies for the project this
is most likely where you will make those changes.
2. `docker/Dockerfile.build` - This layer configures the Docker image with your user so that build
artifacts are owned by your local user.  You shouldn't need to make any changes in this file.
3. `docker/Dockerfile.testdeps` - This layer contains all your test dependencies, sonar scanning software, as well as tools needed to build the project.
4. `do/test` && `do/build` - This is where the Docker commands are executed.  If you
would like to make changes to the Docker commands, or the environment variables
that are passed into Docker, you can make those changes here.
5. `do/private/build_for_test_cmd` - The Sonar scan requires a special build wrapper to be used for
`C` and `C++` code.  This script executes that wrapper when desired.
6. `do/private/docker_build` - This script contains the actual build commands.  Changing the build
process should happen here.
7. `do/private/docker_scan` - This configures and executes the Sonar scan.
8. `do/private/docker_test` - This is where the tests for each project are executed.
9. `do/private/scan` - This executes the Sonar scan against the project.
10. `do/private/build_debian_cmd` - This builds the debian package and places it in the `build` directory.
11. `do/private/build_docker_images` - This builds all the necessary docker images to build, test and scan the project.
12. `do/private/package` - Build the debian package for publishing.
13. `do/private/publish` - Publish the package to the apt repo.  This should only be done by CI and only on release branches (`master / develop`)
14. `do/private/lint` - Use Shellcheck to scan scripts.  Certain rules can be excluded by adding the `--exclude=CODE1,CODE2...` format.  Infomration on error codes and their meaning can be found at the [shellcheck wiki](https://github-wiki-see.page/m/koalaman/shellcheck/wiki).  Append the error code at the end of the URL to go directly to that code's page.