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
byte[] iv = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
var cipher = new ChaCha20Poly1305(key);

string EncodeString(string plainText)
{
    var data = iv.ToList();
    data.AddRange(cipher.Encrypt(Encoding.ASCII.GetBytes(plainText), null, iv).ToList());
    var b64str = Convert.ToBase64String(data.ToArray());
    return b64str;
}

var plainText = "abcd";
//var plainText = "The white cat walked slowly and quitely past the noisy black dog";
var b64data = EncodeString(plainText);
Console.WriteLine(b64data);
