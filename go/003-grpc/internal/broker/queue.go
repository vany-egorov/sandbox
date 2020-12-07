package broker

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"time"

	"github.com/go-x-pkg/log"
	"github.com/hashicorp/consul/api"
	"google.golang.org/grpc"

	pb "github.com/vany-egorov/grpcexample/internal/pkg/service"
)

func (a *App) queuePerform(ctx context.Context) error {
	config := api.DefaultConfig()
	config.Address = net.JoinHostPort(a.ctx.cfg().Consul.Host, strconv.Itoa(a.ctx.cfg().Consul.Port))

	consul, err := api.NewClient(config)
	if err != nil {
		return err
	}

	agent := consul.Agent()

	workers, err := agent.Services()
	if err != nil {
		return err
	}

	workerHost := ""
	workerPort := 0

	for _, worker := range workers {
		health, _, err := agent.AgentHealthServiceByID(worker.ID)
		if err != nil {
			return fmt.Errorf("error check agent health by id: %w", err)
		}
		if health != api.HealthPassing {
			continue
		}

		workerHost = worker.Address
		workerPort = worker.Port + 1 // TODO: move to config ot metadata
		break
	}

	if workerHost == "" && workerPort == 0 {
		return fmt.Errorf("no alive workers!")
	}

	workerAddr := net.JoinHostPort(workerHost, strconv.Itoa(workerPort))

	conn, err := grpc.Dial(workerAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("error dial worker: %w", err)
	}
	defer conn.Close()

	worker := pb.NewWorkerClient(conn)

	reply, err := worker.Perform(ctx, &pb.JobRequest{Name: a.ctx.cfg().Name})
	if err != nil {
		return fmt.Errorf("error grpc perform: %w", err)
	}

	logFn(log.Info, "[>] send job to %s", reply.Name)

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
