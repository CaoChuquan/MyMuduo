#!/bin/bash

set -e

PROJECT_ROOT=$(cd `dirname $0`; pwd)
BUILD_DIR=$PROJECT_ROOT/build

# 清空或创建 build
if [ -d $BUILD_DIR ]; then
    rm -rf $BUILD_DIR/*
else
    mkdir $BUILD_DIR
fi

# 编译
cd $BUILD_DIR
cmake ..
make -j$(nproc)

cd $PROJECT_ROOT

# 安装头文件
INCLUDE_DIR=/usr/include/mymuduo
sudo mkdir -p $INCLUDE_DIR
find muduo -name "*.h" -exec sudo cp {} $INCLUDE_DIR \;

# 安装库（查找 libmymuduo.so）
LIB_SRC=$(find $BUILD_DIR -name "libmymuduo*.so" | head -n 1)
if [ -n "$LIB_SRC" ]; then
    sudo cp $LIB_SRC /usr/lib/libmymuduo.so
    sudo ldconfig
    echo "Installed /usr/lib/libmymuduo.so from $LIB_SRC"
else
    # 如果没有动态库，尝试静态库
    LIB_SRC=$(find $BUILD_DIR -name "libmymuduo*.a" | head -n 1)
    if [ -n "$LIB_SRC" ]; then
        sudo cp $LIB_SRC /usr/lib/libmymuduo.a
        echo "Installed /usr/lib/libmymuduo.a (static)"
    else
        echo "Error: No library libmymuduo found in $BUILD_DIR"
        exit 1
    fi
fi

echo "Build and install complete."