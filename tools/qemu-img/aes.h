#ifndef QEMU_AES_H
#define QEMU_AES_H

/*!
 * @def AES_MAXNR
 * @brief Maximum number of AES rounds (AES-256).
 */
#define AES_MAXNR 14

/*!
 * @def AES_BLOCK_SIZE
 * @brief AES block size in bytes.
 */
#define AES_BLOCK_SIZE 16

/*!
 * @brief AES key schedule.
 */
struct aes_key_st {
    uint32_t rd_key[4 *(AES_MAXNR + 1)];
    int rounds;
};
typedef struct aes_key_st AES_KEY;

/*!
 * @brief Expand the cipher key into the encryption key schedule.
 * @param[in] userKey User-supplied key bytes.
 * @param[in] bits Key size in bits (128, 192 or 256).
 * @param[out] key Destination key schedule.
 * @retval -1 NULL pointer argument.
 * @retval -2 Unsupported key size.
 * @retval 0 Success.
 * @return 0 on success, negative on error.
 */
int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);

/*!
 * @brief Expand the cipher key into the decryption key schedule.
 * @param[in] userKey User-supplied key bytes.
 * @param[in] bits Key size in bits (128, 192 or 256).
 * @param[out] key Destination key schedule.
 * @retval -1 NULL pointer argument.
 * @retval -2 Unsupported key size.
 * @retval 0 Success.
 * @return 0 on success, negative on error.
 */
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);

/*!
 * @brief Encrypt a single block.
 * @param[in] in Input block (AES_BLOCK_SIZE bytes).
 * @param[out] out Output block (may overlap @p in).
 * @param[in] key Encryption key schedule.
 */
void AES_encrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);

/*!
 * @brief Decrypt a single block.
 * @param[in] in Input block (AES_BLOCK_SIZE bytes).
 * @param[out] out Output block (may overlap @p in).
 * @param[in] key Decryption key schedule.
 */
void AES_decrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);

/*!
 * @brief AES-CBC encrypt/decrypt of a buffer.
 * @param[in] in Input buffer.
 * @param[out] out Output buffer (may overlap @p in).
 * @param[in] length Buffer length in bytes.
 * @param[in] key Key schedule.
 * @param[in,out] ivec Initialization vector, updated in place.
 * @param[in] enc Non-zero to encrypt, zero to decrypt.
 */
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key,
		     unsigned char *ivec, const int enc);

#endif
