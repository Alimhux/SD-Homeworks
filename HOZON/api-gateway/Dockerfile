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
COPY api-gateway/ /app/api-gateway/

# Build from service directory
WORKDIR /app/api-gateway
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

COPY --from=builder /app/api-gateway/build/api-gateway /app/

ENV HTTP_PORT=8080
ENV ORDERS_SERVICE_URL=http://orders-service:8081
ENV PAYMENTS_SERVICE_URL=http://payments-service:8082

EXPOSE 8080

CMD ["./api-gateway"]
