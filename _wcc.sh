#! /bin/sh
# SPDX-FileCopyrightText: (C) 2026 osFree Project
# SPDX-License-Identifier: BSD-3-Clause
#

export ROOT=.
while [ ! -f "$ROOT/tools/mk/all.mk" ]; do ROOT="$ROOT/.."; done
export PATH=$ROOT/tools/conf/scripts:$PATH
build-lnx.sh $*
