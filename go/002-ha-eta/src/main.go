package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"github.com/gin-gonic/gin"

	apiV1 "github.com/vany-egorov/ha-eta/api-v1/h"
)

type App struct {
}

func (a *App) init() error {
	return nil
}

func (a *App) run(ctx context.Context) {
	if ctx == nil {
		ctx = context.TODO()
	}

	{ // http
		router := a.HTTPRouter()

		go func() {
			addr := "0.0.0.0:80"
			if e := router.Run(addr); e != nil {
				fmt.Fprintf(os.Stderr, "starting http (%s) server failed: %s", addr, e.Error())
				os.Exit(1)
			}
		}()
	}

	<-ctx.Done()
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

func (a *App) HTTPRouter() *gin.Engine {
	gin.SetMode(gin.ReleaseMode)
	// ginHTTPLogger := seelog.New(g.HTTPLogger())

	r := gin.New()
	r.Use(
		gin.Recovery(),
		// prefix.New(),
	)

	r.HandleMethodNotAllowed = true
	// r.NoRoute(baseH.NoRoute, ginHTTPLogger)
	// r.NoMethod(baseH.NoMethod, ginHTTPLogger)

	{
		α := r.Group("/api/v1")
		// α.Use(ginHTTPLogger)

		α.GET("/eta", apiV1.ETA)
	}

	return r
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
