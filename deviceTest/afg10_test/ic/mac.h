 
#ifdef __cplusplus
extern "C" {
#endif


#ifndef _DES_H
#define _DES_H

typedef struct
{
	unsigned long int esk[32];     /* DES encryption subkeys */
	unsigned long int dsk[32];     /* DES decryption subkeys */
}
des_context;

typedef struct
{
	unsigned long int esk[96];     /* Triple-DES encryption subkeys */
	unsigned long int dsk[96];     /* Triple-DES decryption subkeys */
}
des3_context;

int  des_set_key(des_context *ctx, unsigned char key[8]);
void des_encrypt(des_context *ctx, unsigned char input[8], unsigned char output[8]);
void des_decrypt(des_context *ctx, unsigned char input[8], unsigned char output[8]);

int  des3_set_2keys(des3_context *ctx, unsigned char key1[8], unsigned char key2[8]);
int  des3_set_3keys(des3_context *ctx, unsigned char key1[8], unsigned char key2[8], unsigned char key3[8]);

void des3_encrypt( des3_context *ctx, unsigned char input[8], unsigned char output[8]);
void des3_decrypt( des3_context *ctx, unsigned char input[8], unsigned char output[8]);
void pbocpadding(unsigned char* data, int len,unsigned char* outputdata,int* outputlen);
int TDesEncryptMac(unsigned char *key, unsigned char* input, int inputlen, unsigned char* output, int* outputlen, unsigned char ivdata[8]);//pboc_3des_mac	
#endif /* mac.h */

#ifdef __cplusplus
}
#endif
