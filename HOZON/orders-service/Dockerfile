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
COPY orders-service/ /app/orders-service/

# Build from service directory
WORKDIR /app/orders-service
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

COPY --from=builder /app/orders-service/build/orders-service /app/

ENV HTTP_PORT=8081
ENV DB_HOST=orders-db
ENV DB_PORT=5432
ENV DB_NAME=orders
ENV DB_USER=orders
ENV DB_PASSWORD=orders
ENV RABBITMQ_HOST=rabbitmq
ENV RABBITMQ_PORT=5672
ENV RABBITMQ_USER=guest
ENV RABBITMQ_PASSWORD=guest

EXPOSE 8081

CMD ["./orders-service"]
