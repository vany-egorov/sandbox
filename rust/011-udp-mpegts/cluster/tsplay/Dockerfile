FROM debian:10

RUN apt-get update && apt-get install -y \
  tstools

# CMD ["tsplay", "/mnt/dumps/HD-1.ts", "239.255.1.1:5500", "-loop"]
CMD ["tsplay", "/mnt/dumps/HD-NatGeoWild.ts", "239.255.1.1:5500", "-loop"]
