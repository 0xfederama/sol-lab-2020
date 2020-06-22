#!/bin/bash
cut -d":" -f 1,6 /etc/passwd | grep "/home/" | LC_ALL=C sort | tr ':' ' '
