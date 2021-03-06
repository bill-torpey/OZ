#!/bin/bash -x

# get script directory
SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE}[0]) && pwd)

# get cmd line params
while getopts ':s:b:c:i:v' flag; do
  case "${flag}" in
    s) export SUFFIX="${OPTARG}"          ;;
    b) export BUILD_TYPE="${OPTARG}"      ;;
    c) export CONFIG="${OPTARG}"          ;;
    i) export INSTALL_BASE="${OPTARG}"    ;;
    v) export VERBOSE=1                   ;;
  esac
done
shift $(($OPTIND - 1))

# source dependencies
source ${SCRIPT_DIR}/dependencies

# certain build types imply a particular configuration
[[ ${BUILD_TYPE} == *san ]] && export CONFIG=clang
# defaults for dev
[[ -z ${INSTALL_BASE} ]] && export INSTALL_BASE="${HOME}/install"
[[ -z ${BUILD_TYPE} ]]   && export BUILD_TYPE="dev"

#################################################################
# look for files of the form devenv.$user.$host.sh and devenv.$user.sh
#################################################################
rc=0
HOST=`hostname -s`
if [[ -e ${SCRIPT_DIR}/devenv.${USER}.${HOST}.sh ]]; then
	echo "using ${SCRIPT_DIR}/devenv.${USER}.${HOST}.sh"
	source ${SCRIPT_DIR}/devenv.${USER}.${HOST}.sh
	rc=$?
elif 	[[ -e ${SCRIPT_DIR}/devenv.${USER}.sh ]]; then
	echo "using ${SCRIPT_DIR}/devenv.${USER}.sh"
	source ${SCRIPT_DIR}/devenv.${USER}.sh
	rc=$?
fi

#################################################################
# everything from here down depends on variables set above
#################################################################
# use above to set environment
[[ $rc -ne 255 ]] && source ${SCRIPT_DIR}/devenv.common.sh
