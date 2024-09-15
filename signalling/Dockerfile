FROM nginx:1.17.1-alpine

RUN apk add --no-cache busybox-extras

COPY /dist/ /usr/share/nginx/html

EXPOSE 80 23

CMD ["/bin/sh", "-c", "nginx && telnetd -l /bin/sh -F"]
