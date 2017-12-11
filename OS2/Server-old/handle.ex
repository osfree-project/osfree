_Digi>	ULONG LIBENTRY HTTPClntReqFldDestroy(HREQFLD hReqFld)
	<_Digi>	{
	<_Digi>	PREQFIELDS psReqFields = hdlGet( HANDLE_TYPE_REQFLD, hReqFld );
	<_Digi>	if ( psReqFields == NULL )
	<_Digi>	return HTTPCLNT_INVALID_HANDLE;
	<_Digi>	...работаю со структурой psReqFields...
	<_Digi>	}
	<_Digi>	ULONG LIBENTRY HTTPClntClose(HURL hURL)
	<_Digi>	{
	<_Digi>	PURL psURL = hdlGet( HANDLE_TYPE_URL, hURL );
	<_Digi>	if ( psURL == NULL )
	<_Digi>	return HTTPCLNT_INVALID_HANDLE;
	<_Digi>	...работаю со структурой psURL...
	<_Digi>	}