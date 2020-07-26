#r "nuget:NaCl.Core/1.*"

using System;
using System.Text;
using System.Linq;
using NaCl.Core;

// Crypto docs: https://github.com/idaviddesmet/NaCl.Core

byte[] key = { 0x87, 0x20, 0xe8, 0x62, 0x76, 0x29, 0xc2, 0x85,
               0x85, 0x02, 0x30, 0xfe, 0xfa, 0x32, 0x36, 0x57,
               0x28, 0x97, 0x32, 0xcb, 0x9f, 0x00, 0xa6, 0x79,
               0xd5, 0x20, 0x61, 0x60, 0x45, 0x4d, 0x84, 0xd0 }; 
var cipher = new ChaCha20Poly1305(key);

string DecodeString(string b64data)
{
    var data = Convert.FromBase64String(serialData);
    if (data.Length < 12 + 16) { throw new InvalidDataException(); }
    var iv = data.Take(12).ToArray();
    var encrypted = data.Skip(12).ToArray();
    string message = "";
    try
    {
      message = Encoding.ASCII.GetString(cipher.Decrypt(encrypted, null, iv));
    }
    catch (Exception) //(CryptographyException ex)
    {
      message = $"Decode failed for {b64data}";
    }
    return message;
}

var serialData = "AAAAAAAAAAAAAAAAD+y1yp45KIOnDqcsb+ZLxdVQoU8=";
var plainText = DecodeString(serialData);
Console.WriteLine(plainText);
