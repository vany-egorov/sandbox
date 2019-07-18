FROM ha-eta-build as build

FROM debian:10

COPY --from=build /usr/local/src/ha-eta/ha-eta /usr/local/bin/ha-eta

ENTRYPOINT ["ha-eta"]
