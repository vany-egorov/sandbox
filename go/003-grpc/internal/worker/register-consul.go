package worker

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"time"

	"github.com/go-x-pkg/log"
	"github.com/hashicorp/consul/api"
)

func registerServiceWithConsul(consulHost string, consulPort int, host string, port int) error {
	config := api.DefaultConfig()
	config.Address = net.JoinHostPort(consulHost, strconv.Itoa(consulPort))

	consul, err := api.NewClient(config)
	if err != nil {
		return fmt.Errorf("error create new consul api client: %w", err)
	}

	registration := new(api.AgentServiceRegistration)

	registration.ID = host
	registration.Name = host
	registration.Address = host
	registration.Port = port
	registration.Check = new(api.AgentServiceCheck)
	registration.Check.HTTP = fmt.Sprintf("http://%s:%d/consul-healthcheck", host, port)
	registration.Check.Interval = "5s"
	registration.Check.Timeout = "3s"

	return consul.Agent().ServiceRegister(registration)
}

func retryRegisterServiceWithConsul(ctx context.Context, consulHost string, consulPort int, host string, port int) error {
	t := time.NewTicker(1 * time.Second)
	defer t.Stop()

	for {
		select {
		case <-t.C:
			if err := registerServiceWithConsul(consulHost, consulPort, host, port); err != nil {
				logFn(log.Error, "error register consul. will retry: %s", err)
			} else {
				return nil
			}

		case <-ctx.Done():
			return nil
		}
	}

	return nil
}
