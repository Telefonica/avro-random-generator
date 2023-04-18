FROM gradle:8.1-jdk11

RUN apt-get update && \
  apt-get install -y python3.6 python3-pip jq gettext-base

RUN python3 -m pip install deepmerge

WORKDIR /opt/app

CMD ["/bin/bash"]
