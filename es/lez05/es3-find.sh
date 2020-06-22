#!/bin/bash
find /home/ -type f -mmin -4 -exec grep -l "EDIT" '{}' ';'
