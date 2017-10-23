#!/usr/bin/python
#
# License generator.
# This license generator is hackish, ad-hoc, and probably dumb. But since
# we don't have access to RSA primitives on the library, I couldn't
# figure out a better way to do it...
#
# It's objectives are:
# a) That someone who know a pre-shared secret key is able to check the
# validity of the license.
#
# b) That if the key is compromised, or the binary is modified to avoid
# the check, there's the possibility that some trace may have been
# left on the license to check from where it was originally copied if we
# are somehow able to perform an auditing of a server including the library.
#
# So if a library is found being used on a server, the following procedure
# should be performed to check if it has a valid license:
#
# a) Check if the license file exists, if it doesn't, it's a hacked library
# binary.
# b) Check if the license key is valid for the given license text and
# secret key for the version of the library. If it doesn't, then the license
# text was illegaly modified, or the license key is wrong.
# c) If the license key is invalid, try to get the original license text back
# by regenerating the encryption key and decrypting the encrypted license text.
#
# This procedure can be performed with the verify.py script as long as:
# a) You know the shared key.
# b) You have access to the private rsa.key.

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
                 default="license")
parser.add_option("-o", "--output", dest="output", help="output license",
                 default=None)
parser.add_option("-s", "--secret", dest="secret", help="secret shared key",
                 default="")
parser.add_option("-p", "--privkey", dest="privkey",
                  help="Edantech private key",
                  default="rsa.key")
parser.add_option("-v", "--verbose", action='store_true', dest="verbose",
                  help="be verbose", default=False)
(options, args) = parser.parse_args()

license = options.license
output = options.output or '%s.sig' % license
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

# Create HMAC secret key
# scramble secret
secret_key = [ord(x) for x in binascii.a2b_hex(secret)]
scrambled_key = list(secret_key)
for i in range(len(secret_key)):
    scrambled_key[i] = secret_key[i] ^ secret_key[secret_key[i] % len(secret_key)]
secret_key = ''.join([chr(x) for x in scrambled_key])
if verbose: print 'Secret key =', binascii.b2a_hex(secret_key)

# Create Lincese Key HMAC-SHA-256(secret_key, text)
license_key = hmac.new(secret_key, text, hashlib.sha256).hexdigest()

# Create Signature RSA(text_digest)
signature = priv_key.private_encrypt(text_digest, RSA.pkcs1_padding)
if verbose: print 'Signature =', binascii.b2a_hex(signature)

#
# Now we include an encrypted version of the license. The idea is that if
# someone copies the license to some other place, changes it, and somehow
# manages to correctly update the license key or just modified the binary
# to avoid the check, we still have an encrypted version of the license key
# that can only have been correctly encrypted by us, since we are the only
# ones that know the encryption key. This way we could track down the origin
# of the license file. Of course, if they also mangle the encrypted text,
# we don't have anything to do....
#
# To avoid having an additional encrpytion key, we generate a new key
# by encrypting the signature with our RSA private key. There's no way that
# someone can guess this generated key without knowing our RSA key.
#
# If they mangle the signature text, then we are forced to check every
# previously generated license...
#

# Create Encryption Key encrypt_key=RSA(signature_digest)
signature_digest = hashlib.sha256(signature).digest()
encrypt_key = priv_key.private_encrypt(signature_digest, RSA.pkcs1_padding)
if verbose: print 'Signature digest =', binascii.b2a_hex(signature_digest)
if verbose: print 'Encrypt key =', binascii.b2a_hex(encrypt_key)

# Create encrypted license text AES(key=encrypt_key, iv=signature, text)
aes_cipher = EVP.Cipher(
    alg='aes_128_cbc',
    key=encrypt_key,
    iv=signature,
    op=1)
encrypted_text = aes_cipher.update(text)
encrypted_text += aes_cipher.final()
if verbose: print 'Encrypted text =', binascii.b2a_hex(encrypted_text)

f = open(license, 'r')
buff = ''
for line in f.readlines():
    if line.startswith('Key:'):
        line = 'Key: %s\n' % license_key
    if line.startswith('Signature:'):
        line = 'Signature: %s\n' % b64encode(
            signature + encrypted_text
        )
    buff += line
f.close()

f = open(output, 'w')
f.write(buff)
f.close()
