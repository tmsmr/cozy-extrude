package main

import (
	"github.com/gin-gonic/gin"
	"github.com/kelseyhightower/envconfig"
	"log"
	"math"
	"net/http"
	"strconv"
)

type ServerEnvSpec struct {
	HTTPBindAddress string `envconfig:"COZY_HTTP_BIND_ADDRESS" default:"127.0.0.1:8080"`
}

func CozyExtrudeHTTP(ce *CozyExtrudeConn) {
	var serverEnv ServerEnvSpec
	if err := envconfig.Process("", &serverEnv); err != nil {
		log.Fatal(err)
	}

	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	r.GET("/api/status", func(c *gin.Context) {
		status, err := readStatus(ce)
		if err != nil {
			c.AbortWithError(http.StatusInternalServerError, err)
			return
		}
		c.JSON(http.StatusOK, status)
	})

	r.POST("/api/set/:target", func(c *gin.Context) {
		tgtTemp, err := strconv.Atoi(c.Param("target"))
		tgtTemp *= 100
		if err != nil {
			c.AbortWithError(http.StatusUnprocessableEntity, err)
			return
		}
		if tgtTemp > math.MaxUint16 {
			c.AbortWithError(http.StatusUnprocessableEntity, err)
			return
		}
		err = ReqSetTgtTempBlocking(ce, uint16(tgtTemp))
		if err != nil {
			c.AbortWithError(http.StatusInternalServerError, err)
			return
		}
	})

	r.POST("/api/heating/:enable", func(c *gin.Context) {
		enable, err := strconv.Atoi(c.Param("enable"))
		if err != nil {
			c.AbortWithError(http.StatusUnprocessableEntity, err)
			return
		}
		if enable < 0 || enable > 1 {
			c.AbortWithError(http.StatusUnprocessableEntity, err)
			return
		}
		err = ReqEnableHeatingBlocking(ce, enable > 0)
		if err != nil {
			c.AbortWithError(http.StatusInternalServerError, err)
			return
		}
	})

	r.Run(serverEnv.HTTPBindAddress)
}
