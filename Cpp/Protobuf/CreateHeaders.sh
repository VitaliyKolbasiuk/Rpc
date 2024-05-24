#!/usr/bin/bash

#cargo install protobuf-codegen
PATH=/usr/bin:$PATH
protoc -I=. --rust_out=. AddressBook.proto

protoc -I=. --cpp_out=. AddressBook.proto
