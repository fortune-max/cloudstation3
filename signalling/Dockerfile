FROM ubuntu:20.04

# Install necessary packages
RUN apt-get update && \
    apt-get install -y nginx openssh-server && \
    apt-get clean

# Generate SSH host keys
RUN ssh-keygen -A

# Create privilege separation directory for SSH
RUN mkdir -p /run/sshd && chmod 0755 /run/sshd

# Set root password
RUN echo "root:docker" | chpasswd

# Ensure SSH allows root login and password authentication
RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config && \
    echo "ChallengeResponseAuthentication no" >> /etc/ssh/sshd_config

# Copy the application files
COPY /dist/ /usr/share/nginx/html

# Copy ngrok configuration and binary
COPY /ngrok.yml /root/.config/ngrok/ngrok.yml
COPY /ngrok /usr/bin/ngrok

# Make ngrok executable
RUN chmod +x /usr/bin/ngrok

# Expose ports
EXPOSE 80 22

# Start nginx, sshd, telnetd, and ngrok
CMD ["/bin/sh", "-c", "service nginx start && /usr/sbin/sshd && ngrok tcp 22"]