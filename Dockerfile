# use CDS server as base image
FROM tudinfse/cds_server:latest

# overwrite server config
COPY ./cds_server.json /etc/cds_server.json

# install dependencies
RUN apt update
RUN apt install -y build-essential

# task 1
COPY ./tasks/mopp-2018-t0-harmonic-progression-sum /tmp/task
RUN cd /tmp/task && \
    make && \
    cp harmonic-progression-sum /usr/local/bin/ && \
    rm -rf /tmp/task