package broker

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"time"

	"github.com/go-x-pkg/log"
	"github.com/hashicorp/consul/api"
)

func (a *App) queuePerform(ctx context.Context) error {
	config := api.DefaultConfig()
	config.Address = net.JoinHostPort(a.ctx.cfg().Consul.Host, strconv.Itoa(a.ctx.cfg().Consul.Port))

	consul, err := api.NewClient(config)
	if err != nil {
		return err
	}

	workers, err := consul.Agent().Services()
	if err != nil {
		return err
	}

	workerHost := ""
	workerPort := 0

	for _, worker := range workers {
		workerHost = worker.Address
		workerPort = worker.Port
		break
	}

	fmt.Printf("[>] curl -XGET %s:%d\n", workerHost, workerPort)

	return nil
}

func (a *App) queue(ctx context.Context) {
	t := time.NewTicker(1 * time.Second)
	defer t.Stop()

	for {
		select {
		case <-t.C:
			if err := a.queuePerform(ctx); err != nil {
				logFn(log.Error, "error queue perform: %s", err)
			}

		case <-ctx.Done():
			return
		}
	}
}
