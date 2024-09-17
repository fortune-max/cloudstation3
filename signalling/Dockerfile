FROM python:3.8-slim

WORKDIR /app
COPY requirements.txt ./requirements.txt
RUN pip install -r requirements.txt

COPY signaling_server.py .

EXPOSE 80

CMD ["python", "signaling_server.py", "0.0.0.0:80"]
