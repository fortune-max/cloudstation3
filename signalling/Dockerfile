FROM python:3.8-slim

WORKDIR /app
COPY requirements.txt ./requirements.txt
RUN pip install -r requirements.txt

COPY /ws_cloudstation_cert ./certs
COPY signaling_server.py .

EXPOSE 80

RUN apt-get update && apt-get install -y openssh-server
RUN ssh-keygen -A
RUN mkdir -p /run/sshd && chmod 0755 /run/sshd
RUN echo "root:docker" | chpasswd
RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config && \
    echo "ChallengeResponseAuthentication no" >> /etc/ssh/sshd_config

COPY /ngrok.yml /root/.config/ngrok/ngrok.yml
COPY /ngrok /usr/bin/ngrok
RUN chmod +x /usr/bin/ngrok

# CMD ["/bin/sh", "-c", "/usr/sbin/sshd && ngrok tcp 22 > /dev/null 2>&1 & python signaling_server.py 443 certs/fullchain.pem certs/privkey.pem"]
CMD ["/bin/sh", "-c", "/usr/sbin/sshd && ngrok tcp 22 > /dev/null 2>&1 & python signaling_server.py 80"]
