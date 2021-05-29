#! /bin/bash
# @file build.sh
# @brief script to build cplex_ilp
# idea is to figure out which of several possible directories the cplex and concert libraries
# are in and set the appropriate variables for running make; then run make

# check if executable exists and ask if it should be replaced
if [ -e $HOME/bin/cplex_ilp ]; then
    echo -n "$HOME/bin/cplex_ilp exists. (Aa)bort [default], (Cc)ompile only, (Rr)eplace? "
    read action
    # continue if action is compile or replace
    case $action in
        R | r)
            echo "Replacing existing executable in $HOME/bin"
            ;;
        C | c)
            echo "Compiling source code but not replacing executable in $HOME/bin"
            ;;
        *)
            echo "Compilation aborted."
            exit 1
            ;;
    esac
fi

# find correct root directory for all include and lib files
if [ -d /afs/eos.ncsu.edu/dist/ilog/ ]; then
    # cplex in afs space
    root_dir=/afs/eos.ncsu.edu/dist/ilog/
elif [ -d /Applications/CPLEX_Studio129/ ]; then
    # on a Mac (version 12.9)
    root_dir=/Applications/CPLEX_Studio129/
elif [ -d /opt/ibm/ILOG/CPLEX_Studio129/ ]; then
    # generic Linux, e.g., tdgoodrich server
    root_dir=/opt/ibm/ILOG/CPLEX_Studio129/
elif [ -d /usr/local/apps/cplex/ILOG03/ ]; then
    # latest available on HPC (unsupported)
    root_dir=/usr/local/apps/cplex/ILOG03/
fi

# find where architecture-specific lib directory lives
if [ -d $root_dir/cplex/lib/x86-64_sles10_4.1/ ]; then
    # vcl servers
    arch=x86-64_sles10_4.1
elif [ -d $root_dir/cplex/lib/x86-64_osx/ ]; then
    # Mac
    arch=x86-64_osx
elif [ -d $root_dir/cplex/lib/x86-64_linux/ ]; then
    # generic linux
    arch=x86-64_linux
fi

# check to see if cplexdistmip library exists (in version 12.9)
if [ -e $root_dir/cplex/lib/$arch/static_pic/libcplexdistmip.a ]; then
    distmip=-lcplexdistmip
fi

make ROOT_DIR=$root_dir SYSTEM=$arch DISTMIP=$distmip

echo "Compilation successful, executable in current directory."

case $action in
    C | c)
        echo "Not copying to $HOME/bin"
        exit 1
        ;;
    R | r)
        echo "Replacing executable in $HOME/bin"
        ;;
esac

if [ -d $HOME/bin ]; then
    make install
else
    echo "Cannot install, $HOME/bin does not exist; mv cplex_ilp to another directory"
fi

#  [Last modified: 2021 05 29 at 15:56:49 GMT]
