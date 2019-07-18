package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/vany-egorov/ha-eta/api-v1/h"
)

type App struct {
	name string
}

func (a *App) init() error {
	name, e := os.Hostname()
	if e != nil {
		return e
	}

	a.name = name
	return nil
}

func (a *App) run(ctx context.Context) {
	if ctx == nil {
		ctx = context.TODO()
	}

	t := time.NewTicker(1 * time.Second)

	for {
		select {
		case <-t.C:
			fmt.Printf("< %s/%s @ %d\n", a.name, h.Version, time.Now().Unix())
		case <-ctx.Done():
			return
		}
	}
}

func (*App) signalHandler() {
	ch := make(chan os.Signal)
	signal.Notify(ch,
		os.Interrupt,
		syscall.SIGINT, // CTRL-C
		syscall.SIGTERM,
		syscall.SIGQUIT,
		// syscall.SIGUSR1, // postrotate
		// syscall.SIGHUP,  // reload
	)

	<-ch
	fmt.Println("will shutdown;")
}

func (a *App) Main() error {
	ctx, cancel := context.WithCancel(context.Background())

	if e := a.init(); e != nil {
		return e
	}

	go func() {
		a.signalHandler()
		cancel()
	}()

	a.run(ctx)
	return nil
}

func main() {
	a := App{}
	if e := a.Main(); e != nil {
		fmt.Fprintf(os.Stderr, "execution failed: %s\n", e.Error())
		os.Exit(1)
	}
}
