#!/bin/sh
openssl genrsa 1024 > server.key
openssl req -new -x509 -nodes -sha256 -days 1095 -key server.key > server.cert
cat server.cert server.key > server.pem
