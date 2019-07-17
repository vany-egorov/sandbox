package main

import (
	"fmt"

	"github.com/gin-gonic/gin"

	"github.com/vany-egorov/go-build/fns"
)

func main() {
	fmt.Printf("using <gin@%s>\n", gin.Version)
	fmt.Printf("using <fns@%s>\n", fns.Version)
}
