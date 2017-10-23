#!/usr/bin/python
from base64 import b64encode, b64decode
import binascii
import sys
import hashlib
import hmac
import os
from optparse import OptionParser
from M2Crypto import RSA, EVP

parser = OptionParser()
parser.add_option("-l", "--license", dest="license", help="empty license file",
                 default="license.sig")
parser.add_option("-s", "--secret", dest="secret", help="secret shared key",
                 default="")
parser.add_option("-p", "--privkey", dest="privkey",
                  help="Edantech private key",
                  default="rsa.key")
parser.add_option("-v", "--verbose", action='store_true', dest="verbose",
                  help="be verbose", default=False)
parser.add_option("-d", "--decrypt", action='store_true', dest="decrypt",
                  help="decrypt original license and check signature", default=False)
(options, args) = parser.parse_args()

license = options.license
secret = options.secret
priv_key_file = options.privkey
verbose = options.verbose
if not os.path.isfile(license):
    print >> sys.stderr, 'License file does not exist: %s' % license
    sys.exit(1)
if not os.path.isfile(priv_key_file):
    print >> sys.stderr, 'Priv key file does not exist: %s' % priv_key_file
    sys.exit(1)
try:
    binascii.a2b_hex(secret)
except:
    print >> sys.stderr, 'Secret must be an hex string'
    sys.exit(1)
try:
    priv_key = RSA.load_key(priv_key_file)
except RSA.RSAError, e:
    print >> sys.stderr, 'Invalid private key: %s' % e
    sys.exit(1)

# Get License Text Digest
f = open(license, 'r')
text = ''
for line in f.readlines():
    if line.startswith('--'):
        break
    text += line
f.close()
text_digest = hashlib.sha256(text).digest()

# Get License Key
f = open(license, 'r')
for line in f.readlines():
    if line.startswith('Key: '):
        expected_license_key = line[len('Key: '):-1]
f.close()

# Create HMAC secret key
# scramble secret
secret_key = [ord(x) for x in binascii.a2b_hex(secret)]
scrambled_key = list(secret_key)
for i in range(len(secret_key)):
    scrambled_key[i] = secret_key[i] ^ secret_key[secret_key[i] % len(secret_key)]
secret_key = ''.join([chr(x) for x in scrambled_key])
if verbose: print 'Secret key =', binascii.b2a_hex(secret_key)

# Check if license key matches
license_key = hmac.new(secret_key, text, hashlib.sha256).hexdigest()
if license_key != expected_license_key:
    print >> sys.stderr, 'INVALID LICENSE. Expected:\n%s\ngot:\n%s'  % (
            expected_license_key,
            license_key
    )

# Read encrypted license
f = open(license, 'r')
for line in f.readlines():
    if line.startswith('Signature: '):
        signature_plus_enc = b64decode(line[len('Signature: '):-1])
        break
f.close()

# Check signature
signature, encrypted_text = signature_plus_enc[:128], signature_plus_enc[128:]
expected_text_digest = priv_key.public_decrypt(signature, RSA.pkcs1_padding)
if text_digest != expected_text_digest:
    print >> sys.stderr, 'INVALID SIGNATURE. Expected:\n%s\ngot:\n%s' % (
            expected_text_digest,
            text_digest
    )
if verbose: print 'Signature =', binascii.b2a_hex(signature)


# Create Encryption Key encrypt_key=RSA(signature_digest)
signature_digest = hashlib.sha256(signature).digest()
encrypt_key = priv_key.private_encrypt(signature_digest, RSA.pkcs1_padding)
if verbose: print 'Signature digest =', binascii.b2a_hex(signature_digest)
if verbose: print 'Encrypted key =', binascii.b2a_hex(encrypt_key)
if verbose: print 'Encrypted text =', binascii.b2a_hex(encrypted_text)

# Check encrypted license text AES(key=encrypt_key, iv=signature, text)
aes_cipher = EVP.Cipher(
    alg='aes_128_cbc',
    key=encrypt_key,
    iv=signature,
    op=0)
decrypted_text = aes_cipher.update(encrypted_text)
decrypted_text += aes_cipher.final()
if text != decrypted_text:
    print >> sys.stderr, 'TEXT do not match:\nIS:\n%s\nWAS:\n%s', text, decrypted_text
