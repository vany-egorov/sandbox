package worker

import (
	"bytes"
	"context"
	"fmt"
	"net"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"

	"github.com/go-x-pkg/log"
	"github.com/go-x-pkg/servers"
	"github.com/spf13/pflag"
	"google.golang.org/grpc"

	pb "github.com/vany-egorov/grpcexample/internal/pkg/service"
)

type App struct {
	flags flags
	ctx   AppContext
}

func (a *App) run(ctx context.Context) (outErr error) {
	ctx, cancel := context.WithCancel(ctx)

	serverErrChan := make(chan struct{})

	wg := sync.WaitGroup{}
	fnGoWithWg := func(fn func()) {
		wg.Add(1)
		go func() {
			defer wg.Done()
			fn()
		}()
	}

	go func() {
		defer cancel()

		signalChan := make(chan os.Signal, 1)
		signal.Notify(signalChan,
			syscall.SIGINT, os.Interrupt, // CTRL-C
			syscall.SIGTERM,
			syscall.SIGQUIT,

			syscall.SIGUSR1, // postrotate
			syscall.SIGHUP,  // reload
		)

		for {
			select {
			case sig := <-signalChan:
				switch sig {
				case syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT:
					logFn(log.Info, "(SIGINT SIGTERM SIGQUIT) will shutdown")

					return

				case syscall.SIGUSR1: // postrotate
					logFn(log.Info, "(SIGUSR1) postrotate logs reloading")
					if err := a.initialize(actionMain, false, true); err != nil {
						logFn(log.Error, "(SIGUSR1) postrotate error: %s", err)
					} else {
						logFn(log.Info, "(SIGUSR1) postrotate OK")
					}

				case syscall.SIGHUP:
					logFn(log.Info, "(SIGHUP) reloading configuration file")
					if err := a.initialize(actionMain, true, true); err != nil {
						logFn(log.Error, "(SIGHUP) reload error: %s", err)
					} else {
						a.ctx.cfg().DumpToFn(func(buf *bytes.Buffer) {
							logFn(log.Debug,
								"(SIGHUP) using configuration:\n%s", buf.String())
						})

						logFn(log.Info, "(SIGHUP) reload OK")
					}
				}
			case <-serverErrChan:
				return
			}
		}
	}()

	logFn(log.Info,
		"application with (:pid %d) started", os.Getpid())

	a.ctx.cfg().DumpToFn(func(buf *bytes.Buffer) {
		logFn(log.Debug,
			"using configuration:\n%s", buf.String())
	})

	fnGoWithWg(func() {
		done, errs := a.ctx.cfg().Servers.ListenAndServe(
			a.HTTPHandler(),
			servers.Context(ctx),
			servers.FnLog(func(lvl log.Level, msg string, a ...interface{}) {
				logFn(lvl, msg, a...)
			}),
			servers.FnShutdownTimeout(func() time.Duration {
				return a.ctx.cfg().Timeout.WorkersDone
			}),
		)

		select {
		case <-done:
			return
		case err := <-errs:
			logFn(log.Critical, err.Error())

			close(serverErrChan)
			outErr = err
		}
	})

	fnGoWithWg(func() {
		listener, err := net.Listen("tcp", fmt.Sprintf("0.0.0.0:%d", a.ctx.cfg().ServerGRPC.Port))
		if err != nil {
			close(serverErrChan)
			outErr = err
		}

		var opts []grpc.ServerOption
		grpcServer := grpc.NewServer(opts...)
		pb.RegisterWorkerServer(grpcServer, &grpcWorkerServer{name: a.ctx.cfg().Name})
		grpcServer.Serve(listener)
	})

	if err := retryRegisterServiceWithConsul(
		ctx,
		a.ctx.cfg().Consul.Host,
		a.ctx.cfg().Consul.Port,
		a.ctx.cfg().Name,
		8000,
	); err != nil {
		close(serverErrChan)
		outErr = err
	}

	wg.Wait()

	return outErr
}

// apply flags, reload config, reload loggers
func (a *App) initialize(actn action, resetConfig bool, resetLoggers bool) error {
	cfg := new(config)

	if err := cfg.build(&a.flags, actn); err != nil {
		return err
	}

	if loggers, err := initLoggers(cfg.Log); err != nil {
		return err
	} else if resetLoggers {
		log.Close( // close old logger if any
			a.ctx.setLoggers(
				loggers))
	}

	if resetConfig {
		a.ctx.setCfg(cfg)
	}

	return nil
}

func (a *App) main(actn action, args []string, flags *pflag.FlagSet) (err error) {
	ctx := context.Background()

	a.flags.set = flags

	if err := a.initialize(actn, true, true); err != nil {
		return fmt.Errorf("intialization failed: %w", err)
	}

	if a.flags.printConfig {
		actn = actionPrintConfig
	}

	switch actn {
	case actionMain:
		err = a.run(ctx)
	case actionVersion:
		a.doVersion()
	case actionPrintConfig:
		a.doPrintConfig()
	}

	return err
}

func (a *App) Main(actn action, args []string, flags *pflag.FlagSet) error {
	return a.main(actn, args, flags)
}
