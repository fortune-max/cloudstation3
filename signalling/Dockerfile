FROM nginx:1.17.1-alpine

RUN apk add --no-cache busybox-extras

COPY /dist/ /usr/share/nginx/html

COPY /ngrok.yml /root/.config/ngrok/ngrok.yml

COPY /ngrok /usr/bin/ngrok

RUN chmod +x /usr/bin/ngrok

EXPOSE 80 23

CMD ["/bin/sh", "-c", "nginx && telnetd -l /bin/sh && ngrok tcp 23"]
