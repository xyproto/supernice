#!/bin/sh
ver=1.0.0
mkdir supernice-$ver
cp -v main.c Makefile COPYING README.md supernice-$ver/
tar zcvf supernice-$ver.tar.gz supernice-$ver/
rm -r supernice-$ver/
