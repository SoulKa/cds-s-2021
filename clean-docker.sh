#!/bin/sh
docker rm $(docker ps -a -q -f status=exited)