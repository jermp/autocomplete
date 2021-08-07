FROM ubuntu:latest

EXPOSE 8000

RUN groupadd appgroup && useradd appuser -G appgroup

COPY ./build /app

WORKDIR /app

RUN chmod +x web_server

# USER appuser

CMD ["./web_server", "8000", "trec_05.ef_type1.bin"]
