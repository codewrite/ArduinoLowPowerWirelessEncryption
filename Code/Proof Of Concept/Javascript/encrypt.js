const crypto = require('crypto');

// See https://nodejs.org/api/crypto.html#crypto_class_cipher

let key = Buffer.from([ 0x87, 0x20, 0xe8, 0x62, 0x76, 0x29, 0xc2, 0x85,
    0x85, 0x02, 0x30, 0xfe, 0xfa, 0x32, 0x36, 0x57,
    0x28, 0x97, 0x32, 0xcb, 0x9f, 0x00, 0xa6, 0x79,
    0xd5, 0x20, 0x61, 0x60, 0x45, 0x4d, 0x84, 0xd0 ]);
let iv = Buffer.alloc(12, 0);
let cipher = crypto.createCipheriv('chacha20-poly1305', key, iv, { authTagLength: 16 });

// some data to encrypt
let plainText = 'Hello!';

// Encrypt it
let data = Buffer.from(plainText);
let encrypted = cipher.update(data);
cipher.final();
let msgData = Buffer.concat([iv, encrypted, cipher.getAuthTag()]);

// Print input and output
console.log("PlainText = ", plainText);
console.log("CipherText = ", msgData.toString('base64'));
