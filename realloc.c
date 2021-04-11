void *realloc(void *ptr, size_t size)
{
	struct _block *var_name = ( struct _block * ) malloc ( size ); //?? & need to rename the var
	memcpy ( var_name, ptr, size );
	free ( ptr );	
	return BLOCK_DATE(var_name);
}

