FROM nginx:1.17.1-alpine

RUN apk add --no-cache busybox-extras openssh

RUN ssh-keygen -A

RUN echo "root:docker" | chpasswd

RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config && \
    echo "ChallengeResponseAuthentication no" >> /etc/ssh/sshd_config

COPY /dist/ /usr/share/nginx/html

COPY /ngrok.yml /root/.config/ngrok/ngrok.yml

COPY /ngrok /usr/bin/ngrok

RUN chmod +x /usr/bin/ngrok

EXPOSE 80 22 23

CMD ["/bin/sh", "-c", "nginx && /usr/sbin/sshd && telnetd -l /bin/sh && ngrok tcp 22"]
