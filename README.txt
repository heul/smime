== System requirements ==
This module requires the PHP OpenSSL extension.

== Configuration ==
Visit /admin/settings/smime to upload certificates. Certs must be in p12
format. Some issuers deliver certs in p7s format, and they must be converted
before use.

I've tested with the free e-mail certificates from Comodo. I was able to
convert them to p12 with the OS X "Keychain Access" utility.

== Integration with other mailers ==
At present Drupal's core smtp_library is the only method that has been tested.
Integration is tricky, because any modifications to the message after the
signature has been generated will invalidate it. In most cases, integration
will require collaboration with the module that provides the smtp_library.

The S/MIME module currently can not generate the correct boundaries for
messages that are already formatted with MIME. This is a bug.

If you notice issues with contributed modules that implement
hook_mail_alter(), try increasing the weight of smime to force it to run
later.

== Security ==
Certificate files are stored as BLOBs in the database, and are not encrypted.
Passphrases may be supported in the future, though doing so only marginally
improves security since the passphrase itself must also be stored on the
server.