using System;
using System.Text;
using System.Linq;
using NaCl.Core;

// Crypto docs: https://github.com/idaviddesmet/NaCl.Core

// Note: This is very much "Work In Progress". It could do with a LOT of refactoring.
// Some thoughts are:
// 1. Coping with multiple devices could be improved. Currently we only expect (and work ok) with one device.
// 2. Received IVs should be persisted. Since we could have 255 devices the persisted list should be 255 IVs
// 3. My thinking is that the Raspberry Pi should have an address of 0. 1 - 255 would be for remote Arduinos.
// 4. We currently only receive rather than transmit. Since the Arduinos are mostly asleep, getting them to
//    receive would probably involve them periodically sending something (as they do anyway) and then waiting
//    for a few milliseconds to see if the Raspberry Pi sends something to them.
// 5. We don't check for number of MAC failures. We should, and we should assume someone is trying to hack us
//    if the number of failures exceeds a fixed number, e.g. 16,000 (maximum).
// 6. Dotnet / C# is my language of choice - but Node Red and javascript may be more useful. Also Python might
//    be easier / simpler.

namespace SerialRW
{
  public class Encryption
  {

    byte[] key = { 0x87, 0x20, 0xe8, 0x62, 0x76, 0x29, 0xc2, 0x85,
                   0x85, 0x02, 0x30, 0xfe, 0xfa, 0x32, 0x36, 0x57,
                   0x28, 0x97, 0x32, 0xcb, 0x9f, 0x00, 0xa6, 0x79,
                   0xd5, 0x20, 0x61, 0x60, 0x45, 0x4d, 0x84, 0xd0 }; 
    byte[] iv = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    byte[] device1_lastReceivedIV = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };     // This should be persisted in case the program restarts
    ChaCha20 cipher;
    ChaCha20Poly1305 cipherPoly1305;

    public Encryption()
    {
        cipher = new ChaCha20(key, 1);
        cipherPoly1305 = new ChaCha20Poly1305(key);
    }

    public (string plainText, byte[] shortPoly1305) DecodeString(string b64str)
    {
        var data = new byte[0];
        try
        {
            data = Convert.FromBase64String(b64str);
        }
        catch (FormatException)
        {
            return ("Invalid Base64 String", new byte[] { 0, 0, 0, 0 });
        }
        var encrypted = new byte[0];
        if (data.Length >= 4)
        {
            iv[0] = data[0];
            iv = (iv.Take(9)).Concat(data.Skip(1).Take(3)).ToArray();
            encrypted = data.Skip(4).Take(data.Length - 8).ToArray();
        }
        var shortPoly1305 = new byte[] { 0, 0, 0, 0 };
        if (data.Length >= 8)
        {
            shortPoly1305 = data.Skip(data.Length - 4).ToArray();
        }
        return (Encoding.ASCII.GetString(cipher.Decrypt(encrypted, iv)), shortPoly1305);
    }

    public bool Poly1305Match(string msg, byte[] shortPoly1305)
    {
        var msgPoly1305 = cipherPoly1305.Encrypt(Encoding.ASCII.GetBytes(msg), null, iv);
        return msgPoly1305.Skip(msgPoly1305.Length - 4).SequenceEqual(shortPoly1305);
    }

    public bool IVGreaterThanLast()
    {
        bool valid = false;
        for (int i = 0; i < iv.Length; i++)
        {
            if (iv[i] > device1_lastReceivedIV[i])
            {
                valid = true;
                break;
            }
            else if (iv[i] < device1_lastReceivedIV[i])
            {
                valid = false;
                break;
            }
        }
        if (valid)
        {
            device1_lastReceivedIV = iv;
            // TODO: should persist the last received IV here
        }
        return valid;
    }
  }
}