FROM alpine:latest

RUN addgroup -S appgroup && adduser -S appuser -G appgroup

USER appuser

COPY ./build /app

WORKDIR /app

CMD ["./web_server", "8000", "trec_05.ef_type1.bin"]