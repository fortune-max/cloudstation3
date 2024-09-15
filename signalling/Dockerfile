FROM nginx:1.17.1-alpine

RUN apk add --no-cache openssh

RUN ssh-keygen -A

RUN echo "root:docker" | chpasswd

RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config

COPY /dist/ /usr/share/nginx/html

EXPOSE 80 22

CMD ["/bin/sh", "-c", "nginx && /usr/sbin/sshd -D"]
