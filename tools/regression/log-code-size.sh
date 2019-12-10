#!/bin/bash
# Must be called from inside git repository

set -e

usage() {
    echo Usage: $0 [PLATFORM] >&2
    echo >&2
    echo Reports symbol size for all compilation units in a directory. >&2
    exit 1
}

[ $# == 0 -o $# == 1 ] || usage

git_dirty_str() {
    # ??? TODO: Won't work if something was added to index ?
    [[ $(git diff --shortstat 2> /dev/null | tail -n1) != "" ]] && echo "-dirty" || true
}

platform=$1
if [ "x$platform" != "x" ]; then
    host_str="--host=${platform}"
else
    platform=$(uname -m)
fi

sha=$(git rev-parse --verify HEAD)$(git_dirty_str)
commit_date=$(git show -s --format="%ct" HEAD)

top_srcdir=$(git rev-parse --show-toplevel)

if [ "x${platform_short}" = "xepiphany" -a "x${build_arch}" = "xarm" ]; then
    #Detected Parallella board.
    ldflags_str='LDFLAGS=-Wl,-T/opt/adapteva/esdk/bsps/current/fast.ldf'
fi

# If PAL_BUILDDIR is set then bootstrap and configure must have been run.
# However, no guarantee that 'make' was called.
if [ "x${PAL_BUILDDIR}" = "x" ]; then
    # Bootstrap
    PAL_BUILDDIR=$(mktemp -d /tmp/palXXXXXXXX)
    export PAL_BUILDDIR
    # Bootstrap
    (cd $top_srcdir && ./bootstrap >$PAL_BUILDDIR/boostrap.log 2>&1)

    # Configure
    cd $PAL_BUILDDIR
    $top_srcdir/configure ${ldflags_str} ${host_str} >> build.log 2>&1
    created_pal_builddir="yes"
fi

cd $PAL_BUILDDIR
# Compile
(cd src && make -j -k >> build.log 2>&1 || true)

# Output results in CSV format. Must match database scheme
files=$(find src -name "*.o" | grep -v "\.libs" | sort)
for f in $files; do
    (echo $commit_date,$sha,$f,$platform,$CFLAGS && readelf -s $f) | gawk '
    {
        if (NR==1) {
            oldfs=FS;
            FS=",";
            $0=$0;
            ts=$1;
            sha=$2;
            file=$3;
            platform=$4;
            cflags=$5;
            FS=oldfs;
            next;
        }
        size = $3;
        type = $4;
        symbol = $8;
        # NOTYPE is for Epiphany toolchain bug
        if ((type == "FUNC" || type == "OBJECT" || type == "NOTYPE") && size > 0) {
            printf("%s,%s,%s,%s,%s,%s,%s,%d\n", ts, sha, platform, file, symbol, cflags, type, size);
        }
    }; '
done

cd $top_srcdir

# Invoke benchmark.
# HACK: Redirect stdout to stderr
# TODO: Append to db in this script
if [ -e $PAL_TOOLS/regression/benchmark.sh ]; then
    $PAL_TOOLS/regression/benchmark.sh $platform 1>&2 || true
fi

if [ "x${created_pal_builddir}" = "xyes" ]; then
    rm -rf ${PAL_BUILDDIR}
fi
