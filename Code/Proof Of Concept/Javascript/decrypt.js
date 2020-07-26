const crypto = require('crypto');

// See https://nodejs.org/api/crypto.html#crypto_class_decipher

let key = Buffer.from([ 0x87, 0x20, 0xe8, 0x62, 0x76, 0x29, 0xc2, 0x85,
    0x85, 0x02, 0x30, 0xfe, 0xfa, 0x32, 0x36, 0x57,
    0x28, 0x97, 0x32, 0xcb, 0x9f, 0x00, 0xa6, 0x79,
    0xd5, 0x20, 0x61, 0x60, 0x45, 0x4d, 0x84, 0xd0 ]);

// some data to decrypt
let cipherText = 'AAAAAAAAAAAAAAAAJuu6wjenajKkCdut70j6fMJPyVcpwQ==';
let decrypted = '';

// Decrypt it
try
{
    let cipherWithIV = new Buffer.from(cipherText, 'base64');
    let iv = cipherWithIV.slice(0, 12);
    let encrypted = cipherWithIV.slice(12, cipherWithIV.length - 16);
    let poly1305 = cipherWithIV.slice(cipherWithIV.length - 16, cipherWithIV.length);
    let decipher = crypto.createDecipheriv('chacha20-poly1305', key, iv, { authTagLength: 16 });
    decrypted = decipher.update(encrypted);
    decipher.setAuthTag(poly1305);
    decrypted += decipher.final();
}
catch(err)
{
    console.log(err.message);
}

// Print input and output
console.log("CipherText = ", cipherText);
console.log("PlainText = ", decrypted);
