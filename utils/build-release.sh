#!/bin/bash

set -x -e

DIST=dist
STAGING=_staging_
DATE=$(date +'%Y-%m-%d')
VERSION=$1

rm -f MyMig-Pocket.zip
rm -rf ${STAGING}

pushd src/sw
make clean
make
popd

pushd src/fpga/core/mymig-rtl
python3 mymig.py
popd

quartus_sh --flow compile ./src/fpga/ap_core.qpf

cp -r ${DIST} ${STAGING}

python3 utils/reverse-bits.py src/fpga/output_files/ap_core.rbf ${STAGING}/Cores/markus-zzz.MyMig/bitstream.rbf_r

sed -i "s/VERSION/${VERSION}/" ${STAGING}/Cores/markus-zzz.MyMig/core.json
sed -i "s/DATE_RELEASE/${DATE}/" ${STAGING}/Cores/markus-zzz.MyMig/core.json

pushd ${STAGING}
zip -r ../MyMig-Pocket.zip .
popd

cat src/fpga/output_files/ap_core.fit.summary
