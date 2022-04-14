#! /bin/sh
#
#

svn cleanup
svn up

while [ $? != 0 ]; do
  svn cleanup
  svn up
done
