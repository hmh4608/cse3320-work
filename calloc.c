void *calloc(size_t num, size_t size)
{
	size_t total_size = num * size;
	struct _block *var_name = ( struct _block * ) malloc ( total_size ); //?? & need to rename the var
	memset( var_name, num, 0 ); //maybe the size is the second argument
	return BLOCK_DATA(var_name);
}
