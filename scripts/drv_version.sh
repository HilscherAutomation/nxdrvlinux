#/bin/bash

script_path=$(dirname $(readlink -f $0))
default_version_file="${script_path}/../cifxdrv_version.txt"

set_drv_version() {
	path_drv_version="${default_version_file}"
	version=$1

	echo "Setting driver version to:"
	echo -n ${version} | tee ${path_drv_version}
	echo ""

	exit 0
}

get_drv_version() {
	path_drv_version="${default_version_file}"

	# Prefer always git version over file. The file source is only required
	# for non-versioned driver resource like an archived export.
	git log 1>/dev/null 2>&1 &&  {
		version=$(git describe --long --tags)
		echo $version
		exit 0
	}

	[ -e ${path_drv_version} ] && {
		cat ${path_drv_version}
		exit 0
	}

	echo "Can't determine driver version!"
	echo "Please set driver version via $0 -s"

	exit 1
}

usage() {
cat <<EOF 1>&2
Usage: $0 [OPTION] ..."
   The driver components use this script to determine the current version. By default the latest git tag is returned including a hash.
   e.g. 3.0.0-98-g12abcf5

   If the driver is not provided as git repository the global driver version file is used (default=${default_version_file}).

  -s  {version}   Set the global driver version file.
  -g              Get current driver version.
EOF
    exit 1
}

while getopts "s:g" o; do
	case "${o}" in
		s) set_drv_version "${OPTARG}"
		;;
		g) get_drv_version
		;;
		*)  usage
		;;
	esac
done
