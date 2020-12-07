package worker

import (
	"github.com/go-x-pkg/log"

	"github.com/vany-egorov/grpcexample/appversion"
)

func (a *App) doVersion() {
	log.LogfStd(log.Info, "v%s, build-at %s",
		appversion.Version, appversion.BuildDate)
}
