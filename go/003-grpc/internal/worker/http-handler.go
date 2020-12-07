package worker

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func (a *App) HTTPHandler() http.Handler {
	gin.SetMode(gin.ReleaseMode)

	r := gin.New()

	r.GET("/consul-healthcheck", func(c *gin.Context) {})

	return r
}
