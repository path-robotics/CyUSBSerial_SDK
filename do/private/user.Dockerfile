ARG PROJECT_REPO
ARG DOCKER_VER_IMG
ARG DOCKER_TYPE
FROM ${PROJECT_REPO}:${DOCKER_TYPE}-${DOCKER_VER_IMG}

ARG USER_ID
ARG GROUP_ID

ENV USER_ID=${USER_ID}
ENV GROUP_ID=${GROUP_ID}

RUN if ! getent group ${GROUP_ID} >/dev/null 2>&1; then \
      groupadd -g ${GROUP_ID} path; \
    fi
RUN if ! id ${USER_ID} >/dev/null 2>&1; then \
      useradd -lm --shell /bin/bash -u ${USER_ID} -g ${GROUP_ID} path; \
    else \
      usermod -a -G ${GROUP_ID} $(id -nu ${USER_ID}); \
    fi

USER ${USER_ID}
