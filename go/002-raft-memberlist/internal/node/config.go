package node

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"time"

	"github.com/go-x-pkg/bufpool"
	"github.com/go-x-pkg/dumpctx"
	"github.com/go-x-pkg/fnscli"
	"github.com/go-x-pkg/log"
	"github.com/go-x-pkg/xseelog"
	"github.com/spf13/pflag"
)

const (
	defaultPathConfig string = "/etc/node/config.yml"

	defaultDirLog string = "/var/log/node"

	defaultTimeoutWorkersDone time.Duration = 5 * time.Second
)

type flags struct {
	pathConfig string

	printConfig bool

	peers []string

	timeout struct {
		workersDone time.Duration
	}

	logDisableFile    bool
	logDisableConsole bool
	vv                bool
	vvv               bool

	// corresponding flag-set
	set *pflag.FlagSet
}

type configPeer struct {
	Addr string `yaml:"addr"`
}

type config struct {
	Name  string       `yaml:"name"`
	Peers []configPeer `yaml:"peers"`

	Timeout struct {
		WorkersDone time.Duration `yaml:"workers-done"`
	} `yaml:"timeout"`

	Log *xseelog.Config `yaml:"log"`
}

func (c *config) fromFile(flags *flags) error {
	path := flags.pathConfig

	return fnscli.DecodeYAMLFromPath(
		c,                                      // destination to decode
		path,                                   // path to yaml config file
		fnscli.IsPFlagSet(flags.set, "config"), // path to config file was forced
	)
}

func (c *config) defaultize() error {
	if c.Name == "" {
		if n, err := os.Hostname(); err != nil {
			return fmt.Errorf("error get hostname: %w", err)
		} else {
			c.Name = n
		}
	}

	{ // timeout
		if c.Timeout.WorkersDone == 0 {
			c.Timeout.WorkersDone = defaultTimeoutWorkersDone
		}
	}

	{ // log
		if c.Log == nil {
			c.Log = xseelog.NewConfig()
		}

		if c.Log.Dir == "" {
			c.Log.Dir = defaultDirLog
		}

		c.Log.Ensure("app", "[---]", log.Info, log.Critical)
	}

	return nil
}

func (c *config) fromFlags(flags *flags, actn action) error {
	if fnscli.IsPFlagSet(flags.set, "timeout-worker-done") {
		c.Timeout.WorkersDone = flags.timeout.workersDone
	}

	if fnscli.IsPFlagSet(flags.set, "peers") {
		for _, addr := range flags.peers {
			c.Peers = append(c.Peers, configPeer{Addr: addr})
		}
	}

	if flags.logDisableConsole {
		c.Log.DisableConsole = true
	}

	if flags.logDisableFile {
		c.Log.DisableFile = true
	}

	if c.Log.DisableConsole && c.Log.DisableFile {
		c.Log.DisableConsole = false
		c.Log.Quiet()
	}

	if flags.vv {
		c.Log.VV()
	}

	if flags.vvv {
		c.Log.VVV()
	}

	return nil
}

func (c *config) validate(actn action) error {
	if len(c.Peers) == 0 {
		return fmt.Errorf("no peers defined")
	}

	return nil
}

func (c *config) build(flags *flags, actn action) error {
	if err := c.fromFile(flags); err != nil {
		return err
	}

	c.defaultize()

	if err := c.fromFlags(flags, actn); err != nil {
		return err
	}

	if err := c.validate(actn); err != nil {
		return err
	}

	return nil
}

func (c *config) DumpToFn(cb func(*bytes.Buffer)) {
	buf := bufpool.NewBuf()
	buf.Reset()
	c.Dump(buf)
	cb(&buf.Buffer)
	buf.Release()
}

func (c *config) Dump(w io.Writer) {
	ctx := dumpctx.Ctx{}
	ctx.Init()

	fmt.Fprintf(w, "%speers", ctx.Indent())
	if ln := len(c.Peers); ln == 0 {
		fmt.Fprintf(w, " : ~\n")
	} else {
		fmt.Fprintf(w, " (%d):\n", ln)
		ctx.Wrap(func() {
			for _, p := range c.Peers {
				fmt.Fprintf(w, "%s- addr: %s\n", ctx.Indent(), p.Addr)
			}
		})
	}

	fmt.Fprintf(w, "%stimeout:\n", ctx.Indent())
	ctx.Wrap(func() {
		fmt.Fprintf(w, "%sworkers-done: %s\n", ctx.Indent(), c.Timeout.WorkersDone)
	})

	fmt.Fprintf(w, "%slogs:\n", ctx.Indent())
	ctx.Wrap(func() { c.Log.Dump(&ctx, w) })
}
