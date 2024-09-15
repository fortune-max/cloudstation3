FROM nginx:1.17.1-alpine

RUN apk add --no-cache openssh

RUN ssh-keygen -A

COPY ort.pub /root/.ssh/authorized_keys

RUN chmod 700 /root/.ssh && chmod 600 /root/.ssh/authorized_keys

RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication no" >> /etc/ssh/sshd_config && \
    echo "ChallengeResponseAuthentication no" >> /etc/ssh/sshd_config && \
    echo "PubkeyAuthentication yes" >> /etc/ssh/sshd_config

COPY /dist/ /usr/share/nginx/html

EXPOSE 80 22

CMD ["/bin/sh", "-c", "nginx && /usr/sbin/sshd -D"]
