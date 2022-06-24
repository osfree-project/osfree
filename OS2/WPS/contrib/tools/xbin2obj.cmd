/*
 *      XBIN2OBJ.CMD - V1.0 C.Langanke 1997,1999 - bugfree clone of Peter Kobaks BIN2OBJ
 *
 *    Usage: XBIN2OBJ [options] binfile symbolname [objfile]
 *
 *       binfile    = name of the existing binary file.
 *       symbolname = symbolic name used in the object file.
 *       objfile    = name of the new object file; if not present.
 *                      the object file name will be same as the
 *                      binfile with an .obj extenstion.
 *       options    = any of the following:
 *         -n       = size of the binfile, as 32-bit value, will be put in front
 *                      of the binary block so it's size can be queried.
 *         -sName   = name of the segment; by default it is 'DATA32'.
 *         -cClass  = class of the segment; by default it is 'DATA'.
 *                      The recommended classes are 'DATA', 'CONST', or 'CODE'.
 *
 */
/* first comment is used as help text */

 SIGNAL ON HALT

 TitleLine = STRIP(SUBSTR(SourceLine(2), 3));
 PARSE VAR TitleLine CmdName'.CMD 'Info
 Title     = CmdName Info

 env          = 'OS2ENVIRONMENT';
 TRUE         = (1 = 1);
 FALSE        = (0 = 1);
 Redirection  = '> NUL 2>&1';
 '@ECHO OFF'

 /* OS/2 errorcodes */
 ERROR.NO_ERROR           =  0;
 ERROR.INVALID_FUNCTION   =  1;
 ERROR.FILE_NOT_FOUND     =  2;
 ERROR.PATH_NOT_FOUND     =  3;
 ERROR.ACCESS_DENIED      =  5;
 ERROR.NOT_ENOUGH_MEMORY  =  8;
 ERROR.INVALID_FORMAT     = 11;
 ERROR.INVALID_DATA       = 13;
 ERROR.NO_MORE_FILES      = 18;
 ERROR.WRITE_FAULT        = 29;
 ERROR.READ_FAULT         = 30;
 ERROR.GEN_FAILURE        = 31;
 ERROR.INVALID_PARAMETER  = 87;

 GlobalVars = 'Title CmdName env TRUE FALSE Redirection ERROR.';
 SAY;

 /* show help */
 ARG Parm .
 IF ((Parm = '') | (POS('?', Parm) > 0)) THEN
 DO
    rc = ShowHelp();
    EXIT(ERROR.INVALID_PARAMETER);
 END;

 /* Defaults */
 GlobalVars = GlobalVars 'Seg.';
 Seg.ClassName   = 'DATA';
 Seg.SegmentName = 'DATA32';
 Seg.TypeName    = 'FLAT';
 Seg.WriteSize   = FALSE;
 BinFile         = '';
 SymbolName      = '';
 ObjFile         = '';

 /* get parms */

 PARSE ARG Parms
 DO i = 1 TO WORDS( Parms)
    ThisParm = WORD( Parms, i);
    PARSE VAR ThisParm ThisTag +2 ThisValue;
    ThisTag   = TRANSLATE( ThisTag);
    SELECT
       WHEN (ThisTag = '-N') THEN Seg.WriteSize   = TRUE;
       WHEN (ThisTag = '-S') THEN Seg.SegmentName = ThisValue;
       WHEN (ThisTag = '-C') THEN Seg.ClassName   = ThisValue;
       OTHERWISE
       DO
          /**/ IF (BinFile = '') THEN
             BinFile = ThisParm;
          ELSE IF (SymbolName = '') THEN
             SymbolName = ThisParm;
          ELSE IF (ObjFile = '') THEN
             ObjFile = ThisParm;
          ELSE
          DO
             SAY CmdName': error: Invalid parameter ®'ThisParm'¯ specified.';
             EXIT( ERROR.INVALID_PARAMETER);
          END;
       END;
    END;
 END;

 IF (SymbolName = '') THEN
 DO
    rc = ShowHelp();
    EXIT(ERROR.INVALID_PARAMETER);
 END;

 /* determine object file name */
 IF (ObjFile = '') THEN
 DO
    ObjFile = FILESPEC('N', BinFile);
    ExtPos = LASTPOS( '.', ObjFile);
    IF (ExtPos \= 0) THEN
       ObjFile = LEFT( BinFile, ExtPos - 1);
    ELSE
       ObjFile = BinFile;
    ObjFile = ObjFile'.obj';
 END;

 /* check source file */
 IF (\FileExist( BinFile)) THEN
 DO
    SAY CmdName': Error: File ®'BinFile'¯ not found.';
    EXIT( ERROR.FILE_NO_FOUND);
 END;

 /* delete target file */
 'IF EXIST' ObjFile 'DEL' ObjFile Redirection;
 IF (FileExist( ObjFile)) THEN
 DO
    SAY CmdName': Error: File ®'ObjFile'¯ cannot be written.';
    EXIT( ERROR.WRITE_FAULT);
 END;

 /* generate object code */
 rc = GenerateObj( BinFile, SymbolName, ObjFile);


 EXIT(ERROR.NO_ERROR);

/* ------------------------------------------------------------------------- */
HALT:
 SAY 'Interrupted by User.';
 EXIT(ERROR.GEN_FAILURE);

/* ------------------------------------------------------------------------- */
ShowHelp: PROCEDURE EXPOSE (GlobalVars)

 SAY Title;
 SAY;

 PARSE SOURCE . . ThisFile

 DO i = 1 TO 3
    rc = LINEIN(ThisFile);
 END;

 ThisLine = LINEIN(Thisfile);
 DO WHILE (ThisLine \= ' */')
    SAY SUBSTR(ThisLine, 7);
    ThisLine = LINEIN(Thisfile);
 END;

 /* Datei wieder schlieáen */
 rc = LINEOUT(Thisfile);

 RETURN('');

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

