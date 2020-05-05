package node

import (
	"bytes"
	"fmt"
	"io"
	"time"
)

type flags struct {
	pathConfig string

	printConfig bool

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

type config struct {
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
	if err := c.Servers.Validate(); err != nil {
		return fmt.Errorf("servers config is not valid: %w", err)
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

	fmt.Fprintf(w, "%stimeout:\n", ctx.Indent())
	ctx.Wrap(func() {
		fmt.Fprintf(w, "%sworkers-done: %s\n", ctx.Indent(), c.Timeout.WorkersDone)
	})

	fmt.Fprintf(w, "%slogs:\n", ctx.Indent())
	ctx.Wrap(func() { c.Log.Dump(&ctx, w) })
}
