#/bin/bash

git log 1>/dev/null 2>&1 &&  {
	VERSION=$(git describe --long --tags)
	echo "#define UIO_NETX_VERSION \"${VERSION}"\" > uio_netx_version.h
	exit 0
}

if [ ! -e uio_netx_version.h ]; then
	echo "No git repository - can't determine uio_netx version!"
	echo "Please set version via version file 'uio_netx_version.h' (e.g. '#define UIO_NETX_VERSION \"3.0.0\"')"
	exit 1
}

exit 0
