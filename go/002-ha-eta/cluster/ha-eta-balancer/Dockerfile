FROM debian:10

RUN apt-get update
RUN apt-get install -y \
  nginx

ENTRYPOINT ["nginx"]
CMD ["-g", "daemon off;"]
