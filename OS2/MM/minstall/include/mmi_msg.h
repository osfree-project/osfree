//
// MINSTALL.DLL (c) Copyright 2002-2005 Martin Kiewitz
//
// This file is part of MINSTALL.DLL for OS/2 / eComStation
//
// MINSTALL.DLL is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
// MINSTALL.DLL is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//  along with MINSTALL.DLL.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _MMI_MSG_H_
#define _MMI_MSG_H_

#define MINSTMSG_OutOfMemory                1
#define MINSTMSG_CouldNotLoad               2
#define MINSTMSG_StringTooLong              3
#define MINSTMSG_GenericError               4
#define MINSTMSG_UnlistedFile               5
#define MINSTMSG_NoGroupsSpecified          6
#define MINSTMSG_NoDirectoriesSpecified     7
#define MINSTMSG_NoFileControlSpecified     8
#define MINSTMSG_NoFilesSpecified           9
#define MINSTMSG_InstallDLLnotFound        10
#define MINSTMSG_UnlistedScript            11
#define MINSTMSG_NoKnownGroups             12
#define MINSTMSG_IllegalAdapterCount       13
#define MINSTMSG_IllegalPromptCount        14
#define MINSTMSG_IllegalConfigSysLineCount 15
#define MINSTMSG_IllegalDriverCount        16
#define MINSTMSG_CouldNotFindSourceFile    17
#define MINSTMSG_BadSelectValue            18
#define MINSTMSG_DuplicateGroupID          19
#define MINSTMSG_DuplicateDirectoryID      20
#define MINSTMSG_BadDirectoryFlags         21
#define MINSTMSG_UnknownGroupID            22
#define MINSTMSG_UnknownSourceID           23
#define MINSTMSG_UnknownDestinID           24
#define MINSTMSG_ValueExpected             25
#define MINSTMSG_UnexpectedEndOfLine       26
#define MINSTMSG_UnexpectedEndOfFile       27
#define MINSTMSG_BadCommand                28
#define MINSTMSG_BadConfigCommand          29
#define MINSTMSG_ConfigCommandExpected     30
#define MINSTMSG_NoConfigCommandExpected   31
#define MINSTMSG_InvalidNumeric            32
#define MINSTMSG_ICBadStructure            33
#define MINSTMSG_ICBadArray                34
#define MINSTMSG_ICBadArrayInArray         35
#define MINSTMSG_ICUnknownFunction         36
#define MINSTMSG_ICUnknownParameter        37
#define MINSTMSG_ICParameterNotArray       38
#define MINSTMSG_ICParameterIsArray        39
#define MINSTMSG_ICDuplicateParameter      40
#define MINSTMSG_StringExpected            41
#define MINSTMSG_NumericValueExpected      42
#define MINSTMSG_ICMissingParameter        43
#define MINSTMSG_ICArrayTooBig             44
#define MINSTMSG_ICBadFourCC               45
#define MINSTMSG_StringTooBig              46
#define MINSTMSG_CIBadValueFor             47
#define MINSTMSG_CICouldNotLoadCustomDLL   48
#define MINSTMSG_IllegalPromptChoicesCount 49
#define MINSTMSG_ICMissingParameterWarning 50
#define MINSTMSG_CIValueTruncatedWarning   51
#define MINSTMSG_InstallDLLunlistedWarning 52
#define MINSTMSG_UnlistedScriptWarning     53

#define MINSTMSG_CouldNotFilterCFD         80
#define MINSTMSG_CouldNotReadPrompts       81
#define MINSTMSG_CouldNotCreateDirectory   82
#define MINSTMSG_CouldNotCopyToFile        83
#define MINSTMSG_CONFIGSYSGenericProblem   84
#define MINSTMSG_CONFIGSYSReadOnly         85
#define MINSTMSG_CONFIGSYSFailedBackUp     86
#define MINSTMSG_MultiDisksUnsupported     87
#define MINSTMSG_CARDINFOAutoCfgBadFormat  88
#define MINSTMSG_CARDINFOAutoCfgBadCardNo  89
#define MINSTMSG_CARDINFOAutoCfgMismatch   90
#define MINSTMSG_CARDINFOnotFound          91
#define MINSTMSG_CouldNotAccessFile        92
#define MINSTMSG_DiskFull                  93
#define MINSTMSG_CARDINFOunlistedWarning   94

#define MINSTMSG_AlreadyRunning           100
#define MINSTMSG_MMBaseNotFound           101
#define MINSTMSG_CouldNotLinkIn           102
#define MINSTMSG_PackageNotInitialized    103
#define MINSTMSG_NeedingCleanUp           104
#define MINSTMSG_BannedDriverSet          105
#define MINSTMSG_NeedingInit              106
#define MINSTMSG_GeninReplacementRequired 107

#define MINSTMSG_CLITrailer                  120
#define MINSTMSG_CLICouldNotInitPM           121
#define MINSTMSG_CLICouldNotInitMsgQueue     122
#define MINSTMSG_CLIUnknownParameter         123
#define MINSTMSG_CLICantUseInCombinationParm 124
#define MINSTMSG_CLIInvalidUsageOfParm       125
#define MINSTMSG_CLISyntaxExplanation        126
#define MINSTMSG_CLIExecutingInCIDMode       127
#define MINSTMSG_CLIResponseFileCreated      128
#define MINSTMSG_CLIResponseFileIsInvalid    129
#define MINSTMSG_CLIActivePackageIs          130
#define MINSTMSG_CLIActiveGroupIs            131
#define MINSTMSG_CLIActiveCardNoIs           132
#define MINSTMSG_CLIActivePromptIs           133
#define MINSTMSG_CLIChooseGroup              134
#define MINSTMSG_CLIChoosePrompt             135
#define MINSTMSG_CLIChooseOption             136
#define MINSTMSG_CLISureToInstallPackage     137
#define MINSTMSG_CLIIsSelected               138
#define MINSTMSG_CLISelectionForced          139
#define MINSTMSG_CLICardSelect               140
#define MINSTMSG_CLIGaveControlToGUI         141

#define MINSTMSG_CIDSuccess                  150
#define MINSTMSG_CIDSuccessReboot            151
#define MINSTMSG_CIDSuccessErrorsLogged      152
#define MINSTMSG_CIDDataResourceNotFound     153
#define MINSTMSG_CIDUnexpectedCondition      154
#define MINSTMSG_CIDNotEnoughDiskSpace       155

#endif
