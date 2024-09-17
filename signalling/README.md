# certbot-assist

To generate the certificate (or renew)
- Install certbot locally (apt-get install certbot or from GitHub releases page) v2.9.0 confirmed to work
- certbot certonly --manual --preferred-challenges dns -d cloudstation3.com -d www.cloudstation3.com
- Change the TXT records as specified by command, then press enter (may require some delay (~1min) to allow propagation)

To automate this for renewals (as namecheap doesn't have an API for changing TXT records) see acme dns https://github.com/joohoi/acme-dns?tab=readme-ov-file and https://asciinema.org/a/94903
