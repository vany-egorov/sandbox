package broker

import (
	"github.com/spf13/cobra"
)

type action uint8

const (
	actionUnknown action = iota
	actionVersion
	actionPrintConfig
	actionMain
)

var actionText = map[action]string{
	actionUnknown:     "unknown",
	actionVersion:     "version",
	actionPrintConfig: "print-config",
	actionMain:        "main",
}

func (a action) String() string { return actionText[a] }

func (a *App) applyMainFlags(cmd *cobra.Command) {
	cmd.Flags().StringVarP(&a.flags.pathConfig,
		"config", "c",
		defaultPathConfig, "path to configuration file")

	cmd.Flags().DurationVar(&a.flags.timeout.workersDone,
		"timeout-workers-done", defaultTimeoutWorkersDone,
		"wait duration for workers to shutdown gracefully. otherwise force shutdown")

	cmd.Flags().BoolVar(&a.flags.printConfig,
		"print-config", false,
		"print config and exit")

	cmd.Flags().BoolVar(&a.flags.logDisableConsole,
		"log-disable-console", false,
		"disable console stderr/stdout logging")

	cmd.Flags().BoolVar(&a.flags.logDisableFile,
		"log-disable-file", false,
		"disable file logging")

	cmd.Flags().BoolVar(&a.flags.vv,
		"vv", false,
		"all loggers log level forced to debug (very verbose)")

	cmd.Flags().BoolVar(&a.flags.vvv,
		"vvv", false,
		"all loggers log level forced to trace (very very verbose)")
}

func (a *App) CmdSetRunFnAndFlags(cmd *cobra.Command) {
	a.applyMainFlags(cmd)

	cmd.RunE = func(cmd *cobra.Command, args []string) error {
		return a.Main(actionMain, nil, cmd.Flags())
	}

	fnCmdWrap := func(cmd *cobra.Command) *cobra.Command {
		cmd.SilenceUsage = true
		cmd.SilenceErrors = true

		a.applyMainFlags(cmd)

		return cmd
	}

	cmdPrintConfig := fnCmdWrap(&cobra.Command{
		Use:     "print-config",
		Short:   "log down config and exit",
		Aliases: []string{"config"},
		RunE: func(cmd *cobra.Command, args []string) error {
			return a.Main(actionPrintConfig, nil, cmd.Flags())
		},
	})

	cmdVersion := fnCmdWrap(&cobra.Command{
		Use:     "version",
		Short:   "show version and exit",
		Aliases: []string{"print-version"},
		RunE: func(cmd *cobra.Command, args []string) error {
			return a.Main(actionVersion, nil, cmd.Flags())
		},
	})

	cmd.AddCommand(
		cmdVersion,
		cmdPrintConfig,
	)

	// ... add other commands here if any
}

func (a *App) Command() *cobra.Command {
	cmd := &cobra.Command{
		Use:   "broker [...]",
		Short: "broker node",

		// SilenceUsage is an option to silence usage when an error occurs.
		SilenceUsage: true,

		// SilenceErrors is an option to quiet errors down stream.
		SilenceErrors: true,
	}

	a.CmdSetRunFnAndFlags(cmd)

	return cmd
}
