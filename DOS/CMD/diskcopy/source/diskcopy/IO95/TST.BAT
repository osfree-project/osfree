@echo off
type nul >longfilename1
if exist longfilename2 del longfilename2 > nul
if isdir longpathname rm -fr %@sfn[longpathname]
t.exe
