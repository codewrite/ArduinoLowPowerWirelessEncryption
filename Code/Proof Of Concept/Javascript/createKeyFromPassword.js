const crypto = require('crypto');

// Use your own password or this random one
let password = crypto.randomBytes(256);
let salt = crypto.randomBytes(256);

const key = crypto.scryptSync(password, salt, 32);
let keyStr = '';
for (i = 0; i < key.length; i++)
{
    keyStr += '0x' + key[i].toString(16).padStart(2, '0') + (i == key.length - 1 ? '' : ', ');
    if (i % 8 == 7)
        keyStr += "\n";
}

console.log(keyStr);