/* ========================================================================= */
GenerateObj: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG BinFile, SymbolName, ObjFile;

 rc = ERROR.NO_ERROR;
 SegmentAttr = X2D(69);
 DataOffset = 0;

 /* determine module name */
 ModuleName = FILESPEC( 'N', BinFile);

 /* query filesize of file */
 BinSize = STREAM( BinFile, 'C', 'QUERY SIZE');

 /* determine seg size */
 SegSize = BinSize;
 IF (Seg.WriteSize) THEN
    SegSize = SegSize + 4;

 /* write Translator Header */
 StringLen = LENGTH( ModuleName);
 HeaderLen = StringLen + 2;
 rc = CHAROUT( ObjFile, D2C(X2D(80)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(StringLen));
 rc = CHAROUT( ObjFile, ModuleName);

 rc = CHAROUT( ObjFile, D2C(0));

 /* write Name list */
 StringLen = LENGTH( Seg.SegmentName''Seg.ClassName''Seg.TypeName);
 HeaderLen = StringLen + 4;
 rc = CHAROUT( ObjFile, D2C(X2D(96)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(LENGTH(Seg.SegmentName)));
 rc = CHAROUT( ObjFile, Seg.SegmentName);

 rc = CHAROUT( ObjFile, D2C(LENGTH(Seg.ClassName)));
 rc = CHAROUT( ObjFile, Seg.ClassName);

 rc = CHAROUT( ObjFile, D2C(LENGTH(Seg.TypeName)));
 rc = CHAROUT( ObjFile, Seg.TypeName);

 rc = CHAROUT( ObjFile, D2C(0));

 /* write segment definition */
 HeaderLen = 9;
 rc = CHAROUT( ObjFile, D2C(X2D(99)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(SegmentAttr));

 rc = CHAROUT( ObjFile, REVERSE(D2C(SegSize, 2))); /* size of all data */

 rc = CHAROUT( ObjFile, REVERSE(D2C(0, 2)));    /* DW: Segm Name Index    */
 rc = CHAROUT( ObjFile, D2C(1));                /* DB: Class Name Index   */
 rc = CHAROUT( ObjFile, REVERSE(D2C(2, 2)));    /* DW: Overlay name index */

 rc = CHAROUT( ObjFile, D2C(0));

 /* write group name definition */
 HeaderLen = 2;
 rc = CHAROUT( ObjFile, D2C(X2D(9A)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(3));       /* DB: group name index */

 rc = CHAROUT( ObjFile, D2C(0));

 /* write Public Names Definition */
 HeaderLen = LENGTH(SymbolName) + 9;
 rc = CHAROUT( ObjFile, D2C(X2D(91)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(0));       /* DB: base group index */
 rc = CHAROUT( ObjFile, D2C(1));       /* DB: base segment index */

 rc = CHAROUT( ObjFile, D2C(LENGTH(SymbolName)));
 rc = CHAROUT( ObjFile, SymbolName);

 rc = CHAROUT( ObjFile, REVERSE(D2C(0, 4))); /* data offset */
 rc = CHAROUT( ObjFile, D2C(0));             /* type index */

 rc = CHAROUT( ObjFile, D2C(0));

 /* write comment */
 HeaderLen = 4;
 rc = CHAROUT( ObjFile, D2C(X2D(88)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(X2D(40)));
 rc = CHAROUT( ObjFile, D2C(X2D(A2)));
 rc = CHAROUT( ObjFile, D2C(1));

 rc = CHAROUT( ObjFile, D2C(0));

 IF (Seg.WriteSize) THEN
 DO
    /* write data item: size file */
    DataSize   = 4;
    HeaderLen  = DataSize + 6;
    rc = CHAROUT( ObjFile, D2C(X2D(A1)));
    rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));
    rc = CHAROUT( ObjFile, D2C(1));                      /* segment index */
    rc = CHAROUT( ObjFile, REVERSE(D2C(DataOffset, 4))); /* data offset */
    rc = CHAROUT( ObjFile, REVERSE(D2C(BinSize, 4)));    /* this is the data */
    rc = CHAROUT( ObjFile, D2C(0));
    DataOffset = DataOffset + DataSize;
 END;

 /* make data records for each 1024 bytes ! */
 DataToWrite = BinSize;
 DO WHILE (DataToWrite  \= 0)
    /* write data item */
    DataSize   = MIN( 1024, DataToWrite);
    Data = CHARIN(BinFile,, DataSize);
    HeaderLen  = DataSize + 6;
    rc = CHAROUT( ObjFile, D2C(X2D(A1)));
    rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));
    rc = CHAROUT( ObjFile, D2C(1));                      /* segment index */
    rc = CHAROUT( ObjFile, REVERSE(D2C(DataOffset, 4))); /* data offset */
    rc = CHAROUT( ObjFile, data);
    rc = CHAROUT( ObjFile, D2C(0));
    DataOffset  = DataOffset + DataSize;
    DataToWrite = DataToWrite - DataSize;
 END;

 /* write end-of-module */
 HeaderLen  = 2
 rc = CHAROUT( ObjFile, D2C(X2D(8B)));
 rc = CHAROUT( ObjFile, REVERSE(D2C(HeaderLen, 2)));
 rc = CHAROUT( ObjFile, D2C(1));
 rc = CHAROUT( ObjFile, D2C(0));

 /* close file */
 rc = STREAM( BinFile, 'C', 'CLOSE');
 rc = LINEOUT( ObjFile);

 RETURN rc;
