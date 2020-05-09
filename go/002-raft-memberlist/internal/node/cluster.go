package node

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/cenkalti/backoff/v4"
	"github.com/go-x-pkg/log"
	"github.com/hashicorp/memberlist"
)

type clusterLogOutput struct {
	*cluster
}

func (clo *clusterLogOutput) Write(p []byte) (n int, err error) {
	s := strings.TrimSuffix(string(p), "\n")
	clo.fnLog(log.Info, "<<< %s", s)
	return len(s), nil
}

type cluster struct {
	peers Peers
	fnLog log.FnT

	listCfg    *memberlist.Config
	listEvents chan memberlist.NodeEvent
}

func fnLogMembers(fnLog log.FnT, list *memberlist.Memberlist) {
	for _, n := range list.Members() {
		fnLog(log.Info, "* %s", n)
	}
}

func (c *cluster) notifyJoin(n *memberlist.Node) {
	c.fnLog(log.Info, "[+] %s", n)
}

func (c *cluster) notifyLeave(n *memberlist.Node) {
	c.fnLog(log.Warn, "[-] %s", n)
}

func (c *cluster) notifyUpdate(n *memberlist.Node) {
	c.fnLog(log.Info, "[~] %s", n)
}

func (c *cluster) Start(ctx context.Context) error {
	go func() {
		for {
			select {
			case nodeEvent := <-c.listEvents:
				switch nodeEvent.Event {
				case memberlist.NodeJoin:
					c.notifyJoin(nodeEvent.Node)
				case memberlist.NodeLeave:
					c.notifyLeave(nodeEvent.Node)
				case memberlist.NodeUpdate:
					c.notifyUpdate(nodeEvent.Node)
				}
			case <-ctx.Done():
				return
			}
		}
	}()

	list, err := memberlist.Create(c.listCfg)
	if err != nil {
		return fmt.Errorf("memberlist creation failed: %w", err)
	}

	bckff := backoff.WithContext(backoff.NewExponentialBackOff(), ctx)
	ticker := backoff.NewTicker(bckff)

	for range ticker.C {
		if _, err := list.Join(c.peers.pluckAddr()); err != nil {
			c.fnLog(log.Error, "failed to create cluster: %s. will retry.", err)
			continue
		}

		fnLogMembers(c.fnLog, list)
		ticker.Stop()
		break
	}

	return nil
}

func (c *cluster) init(fnLog log.FnT, peers Peers) error {
	if fnLog == nil {
		fnLog = log.LogfStd
	}
	c.fnLog = fnLog

	c.peers = peers

	c.listEvents = make(chan memberlist.NodeEvent)

	cfg := memberlist.DefaultLANConfig()
	cfg.Events = &memberlist.ChannelEventDelegate{
		Ch: c.listEvents,
	}
	// cfg.Name = "$(fqdn)"
	// cfg.BindPort = 7946
	cfg.LogOutput = &clusterLogOutput{c}
	cfg.TCPTimeout = 10 * time.Second
	cfg.PushPullInterval = 30 * time.Second
	cfg.ProbeTimeout = 500 * time.Millisecond
	cfg.ProbeInterval = time.Second
	// cfg.GossipNodes = 3
	cfg.GossipInterval = 200 * time.Millisecond
	cfg.GossipToTheDeadTime = 30 * time.Second

	c.listCfg = cfg

	return nil
}
