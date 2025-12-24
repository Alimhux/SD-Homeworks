FROM nginx:alpine

COPY index.html /usr/share/nginx/html/index.html
COPY openapi.yaml /usr/share/nginx/html/openapi.yaml

RUN echo 'server { \
    listen 80; \
    server_name localhost; \
    root /usr/share/nginx/html; \
    index index.html; \
    location / { \
        try_files $uri $uri/ /index.html; \
    } \
    location ~* \.(yaml|yml)$ { \
        add_header Content-Type "text/yaml; charset=utf-8"; \
    } \
}' > /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
