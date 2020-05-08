package node

import (
	"bytes"
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"github.com/go-x-pkg/log"
	"github.com/spf13/pflag"
)

type App struct {
	flags flags
	ctx   AppContext
}

func (a *App) run() (outErr error) {
	_, cancel := context.WithCancel(context.Background())

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
			}
		}
	}()

	logFn(log.Info,
		"application with (:pid %d) started", os.Getpid())

	a.ctx.cfg().DumpToFn(func(buf *bytes.Buffer) {
		logFn(log.Debug,
			"using configuration:\n%s", buf.String())
	})

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
	a.flags.set = flags

	if err := a.initialize(actn, true, true); err != nil {
		return fmt.Errorf("intialization failed: %w", err)
	}

	if a.flags.printConfig {
		actn = actionPrintConfig
	}

	switch actn {
	case actionMain:
		err = a.run()
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
