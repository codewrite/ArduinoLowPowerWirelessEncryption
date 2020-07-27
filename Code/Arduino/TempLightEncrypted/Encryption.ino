#include <ChaChaPoly.h>

// Crypto lib docs: https://github.com/rweather/arduinolibs

/*
  The encrypted message is in this format:
  
  +----+----------------+---------------------+
  | IV | Encrypted Data | Authentication Code |
  +----+----------------+---------------------+
  
  Where:
  IV = Initialization vector
  Encrypted data is encrypted using ChaCha20 (IETF variant)
  The authentication code is calculated from the encrypted data using Poly1305
  
  The IV is 12 bytes and is incremented for every message sent.
*/

// The following key should be randomly generated rather than created from a passphrase
byte cipherKey[] = { 0x87, 0x20, 0xe8, 0x62, 0x76, 0x29, 0xc2, 0x85,
                  0x85, 0x02, 0x30, 0xfe, 0xfa, 0x32, 0x36, 0x57,
                  0x28, 0x97, 0x32, 0xcb, 0x9f, 0x00, 0xa6, 0x79,
                  0xd5, 0x20, 0x61, 0x60, 0x45, 0x4d, 0x84, 0xd0 };
// It is the sender's responsibility to ensure the IV only gets used ONCE
byte cipherIV[] = { SEND_ADDR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

#define IV_LEN 12
#define KEY_LEN 32
#define POLY1305_LEN 16
#define SHORT_IV_LEN 3
#define SHORT_POLY_LEN 4

ChaChaPoly cipher;

byte receivedIV[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int EncryptPlainText(byte* cipherData, byte* unencrypted, int len)
{
  byte polyData[POLY1305_LEN];
  cipher.clear();
  cipher.setKey(cipherKey, KEY_LEN);
  cipher.setIV(cipherIV, IV_LEN);
  cipherData[0] = cipherIV[0];
  memcpy(&cipherData[1], &cipherIV[IV_LEN - SHORT_IV_LEN], SHORT_IV_LEN);
  cipher.encrypt(&cipherData[1 + SHORT_IV_LEN], unencrypted, len);
  cipher.computeTag(polyData, POLY1305_LEN);
  memcpy(&cipherData[1 + SHORT_IV_LEN + len], &polyData[POLY1305_LEN - SHORT_POLY_LEN], SHORT_POLY_LEN);
  return 1 + SHORT_IV_LEN + len + SHORT_POLY_LEN;
}

void IncrementIV()
{
  for (int i = IV_LEN - 1; i >= 0; i--)
  {
    cipherIV[i]++;
    if (cipherIV[i] != 0)
      break;
  }
}

//-----------------------------------------------------------------------------------------------------------------------
// This is not used. It is left here in case it is needed in the future.
// Normally we wouldn't leave unused code in, but this gives a good idea of how to decrypt messages
// Note this is based on full ChaCha20-Poly1305, not the cut down (4 byte IV, 4 byte signature) that we are using to encrypt
int DecryptCipherText(byte* unencrypted, byte* cipherData, int len)
{
  cipher.clear();
  cipher.setKey(cipherKey, KEY_LEN);
  cipher.setIV(cipherData, IV_LEN);
  cipher.decrypt(unencrypted, cipherData + IV_LEN, len - IV_LEN - POLY1305_LEN);
  bool valid = cipher.checkTag(cipherData + len - POLY1305_LEN, POLY1305_LEN);
  if (valid)
  {
    valid = IsIncomingIVValid(cipherData);
  }
  if (valid)
  {
    memcpy(receivedIV, cipherData, IV_LEN);
    return len - IV_LEN - POLY1305_LEN;
  }
  else
  {
    return -1;
  }
}

// Not used. See DecryptCipherText() above
bool IsIncomingIVValid(byte* iv)
{
  // Check incoming IV is greater than the previous valid IV to prevent replay attacks
  bool valid = true;
  // TODO: Check SEND_ADDR is same as message address
  
  // TODO: work out what we are going to check
  for (int i = 0; i < IV_LEN && valid; i++)
  {
    if (i == IV_LEN - 1 && receivedIV[i] >= iv[i])
    {
      valid = false;
    }
    else if (receivedIV[i] > iv[i])
    {
      valid = false;
    }
  }
  return valid;
}
