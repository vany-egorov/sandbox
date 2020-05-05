package main

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"

	"github.com/vany-egorov/raftmemberlistexample/appversion"
	"github.com/vany-egorov/raftmemberlistexample/internal/node"
)

func newCommand() (*cobra.Command, error) {
	cmd := &cobra.Command{
		Use:   "node",
		Short: "memberlsit/raft node",

		// SilenceUsage is an option to silence usage when an error occurs.
		SilenceUsage: true,

		// SilenceErrors is an option to quiet errors down stream.
		SilenceErrors: true,

		Version: fmt.Sprintf("v%s, build-at %s", appversion.Version, appversion.BuildDate),
	}

	app := node.App{}

	// specify command(run func) by default
	// and flags
	app.CmdSetRunFnAndFlags(cmd)

	cmd.AddCommand(
		app.Command(),
		// ... add other commands here if any
		//     tools, clis, other servers, etc
	)

	return cmd, nil
}

func main() {
	fnTryHandleError := func(err error) {
		if err != nil {
			fmt.Fprintf(os.Stderr, "%s\n", err)
			os.Exit(1)
		}
	}

	cmd, err := newCommand()
	fnTryHandleError(err)
	fnTryHandleError(cmd.Execute())
}
