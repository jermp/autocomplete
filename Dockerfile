FROM ubuntu:latest

EXPOSE 8000

RUN groupadd appgroup && useradd appuser -G appgroup

COPY . /src

WORKDIR /app

RUN apt update && apt install -y cmake g++ python

RUN cmake /src && cmake --build .

RUN chmod +x web_server && chmod +x build

RUN ./build ef_type1 /src/test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions -o trec_05.ef_type1.bin

RUN apt purge -y cmake g++ python

RUN rm -rf /src

USER appuser

CMD ["./web_server", "8000", "trec_05.ef_type1.bin"]
