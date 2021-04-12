#!/bin/sh
set -e
PORT=7227
docker run --name cds-lab-server --rm -d -eCDS_PORT=$PORT -p$PORT:$PORT cds-lab-image