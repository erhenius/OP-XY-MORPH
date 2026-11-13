#!/bin/bash
cd -- "$(dirname -- "$0")/builder" || exit 1
exec ./Open_XY_Machine_Manager.command
