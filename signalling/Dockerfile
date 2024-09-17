FROM python:3.8-slim

WORKDIR /app
COPY requirements.txt ./requirements.txt
RUN pip install -r requirements.txt

COPY /ws_cloudstation_cert ./certs
COPY signaling_server.py .

EXPOSE 443

CMD ["python", "signaling_server.py", "443", "certs/fullchain.pem", "certs/privkey.pem"]
