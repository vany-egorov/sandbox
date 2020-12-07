package worker

import (
	"context"

	"github.com/go-x-pkg/log"
	pb "github.com/vany-egorov/grpcexample/internal/pkg/service"
)

type grpcWorkerServer struct {
	name string
	pb.UnimplementedWorkerServer
}

func (gws *grpcWorkerServer) Perform(ctx context.Context, request *pb.JobRequest) (*pb.JobReply, error) {
	logFn(log.Info, "[<] received job from %s", request.Name)

	return &pb.JobReply{Name: gws.name}, nil
}
