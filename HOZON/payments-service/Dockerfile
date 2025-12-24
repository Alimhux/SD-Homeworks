# Build stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpqxx-dev \
    libpq-dev \
    librabbitmq-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy common library and this service
COPY common/ /app/common/
COPY payments-service/ /app/payments-service/

# Build from service directory
WORKDIR /app/payments-service
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j2

# Runtime stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libpqxx-6.4 \
    libpq5 \
    librabbitmq4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/payments-service/build/payments-service /app/

ENV HTTP_PORT=8082
ENV DB_HOST=payments-db
ENV DB_PORT=5432
ENV DB_NAME=payments
ENV DB_USER=payments
ENV DB_PASSWORD=payments
ENV RABBITMQ_HOST=rabbitmq
ENV RABBITMQ_PORT=5672
ENV RABBITMQ_USER=guest
ENV RABBITMQ_PASSWORD=guest

EXPOSE 8082

CMD ["./payments-service"]
