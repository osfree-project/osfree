/*!
 * @file porting.h
 * @brief Portability declarations for non-POSIX hosts.
 */

#ifndef LINUX
/*!
 * @def EOPNOTSUPP
 * @brief Operation not supported (fallback definition when the host lacks it).
 */
#undef EOPNOTSUPP
#define	EOPNOTSUPP	45		/* Operation not supported */
#endif

/*!
 * @def ENOTSUP
 * @brief Operation not supported.
 */
#define	ENOTSUP		EOPNOTSUPP	/* Operation not supported */


/*!
 * @brief Truncate or extend a file to a specified length.
 * @param[in] handle File descriptor.
 * @param[in] size New file size in bytes.
 * @retval -1 Failure.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
int  ftruncate( int handle, long size );

/*!
 * @brief Resolve a pathname to an absolute path.
 * @param[in] path Input pathname.
 * @param[out] resolved_path Destination buffer for the resolved path.
 * @return Pointer to @p resolved_path.
 */
char *realpath(const char *path, char *resolved_path);
