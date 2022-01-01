#!/bin/sh

current_wd="$(pwd)"
build_type="Release"
src_dir="$(readlink -f $(dirname $0)/..)"
build_dir="${src_dir}/build"
install_dir="${src_dir}/install"
tools_dir="${src_dir}/tools"
install_args=""

USAGE="Usage info: install.sh [OPTION(s)]
Options:
    -h, --help                Show this help
    -u, --use-cache           Use existing build cache. Cache from previous build wouldn't be deleted
    --build-type      <type>  Build type. 'Debug' or 'Realse' value is expected
    -S, --src-dir     <path>  Path to sources directory
    -B, --build-dir   <path>  Path to build directory
    -I, --install-dir <path>  Path to install direcory
    *                         All other options are passed directly to cmake"

while [ -n "$1" ]; do
  case "$1" in
    -u|--use-cache)
      install_args="${install_args} $1"
      ;;
    --build-type)
      build_type="$2"
      shift
      ;;
    -S|--src-dir)
      src_dir="$2"
      shift
      ;;
    -B|--build-dir)
      build_dir="$2"
      shift
      ;;
    -I|--install-dir)
      install_dir="$2"
      shift
      ;;
    -h|--help)
      echo "$USAGE"
      exit 0
      ;;
    *)
      install_args="${install_args} $1"
      ;;
  esac
  shift
done

install_args="${install_args} --build-type ${build_type}"
install_args="${install_args} -S ${src_dir}"
install_args="${install_args} -B ${build_dir}"
install_args="${install_args} -I ${install_dir}"

${tools_dir}/install.sh ${install_args}
cd ${build_dir}
ctest
cd ${current_wd}
